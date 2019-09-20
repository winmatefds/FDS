/*
	$Workfile:   OverdriveLut.h  $
	$Revision: 1.1 $
	$Date: 2010/03/19 07:53:10 $
*/

/******************************************************************************
				O V E R D R I V E    L U T
/*****************************************************************************/


extern ROM WORD OverDriveLut_SXGA_LG_LM190E01_MED[];
//extern ROM WORD OverDriveLut_SXGA_LG_LM190E01_LOW[];
//extern ROM WORD OverDriveLut_SXGA_LG_LM190E01_HIGH[];

WORD ROM * ROM Wa_OverDriveLUT_SXGA_LG_LM190E01[] =
{
	OverDriveLut_SXGA_LG_LM190E01_MED,
//	OverDriveLut_SXGA_LG_LM190E01_LOW,
//	OverDriveLut_SXGA_LG_LM190E01_HIGH,
	NULL_PTR,						// Signifies no more tables
};


//Medium
ROM WORD OverDriveLut_SXGA_LG_LM190E01_MED[] = 
{
// Given by Ling Jul. 07, 2005
0x22, 0x49, 0x73, 0x94, 0xc2, 0xdc, 0xf5, 0x106, 
0x3fe, 0x46, 0x71, 0x94, 0xbb, 0xd6, 0xf2, 0x106, 
0x3fb, 0x16, 0x69, 0x8d, 0xb3, 0xd4, 0xf2, 0x106, 
0x3f8, 0xe, 0x3a, 0x85, 0xac, 0xce, 0xef, 0x105, 
0x3f8, 0xa, 0x36, 0x5c, 0xa6, 0xca, 0xec, 0x104, 
0x3f5, 0x4, 0x30, 0x57, 0x7b, 0xc5, 0xe9, 0x103, 
0x3f2, 0x3ff, 0x2b, 0x52, 0x76, 0x9c, 0xe5, 0x102, 
0x3ef, 0x3fb, 0x27, 0x4e, 0x72, 0x99, 0xbd, 0x101, 
0x3ef, 0x3fa, 0x24, 0x4c, 0x6f, 0x97, 0xbb, 0xde,
};

//Low
/*
ROM WORD OverDriveLut_SXGA_LG_LM190E01_LOW[] = 
{
// Given by Ling Jul. 07, 2005
0x22, 0x49, 0x73, 0x94, 0xc2, 0xdc, 0xf5, 0x106, 
0x3fe, 0x46, 0x71, 0x94, 0xbb, 0xd6, 0xf2, 0x106, 
0x3fb, 0x16, 0x69, 0x8d, 0xb3, 0xd4, 0xf2, 0x106, 
0x3f8, 0xe, 0x3a, 0x85, 0xac, 0xce, 0xef, 0x105, 
0x3f8, 0xa, 0x36, 0x5c, 0xa6, 0xca, 0xec, 0x104, 
0x3f5, 0x4, 0x30, 0x57, 0x7b, 0xc5, 0xe9, 0x103, 
0x3f2, 0x3ff, 0x2b, 0x52, 0x76, 0x9c, 0xe5, 0x102, 
0x3ef, 0x3fb, 0x27, 0x4e, 0x72, 0x99, 0xbd, 0x101, 
0x3ef, 0x3fa, 0x24, 0x4c, 0x6f, 0x97, 0xbb, 0xde,
};
*/

//High
/*
ROM WORD OverDriveLut_SXGA_LG_LM190E01_HIGH[] = 
{
// Given by Ling Jul. 07, 2005
0x22, 0x49, 0x73, 0x94, 0xc2, 0xdc, 0xf5, 0x106, 
0x3fe, 0x46, 0x71, 0x94, 0xbb, 0xd6, 0xf2, 0x106, 
0x3fb, 0x16, 0x69, 0x8d, 0xb3, 0xd4, 0xf2, 0x106, 
0x3f8, 0xe, 0x3a, 0x85, 0xac, 0xce, 0xef, 0x105, 
0x3f8, 0xa, 0x36, 0x5c, 0xa6, 0xca, 0xec, 0x104, 
0x3f5, 0x4, 0x30, 0x57, 0x7b, 0xc5, 0xe9, 0x103, 
0x3f2, 0x3ff, 0x2b, 0x52, 0x76, 0x9c, 0xe5, 0x102, 
0x3ef, 0x3fb, 0x27, 0x4e, 0x72, 0x99, 0xbd, 0x101, 
0x3ef, 0x3fa, 0x24, 0x4c, 0x6f, 0x97, 0xbb, 0xde,
};
*/