/* Translate_hex_to_C.exe - This utility parses a file with Intel Hex records and creates 
 * an output file with the data in C static arrays. This code has been modified from the 
 * original Map2 program to convert the hex file from the low power monitor project from 
 * Sequoia into C static arrays so that the hex file can be included in the flash when 
 * compiling the Cortez project. This tool separates code for different segments into 
 * different arrays.
 */
 
//
// History
//
// Ver.
// 1.0 First implementation (by Gopy Bendigeri on 2005/01/03)
// 2.0 Modified to populate FLASHDriverDumps[] with starting address and size of each segment
//     for low power mode for Sequoia (by Brian Bao on May 14, 2007)
//     Also changed name from Map2 to Translate_hex_to_c to better reflect its purpose.
// 3.0 Modified to meet Athena memory map (By Eugene Su on Oct 19, 2010)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAJORVER     3
#define MINORVER     0

// Maximum length of one Intel Hex record
#define LINELENGTH   81

// Output file name
#define OUTPUTFILENAME     "LPM_hex.c"

// Data array base name and first suffix
#define BASEARRAYNAME      "DataArray_"
#define FIRSTARRAYSUFFIX   'A'

// Types of Intel Hex records
#define DATA_RECORD              0x00
#define END_RECORD               0x01
#define EXT_SEG_ADDR_RECORD      0x02
#define START_SEG_ADDR_RECORD    0x03
#define EXT_LINEAR_ADDR_RECORD   0x04
#define START_LINEAR_ADDR_RECORD 0x05

#define TBL_OFFSET 0x4000 // Table Access Offset

typedef enum eParseStateTag
{
   NullState,
   DataRecordState,
   EndRecordState,
   ExtSegAddrRecordState,
   StartSegAddrRecordState,
   ExtLinearAddrRecordState,
   StartLinearAddrRecordState
} eParseState_t;


// Prototypes of local functions
void GetRecordParams( char *pS_Line, unsigned char *pUC_RecLength, unsigned int *pUI_Address,
      unsigned char *pUC_RecType );
void GetRecordData( char *pS_Line, unsigned char UC_RecLength, FILE *pF_OutputFile );
unsigned int hexatoi( char *pS_Num );
void PrintHelp( void );
void PrintDescriptor( unsigned int *pUI_SegAddrs, unsigned int uiNumSegments, FILE *pF_OutFile );

unsigned int SegmentAddrs[128];  // We assume there are no more than 128 segments in the image
unsigned int SegmentSizes[128];
unsigned int SegmentCount = 0;

