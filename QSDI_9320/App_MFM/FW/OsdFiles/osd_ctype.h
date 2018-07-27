////////////////////////////////////////////////////////////////////////////////
//    Genesis Microchip, Inc.
// This file was generated by Genesis Workbench. PLEASE DO NOT EDIT MANUALLY
////////////////////////////////////////////////////////////////////////////////

#ifndef OSD_CTYPE_H_DEFINED
#define OSD_CTYPE_H_DEFINED

#define WB_MAJOR		0x0004
#define WB_MINOR		0x0004
#define WB_REVISION		0x0004
#define WB_BUILD		0x0004

#define WB_ADJUSTER_ENGINE_VERSION		0x0004
#define WB_STATE_MACHINE_VERSION		0x0004
#ifdef __PARADIGM__
#pragma pack(push)
#pragma pack(1)
#endif //__PARADIGM__

////////////////////////////////////////////////////////////////////////////////
//    Enumerations
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//    Control Data Structures
////////////////////////////////////////////////////////////////////////////////

/*

Begin block defined in libs

typedef struct gmt_OSD_INVISIBLESTATE_CTRLStruct
{
	BYTE	FragmNum;
	gmt_OSD_CLUT ROM*	ColorRef;
} gmt_OSD_INVISIBLESTATE_CTRL;



End block defined in libs*/

/*

Begin block defined in libs

typedef struct gmt_OSD_TILE_CTRLStruct
{
	BYTE	TileId;
	BYTE	DialogId;
	WORD	XPos;
	WORD	YPos;
	WORD	XSize;
	WORD	YSize;
	BYTE	Attr;
	BYTE	ColorAttr;
	DWORD	TileAddr;
	BYTE	AllocScheme;
} gmt_OSD_TILE_CTRL;



End block defined in libs*/

/*

Begin block defined in libs

typedef struct gmt_OSD_BITMAP_CTRLStruct
{
	BYTE	TileId;
	WORD	XPos;
	WORD	YPos;
	WORD	XSize;
	WORD	YSize;
	BYTE	EraseColorIndex;
	gmt_OSD_UNI_BITMAP	Colors;
} gmt_OSD_BITMAP_CTRL;



End block defined in libs*/

/*

Begin block defined in libs

typedef struct gmt_OSD_RECTANGLE_CTRLStruct
{
	BYTE	TileId;
	WORD	XPos;
	WORD	YPos;
	WORD	XSize;
	WORD	YSize;
	BYTE	EraseColorIndex;
	BYTE ROM *	Colors;
	BYTE	BorderWidth;
} gmt_OSD_RECTANGLE_CTRL;



End block defined in libs*/

/*

Begin block defined in libs

typedef struct gmt_OSD_TEXT_CTRLStruct
{
	BYTE	TileId;
	WORD	XPos;
	WORD	YPos;
	WORD	XSize;
	WORD	YSize;
	BYTE	EraseColorIndex;
	BYTE ROM *	Text;
	BYTE ROM *	Colors;
} gmt_OSD_TEXT_CTRL;



End block defined in libs*/

/*

Begin block defined in libs

typedef struct gmt_OSD_TEXTBUTTON_CTRLStruct
{
	BYTE	TileId;
	WORD	XPos;
	WORD	YPos;
	WORD	XSize;
	WORD	YSize;
	BYTE	EraseColorIndex;
	BYTE ROM *	Text;
	BYTE ROM *	Colors;
} gmt_OSD_TEXTBUTTON_CTRL;



End block defined in libs*/

/*

Begin block defined in libs

typedef struct gmt_OSD_SLIDERBUTTON_CTRLStruct
{
	BYTE	TileId;
	WORD	XPos;
	WORD	YPos;
	WORD	XSize;
	WORD	YSize;
	BYTE	EraseColorIndex;
	WORD	Associate;
	BYTE	Direction;
	BYTE ROM *	Colors;
	BYTE	BorderWidth;
} gmt_OSD_SLIDERBUTTON_CTRL;



End block defined in libs*/

