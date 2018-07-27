

#include "typedef.h"
/*
typedef unsigned char BYTE;
typedef char          SBYTE;
typedef unsigned int  WORD;
typedef	int           SWORD;
typedef unsigned long DWORD;
typedef long          SDWORD;
typedef unsigned short USHORT ;
*/








#define _DISABLE			0
#define _ENABLE			1
#define _OFF				0
#define _ON				1
#define _FAIL			0
#define _SUCCESS		1
#define _FALSE			0
#define _TRUE			1
#define _LOW			0
#define _HIGH			1



//--------------------------------------------------
// Definitions of Bits
//--------------------------------------------------
#define _BIT0                           0x0001
#define _BIT1                           0x0002
#define _BIT2                           0x0004
#define _BIT3                           0x0008
#define _BIT4                           0x0010
#define _BIT5                           0x0020
#define _BIT6                           0x0040
#define _BIT7                           0x0080
#define _BIT8                           0x0100
#define _BIT9                           0x0200
#define _BIT10                          0x0400
#define _BIT11                          0x0800
#define _BIT12                          0x1000
#define _BIT13                          0x2000
#define _BIT14                          0x4000
#define _BIT15                          0x8000

/*

#ifdef __ACCESS__

// Data

unsigned int  usNum=300;
unsigned char data  Data[16];	// data array for RTD/I2C R/W
unsigned char Datalow;
unsigned char Datahigh;

void CDelay ( WORD DelayCount );
#else

// Data

extern unsigned int  usNum;
extern unsigned char data Data[16];
extern unsigned char Datalow;	
extern unsigned char Datahigh;	


extern void CDelay ( WORD DelayCount );
#endif
*/