// Main program
int main( int argc, char **argv )
{
   eParseState_t ParseState;
   FILE *Fp_Input = (FILE*)NULL;        // Input stream
   FILE *Fp_Output = (FILE*)NULL;       // Output stream
   char OneLine[LINELENGTH];
   char sArraySuffix[2];
   char sArrayName[16];
   unsigned int ulLineCount = 0, uiSegmentCount = 0;
   unsigned char ucRecLength, ucRecType, ucPrevLength;
   unsigned int uiLdAddr, uiStartAddr, uiEndAddr, uiSegmentAddr;
   unsigned int uiPrevAddr = 0;

   printf("Translate_hex_to_C.exe - A utility to convert Intel Hex records to C static arrays. ");
   printf("Ver %d.%d\n\n", MAJORVER, MINORVER);

   // Check number of arguments
   if ( argc < 2 )
   {
      printf("Not enough arguments.\n");
      PrintHelp();
      exit(1);
   }

   // Open input file for reading
   if ( (Fp_Input = fopen( argv[1], "r" )) == NULL )
   {
      printf("Could not open input file %s for reading.\n", argv[1]);
      exit(1);
   }

   // Open output file for writing
   if ( (Fp_Output = fopen(OUTPUTFILENAME, "w")) == NULL )
   {
      printf("Could not open output file %s for writing.\n", OUTPUTFILENAME);
      exit(1);
   }

   // Write the header, etc to the output file
   fprintf( Fp_Output, "// This is a generated file by Translate_hex_to_C.exe with Intel\n" );
   fprintf( Fp_Output, "// Hex Data records converted to C static arrays.\n\n");
   fprintf( Fp_Output, "#include \".\\StdLibInc\\gm_Global.h\"\n");
   fprintf( Fp_Output, "#include \".\\StdLibInc\\gm_ChipFeature.h\"\n\n");
   fprintf( Fp_Output, "#if (FEATURE_LOW_POWER_MONITOR == ENABLE)\n");

   // Initialize the data array name suffix
   sArraySuffix[0] = FIRSTARRAYSUFFIX;
   sArraySuffix[1] = 0;

   // As long as there are lines to read, process input
   while ( (feof( Fp_Input ) == 0) && (fgets( OneLine, LINELENGTH, Fp_Input ) != NULL) )
   {
      ulLineCount++;    // Increment line count

      // Check if the first character is a colon. All Intel hex records start with a colon
      // in the first column.
      if ( OneLine[0] != ':' )
      {
         printf("Error processing Intel Hex record on line %ul\n", ulLineCount);
         // break;
      }

      // Get the record parameters
      GetRecordParams( &OneLine[0], &ucRecLength, &uiLdAddr, &ucRecType );

      // Debug code
      // printf( "Length: 0x%x Address: 0x%x Type 0x%x ", ucRecLength, uiLdAddr, ucRecType );

      // Take action based on the record type
      switch ( ucRecType )
      {
         case DATA_RECORD:
            if ( ParseState != DataRecordState )
            {
               // We've come here for the first time. We have to open
               // the data array for this block of data.
               uiStartAddr = uiLdAddr;
               // Compose the array name
               strcpy( sArrayName, BASEARRAYNAME );
               strcat( sArrayName, sArraySuffix );
               // Increment the suffix
               sArraySuffix[0]++;
               SegmentCount++;
               
               if ( sArraySuffix[0] > 'Z' )
               {
                  sArraySuffix[0] = 'a';
               }
               fprintf(Fp_Output, "BYTE ROM %s[] =\n", sArrayName);
               fprintf(Fp_Output, "{\n");
            }

            // Call the function to extract the data from the record and
            // write to the output file
            GetRecordData( &OneLine[0], ucRecLength, Fp_Output );

            // Save the current address and length
            uiPrevAddr = uiLdAddr;
            ucPrevLength = ucRecLength;
            ParseState = DataRecordState;
            break;

         case END_RECORD:
            if ( ParseState == DataRecordState )
            {
               // The previous record was a data record.
               // We need to calculate the end address and the length
               // of data. We also have to close the current array.
               uiEndAddr = uiPrevAddr + (int)ucPrevLength;
               // Print addresses and lengths to the output file
               fprintf(Fp_Output, "};\n");
               fprintf(Fp_Output, "// END Start Addr: 0x%x, End Addr: 0x%x, Length: %d\n\n",
                     uiStartAddr, uiEndAddr-1, (uiEndAddr - uiStartAddr));
               SegmentAddrs[SegmentCount-1] = uiStartAddr - TBL_OFFSET;
               SegmentSizes[SegmentCount-1] = (uiEndAddr - uiStartAddr);
            }
            ParseState = EndRecordState;
            break;

         case EXT_SEG_ADDR_RECORD:
            if ( ParseState == DataRecordState )
            {
               // The previous record was a data record.
               // We need to calculate the end address and the length
               // of data. We also have to close the current array.
               uiEndAddr = uiPrevAddr + (int)ucPrevLength;
               // Print addresses and lengths to the output file
               fprintf(Fp_Output, "};\n");
               fprintf(Fp_Output, "// EXT Start Addr: 0x%x, End Addr: 0x%x, Length: %d\n\n",
                     uiStartAddr, uiEndAddr-1, (uiEndAddr - uiStartAddr));
            }

            // Process the extended segment address. This forms bits A19:A4 of the
            // physical segment address. Print out the physical address.
            // Copy the segment address string to a temporary location. Use the
            // array sArrayName for this purpose.
            strncpy( sArrayName, &OneLine[9], 4 );
            sArrayName[4] = '\0';   // Do not forget to append a null char
            uiSegmentAddr = hexatoi( sArrayName );
            SegmentAddrs[uiSegmentCount] = uiSegmentAddr << 4;
            fprintf(Fp_Output, "// Segment address: 0x%x\n", SegmentAddrs[uiSegmentCount]);
            uiSegmentCount++;
            ParseState = ExtSegAddrRecordState;
            break;

         case START_SEG_ADDR_RECORD:
            if ( ParseState == DataRecordState )
            {
               // The previous record was a data record.
               // We need to calculate the end address and the length
               // of data. We also have to close the current array.
               uiEndAddr = uiPrevAddr + (int)ucPrevLength;
               // Print addresses and lengths to the output file
               fprintf(Fp_Output, "};\n");
               fprintf(Fp_Output, "// START Start Addr: 0x%x, End Addr: 0x%x, Length: %d\n\n",
                     uiStartAddr, uiEndAddr-1, (uiEndAddr - uiStartAddr));
               SegmentAddrs[SegmentCount-1] = uiStartAddr - TBL_OFFSET;
               SegmentSizes[SegmentCount-1] = (uiEndAddr - uiStartAddr);
            }
            ParseState = StartSegAddrRecordState;
            break;

         case EXT_LINEAR_ADDR_RECORD:
            if ( ParseState == DataRecordState )
            {
               // The previous record was a data record.
               // We need to calculate the end address and the length
               // of data. We also have to close the current array.
               uiEndAddr = uiPrevAddr + (int)ucPrevLength;
               // Print addresses and lengths to the output file
               fprintf(Fp_Output, "};\n");
               fprintf(Fp_Output, "// EXT LIN Start Addr: 0x%x, End Addr: 0x%x, Length: %d\n\n",
                     uiStartAddr, uiEndAddr-1, (uiEndAddr - uiStartAddr));
               SegmentAddrs[SegmentCount-1] = uiStartAddr- TBL_OFFSET;
               SegmentSizes[SegmentCount-1] = (uiEndAddr - uiStartAddr);
            }
            ParseState = ExtLinearAddrRecordState;
            break;

         case START_LINEAR_ADDR_RECORD:
            if ( ParseState == DataRecordState )
            {
               // The previous record was a data record.
               // We need to calculate the end address and the length
               // of data. We also have to close the current array.
               uiEndAddr = uiPrevAddr + (int)ucPrevLength;
               // Print addresses and lengths to the output file
               fprintf(Fp_Output, "};\n");
               fprintf(Fp_Output, "// START LIN Start Addr: 0x%x, End Addr: 0x%x, Length: %d\n\n",
                     uiStartAddr, uiEndAddr-1, (uiEndAddr - uiStartAddr));
               SegmentAddrs[SegmentCount-1] = uiStartAddr - TBL_OFFSET;
               SegmentSizes[SegmentCount-1] = (uiEndAddr - uiStartAddr);
            }
            ParseState = StartLinearAddrRecordState;
            break;

         default:
            ParseState = NullState;
            break;
      }

   } // End of loop

   // Print the descriptor array
   //PrintDescriptor( SegmentAddrs, uiSegmentCount, Fp_Output );
	PrintDescriptor( SegmentAddrs, SegmentCount, Fp_Output );
   printf("Lines processed: %ul\n", ulLineCount);
   printf("Output is stored in file %s.\n", OUTPUTFILENAME);
   fclose( Fp_Input );
   fclose( Fp_Output );
}