/*

Begin block defined in libs

typedef struct gmt_OSD_LONGVALUE_CTRLStruct
{
	BYTE	TileId;
	WORD	XPos;
	WORD	YPos;
	WORD	XSize;
	WORD	YSize;
	BYTE	EraseColorIndex;
	WORD	Associate;
	BYTE ROM *	UsedGlyphs;
	SWORD	MinValue;
	SWORD	MaxValue;
	BYTE ROM *	Colors;
	BYTE	Horizontal;
	BYTE	Vertical;
} gmt_OSD_LONGVALUE_CTRL;



End block defined in libs*/

/*

Begin block defined in libs

typedef struct gmt_OSD_DSTATICTEXTBUTTON_CTRLStruct
{
	BYTE	TileId;
	WORD	XPos;
	WORD	YPos;
	WORD	XSize;
	WORD	YSize;
	BYTE	EraseColorIndex;
	BYTE ROM *	Text;
	BYTE ROM *	Colors;
	BYTE ROM *	Text_2;
	BYTE ROM *	Text_3;
	BYTE ROM *	Text_4;
	BYTE ROM *	Text_5;
	BYTE ROM *	Text_6;
	BYTE ROM *	Text_7;
	BYTE ROM *	Text_8;
} gmt_OSD_DSTATICTEXTBUTTON_CTRL;



End block defined in libs*/

/*

Begin block defined in libs

typedef struct gmt_OSD_LONGVALUEBUTTON_CTRLStruct
{
	BYTE	TileId;
	WORD	XPos;
	WORD	YPos;
	WORD	XSize;
	WORD	YSize;
	BYTE	EraseColorIndex;
	WORD	Associate;
	BYTE ROM *	UsedGlyphs;
	SWORD	MinValue;
	SWORD	MaxValue;
	BYTE ROM *	Colors;
	BYTE	Horizontal;
	BYTE	Vertical;
} gmt_OSD_LONGVALUEBUTTON_CTRL;



End block defined in libs*/

/*

Begin block defined in libs

typedef struct gmt_OSD_BITMAPBUTTON_CTRLStruct
{
	BYTE	TileId;
	WORD	XPos;
	WORD	YPos;
	WORD	XSize;
	WORD	YSize;
	BYTE	EraseColorIndex;
	gmt_OSD_UNI_BITMAP	Colors;
} gmt_OSD_BITMAPBUTTON_CTRL;



End block defined in libs*/

/*

Begin block defined in libs

typedef struct gmt_OSD_TIMINGINFO_CTRLStruct
{
	BYTE	TileId;
	WORD	XPos;
	WORD	YPos;
	WORD	XSize;
	WORD	YSize;
	BYTE	EraseColorIndex;
	BYTE ROM *	UsedGlyphs;
	BYTE ROM *	Colors;
	BYTE	Channel;
	BYTE	Horizontal;
	BYTE	Vertical;
} gmt_OSD_TIMINGINFO_CTRL;



End block defined in libs*/

/*

Begin block defined in libs

typedef struct gmt_OSD_DSTATICTEXT_CTRLStruct
{
	BYTE	TileId;
	WORD	XPos;
	WORD	YPos;
	WORD	XSize;
	WORD	YSize;
	BYTE	EraseColorIndex;
	BYTE ROM *	Text;
	BYTE ROM *	Colors;
	BYTE ROM *	Text_2;
	BYTE ROM *	Text_3;
	BYTE ROM *	Text_4;
	BYTE ROM *	Text_5;
	BYTE ROM *	Text_6;
	BYTE ROM *	Text_7;
	BYTE ROM *	Text_8;
} gmt_OSD_DSTATICTEXT_CTRL;



End block defined in libs*/

/*

Begin block defined in libs

typedef struct gmt_OSD_DIALOG_CTRLStruct
{
	BYTE	DialogId;
	WORD	XSize;
	WORD	YSize;
	WORD	MaxXpos;
	WORD	MaxYpos;
	WORD	HPositionRef;
	WORD	VPositionRef;
	BYTE	FragmNum;
	gmt_OSD_CLUT ROM*	ColorRef;
	WORD	XScalePercent;
	WORD	YScalePercent;
} gmt_OSD_DIALOG_CTRL;



End block defined in libs*/

