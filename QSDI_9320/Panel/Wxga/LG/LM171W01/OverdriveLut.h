/*
	$Workfile:   OverdriveLut.h  $
	$Revision: 1.2 $
	$Date: 2010/10/14 09:12:18 $
*/

/******************************************************************************
				O V E R D R I V E    L U T
/*****************************************************************************/


//extern ROM WORD OverDriveLut_LM171W01_MED[];
//extern ROM WORD OverDriveLut_LM171W01_LOW[];
//extern ROM WORD OverDriveLut_LM171W01_HIGH[];


//Medium
ROM SWORD OverDriveLut_LM171W01_MED[] =
{
// Given by Ling Jul. 07, 2005
0x23, 0x51, 0x86, 0xb6, 0xdf, 0xf1, 0x105, 0x11d, 
0x3fc, 0x48, 0x77, 0xa7, 0xd1, 0xeb, 0xff, 0x117, 
0x3f9, 0x17, 0x6c, 0x97, 0xc2, 0xe5, 0xfb, 0x112, 
0x3f1, 0x10, 0x39, 0x8f, 0xb8, 0xe1, 0xf8, 0x111, 
0x3ed, 0x9, 0x34, 0x59, 0xb1, 0xda, 0xf5, 0x10f, 
0x3ea, 0x4, 0x2f, 0x51, 0x76, 0xd2, 0xf2, 0x10f, 
0x3eb, 0x2, 0x2b, 0x4d, 0x6f, 0x93, 0xec, 0x10a, 
0x3ee, 0x3, 0x2a, 0x4a, 0x6a, 0x8d, 0xb3, 0x108, 
0x3fa, 0x11, 0x2f, 0x49, 0x65, 0x89, 0xb0, 0xd6,
};

//Low
/*
ROM WORD OverDriveLut_LM171W01_LOW[] =
{
// Given by Ling Jul. 07, 2005
0x23, 0x51, 0x86, 0xb6, 0xdf, 0xf1, 0x105, 0x11d, 
0x3fc, 0x48, 0x77, 0xa7, 0xd1, 0xeb, 0xff, 0x117, 
0x3f9, 0x17, 0x6c, 0x97, 0xc2, 0xe5, 0xfb, 0x112, 
0x3f1, 0x10, 0x39, 0x8f, 0xb8, 0xe1, 0xf8, 0x111, 
0x3ed, 0x9, 0x34, 0x59, 0xb1, 0xda, 0xf5, 0x10f, 
0x3ea, 0x4, 0x2f, 0x51, 0x76, 0xd2, 0xf2, 0x10f, 
0x3eb, 0x2, 0x2b, 0x4d, 0x6f, 0x93, 0xec, 0x10a, 
0x3ee, 0x3, 0x2a, 0x4a, 0x6a, 0x8d, 0xb3, 0x108, 
0x3fa, 0x11, 0x2f, 0x49, 0x65, 0x89, 0xb0, 0xd6,
};
*/

//High
/*
ROM WORD OverDriveLut_LM171W01_HIGH[] =
{
// Given by Ling Jul. 07, 2005
0x23, 0x51, 0x86, 0xb6, 0xdf, 0xf1, 0x105, 0x11d, 
0x3fc, 0x48, 0x77, 0xa7, 0xd1, 0xeb, 0xff, 0x117, 
0x3f9, 0x17, 0x6c, 0x97, 0xc2, 0xe5, 0xfb, 0x112, 
0x3f1, 0x10, 0x39, 0x8f, 0xb8, 0xe1, 0xf8, 0x111, 
0x3ed, 0x9, 0x34, 0x59, 0xb1, 0xda, 0xf5, 0x10f, 
0x3ea, 0x4, 0x2f, 0x51, 0x76, 0xd2, 0xf2, 0x10f, 
0x3eb, 0x2, 0x2b, 0x4d, 0x6f, 0x93, 0xec, 0x10a, 
0x3ee, 0x3, 0x2a, 0x4a, 0x6a, 0x8d, 0xb3, 0x108, 
0x3fa, 0x11, 0x2f, 0x49, 0x65, 0x89, 0xb0, 0xd6,
};
*/


SWORD ROM * ROM Wa_OverDriveLUT_LM171W01[] =
{
	OverDriveLut_LM171W01_MED,
//	OverDriveLut_LM171W01_LOW,
//	OverDriveLut_LM171W01_HIGH,
	NULL_PTR,						// Signifies no more tables
};