// Function that extracts the record type, record length and load address of the Intel
// Hex record.
void GetRecordParams( char *pS_Line, unsigned char *pUC_RecLength, unsigned int *pUI_Address,
      unsigned char *pUC_RecType )
{
   char TempStr[8];
   unsigned int iParam;

   // First copy the length string to the temporary array
   strncpy( TempStr, &pS_Line[1], 2 );
   TempStr[2] = '\0';   // Append a null character to make it a string
   iParam = hexatoi( TempStr );
   *pUC_RecLength = (unsigned char)iParam;

   // Next, parse the address and the record type the same way
   strncpy( TempStr, &pS_Line[3], 4 );
   TempStr[4] = '\0';   // Append a null character to make it a string
   iParam = hexatoi( TempStr );
   *pUI_Address = (unsigned int)iParam;

   strncpy( TempStr, &pS_Line[7], 2 );
   TempStr[2] = '\0';   // Append a null character to make it a string
   iParam = hexatoi( TempStr );
   *pUC_RecType = (unsigned char)iParam;
}

// Function that reads the data from the Intel Hex record and prints the string
// directly to the output file stream passed in by the caller.
void GetRecordData( char *pS_Line, unsigned char UC_RecLength, FILE *pF_OutputFile )
{
   int i;
   char TempOneByte[3];
   char *pS_CurrByte;

   // Point to the first byte of the data
   pS_CurrByte = &pS_Line[9];

   for ( i = 0; i < UC_RecLength; i++ )
   {
      strncpy( TempOneByte, pS_CurrByte, 2 );
      TempOneByte[2] = '\0';     // Do not forget to append a null character
      pS_CurrByte += 2;    // Point to the next byte
      fprintf(pF_OutputFile, "0x%s, ", TempOneByte);
   }
   fprintf(pF_OutputFile, "\n");
}