typedef struct gmt_OSD_TIMELONGVALUE_CTRLStruct
{
	BYTE	TileId;
	WORD	XPos;
	WORD	YPos;
	WORD	XSize;
	WORD	YSize;
	BYTE	EraseColorIndex;
	WORD	Associate;
	BYTE ROM *	UsedGlyphs;
	SWORD	MinValue;
	SWORD	MaxValue;
	BYTE ROM *	Colors;
	BYTE	Horizontal;
	BYTE	Vertical;
} gmt_OSD_TIMELONGVALUE_CTRL;

/*

Begin block defined in libs

typedef struct gmt_OSD_DSTATICTEXTEXT_CTRLStruct
{
	BYTE	TileId;
	WORD	XPos;
	WORD	YPos;
	WORD	XSize;
	WORD	YSize;
	BYTE	EraseColorIndex;
	BYTE ROM *	Text;
	BYTE ROM *	Colors;
	BYTE ROM *	Text_2;
	BYTE ROM *	Text_3;
	BYTE ROM *	Text_4;
	BYTE ROM *	Text_5;
	BYTE ROM *	Text_6;
	BYTE ROM *	Text_7;
	BYTE ROM *	Text_8;
	BYTE ROM *	Text_9;
	BYTE ROM *	Text_10;
	BYTE ROM *	Text_11;
	BYTE ROM *	Text_12;
	BYTE ROM *	Text_13;
	BYTE ROM *	Text_14;
	BYTE ROM *	Text_15;
	BYTE ROM *	Text_16;
	BYTE ROM *	Text_17;
	BYTE ROM *	Text_18;
	BYTE ROM *	Text_19;
	BYTE ROM *	Text_20;
	BYTE ROM *	Text_21;
	BYTE ROM *	Text_22;
	BYTE ROM *	Text_23;
	BYTE ROM *	Text_24;
	BYTE ROM *	Text_25;
	BYTE ROM *	Text_26;
	BYTE ROM *	Text_27;
	BYTE ROM *	Text_28;
	BYTE ROM *	Text_29;
	BYTE ROM *	Text_30;
	BYTE ROM *	Text_31;
	BYTE ROM *	Text_32;
} gmt_OSD_DSTATICTEXTEXT_CTRL;



End block defined in libs*/

/*

Begin block defined in libs

typedef struct gmt_OSD_DYNAMICTEXT_CTRLStruct
{
	BYTE	TileId;
	WORD	XPos;
	WORD	YPos;
	WORD	XSize;
	WORD	YSize;
	BYTE	EraseColorIndex;
	BYTE ROM *	Colors;
	BYTE	Horizontal;
	BYTE	Vertical;
	BYTE	MaxString;
	BYTE ROM *	UsedGlyphs;
	WORD	PtrSegment;
	WORD	PtrOffset;
} gmt_OSD_DYNAMICTEXT_CTRL;



End block defined in libs*/

/*

Begin block defined in libs

typedef struct gmt_OSD_BITMAPSLIDER_CTRLStruct
{
	BYTE	TileId;
	WORD	XPos;
	WORD	YPos;
	WORD	XSize;
	WORD	YSize;
	BYTE	EraseColorIndex;
	gmt_OSD_UNI_BITMAP	Colors;
	WORD	Associate;
	BYTE	Direction;
} gmt_OSD_BITMAPSLIDER_CTRL;



End block defined in libs*/

/*

Begin block defined in libs

typedef struct gmt_OSD_RECTANGLEBUTTON_CTRLStruct
{
	BYTE	TileId;
	WORD	XPos;
	WORD	YPos;
	WORD	XSize;
	WORD	YSize;
	BYTE	EraseColorIndex;
	BYTE ROM *	Colors;
	BYTE	BorderWidth;
} gmt_OSD_RECTANGLEBUTTON_CTRL;



End block defined in libs*/

typedef struct ST_OSD_SETSYSADJBYTE_PARAM_CTRLStruct
{
	SM_PARAM_TYPE_ADJUSTER_ID	AdjusterID;
	SM_PARAM_TYPE_BYTE	Value;
} ST_OSD_SETSYSADJBYTE_PARAM_CTRL;