// Converts a string with Hex digits to an integer. Limit is a
// 16-bit value (4 Hex digits). Can be expanded to more, if needed.
unsigned int hexatoi( char *pS_Num )
{
   int i, iLen;
   int iNum = 0;
   char cDigit;

   iLen = strlen(pS_Num);     // Get the string length

   if ( iLen > 4 )
   {
      return iNum;   // Return 0 if more than four digits
   }

   for ( i = 0; i < iLen; i++ )
   {
      cDigit = pS_Num[i];  // Get the digit

      if ( cDigit >= '0' && cDigit <= '9' )
      {
         cDigit -= '0';
      }
      else if ( cDigit >= 'a' && cDigit <= 'f' )
      {
         cDigit = cDigit - 'a' + 10;
      }
      else if ( cDigit >= 'A' && cDigit <= 'F' )
      {
         cDigit = cDigit - 'A' + 10;
      }
      else
      {
         cDigit = 0;
      }

      iNum = iNum * 16 + (int)cDigit;
   }
   
   return iNum;
}

// Function that prints the usage of this utility.
void PrintHelp( void )
{
   printf("\nUsage: Translate_hex_to_C.exe <filename>\n");
   printf("    where <filename> is the file containing Intel Hex records.\n");
}


// Function that prints the descriptor array for the main code to include
void PrintDescriptor( unsigned int *pUI_SegAddrs, unsigned int uiNumSegments, FILE *pF_OutFile )
{
   unsigned int i;
   char sArray[16];
   char sSuffix[2];

   fprintf(pF_OutFile, "typedef struct\n");
   fprintf(pF_OutFile, "{\n");
   fprintf(pF_OutFile, "\tBYTE ROM * Ba_Array;\n");
   fprintf(pF_OutFile, "\tWORD W_StartingOffset;\n");
   fprintf(pF_OutFile, "\tWORD W_Length;\n");
   fprintf(pF_OutFile, "} TDumpDescriptor;\n");

   fprintf(pF_OutFile, "\n// Descriptor array for above ISP driver\n");
   fprintf(pF_OutFile, "TDumpDescriptor ROM FLASHDriverDumps[] =\n");
   fprintf(pF_OutFile, "{\n");

   sSuffix[0] = FIRSTARRAYSUFFIX;
   sSuffix[1] = 0;

   for ( i = 0; i < uiNumSegments; i++ )
   {
      strcpy( sArray, BASEARRAYNAME );
      strcat( sArray, sSuffix );
      fprintf(pF_OutFile, "\t{%s, 0x%x, %dU},\n", sArray, pUI_SegAddrs[i], SegmentSizes[i]);
      sSuffix[0]++;
      if ( sSuffix[0] > 'Z' )
      {
         sSuffix[0] = 'a';
      }
   }
   fprintf(pF_OutFile, "\t{0, 0, 0}\n");//delimiter
   fprintf(pF_OutFile, "};\n\n");
   fprintf(pF_OutFile, "#endif\n\n");
}