typedef struct ST_OSD_SETADJSBYTE_PARAM_CTRLStruct
{
	SM_PARAM_TYPE_ADJUSTER_ID	AdjusterID;
	SM_PARAM_TYPE_SBYTE	Value;
} ST_OSD_SETADJSBYTE_PARAM_CTRL;

typedef struct ST_OSD_SETSYSADJSBYTE_PARAM_CTRLStruct
{
	SM_PARAM_TYPE_ADJUSTER_ID	AdjusterID;
	SM_PARAM_TYPE_SBYTE	Value;
} ST_OSD_SETSYSADJSBYTE_PARAM_CTRL;

typedef struct ST_OSD_SETADJBYTE_PARAM_CTRLStruct
{
	SM_PARAM_TYPE_ADJUSTER_ID	AdjusterID;
	SM_PARAM_TYPE_BYTE	Value;
} ST_OSD_SETADJBYTE_PARAM_CTRL;

typedef struct ST_OSD_SETSYSADJSWORD_PARAM_CTRLStruct
{
	SM_PARAM_TYPE_ADJUSTER_ID	AdjusterID;
	SM_PARAM_TYPE_SWORD	Value;
} ST_OSD_SETSYSADJSWORD_PARAM_CTRL;

typedef struct ST_OSD_SETADJWORD_PARAM_CTRLStruct
{
	SM_PARAM_TYPE_ADJUSTER_ID	AdjusterID;
	SM_PARAM_TYPE_WORD	Value;
} ST_OSD_SETADJWORD_PARAM_CTRL;

typedef struct ST_OSD_SWITCHLOGICALADDRESSES_PARAM_CTRLStruct
{
	SM_PARAM_TYPE_BYTE	param1;
	SM_PARAM_TYPE_BYTE	param2;
} ST_OSD_SWITCHLOGICALADDRESSES_PARAM_CTRL;

////////////////////////////////////////////////////////////////////////////////
//    Defines Control Types used in OSD
////////////////////////////////////////////////////////////////////////////////

#define CONTROLTYPE_INVISIBLESTATE_USED 
#define CONTROLTYPE_TILE_USED 
#define CONTROLTYPE_BITMAP_USED 
#define CONTROLTYPE_RECTANGLE_USED 
#define CONTROLTYPE_TEXT_USED 
#define CONTROLTYPE_TEXTBUTTON_USED 
#define CONTROLTYPE_SLIDERBUTTON_USED 
#define CONTROLTYPE_LONGVALUE_USED 
#define CONTROLTYPE_DSTATICTEXTBUTTON_USED 
#define CONTROLTYPE_LONGVALUEBUTTON_USED 
#define CONTROLTYPE_BITMAPBUTTON_USED 
#define CONTROLTYPE_TIMINGINFO_USED 
#define CONTROLTYPE_DSTATICTEXT_USED 
#define CONTROLTYPE_DIALOG_USED 
#define CONTROLTYPE_TIMELONGVALUE_USED 
#define CONTROLTYPE_DSTATICTEXTEXT_USED 
#define CONTROLTYPE_DYNAMICTEXT_USED 
#define CONTROLTYPE_BITMAPSLIDER_USED 
#define CONTROLTYPE_RECTANGLEBUTTON_USED 
#define STRUCTURETYPE_SETSYSADJBYTE_PARAM_USED 
#define STRUCTURETYPE_SETADJSBYTE_PARAM_USED 
#define STRUCTURETYPE_SETSYSADJSBYTE_PARAM_USED 
#define STRUCTURETYPE_SETADJBYTE_PARAM_USED 
#define STRUCTURETYPE_SETSYSADJSWORD_PARAM_USED 
#define STRUCTURETYPE_SETADJWORD_PARAM_USED 
#define STRUCTURETYPE_SWITCHLOGICALADDRESSES_PARAM_USED 

#ifdef __PARADIGM__
#pragma pack(pop)
#endif //__PARADIGM__


#endif  // ifndef OSD_CTYPE_H_DEFINED
