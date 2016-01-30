///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
//	Source filename: brother_cmatch.h
//
//		Copyright(c) 1997-2000 Brother Industries, Ltd.  All Rights Reserved.
//
//
//	Abstract:
//			���顼�ޥå��󥰽����⥸�塼�롦�إå���
//
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifndef _BROTHER_CMATCH_H_
#define _BROTHER_CMATCH_H_

#include "brcolor.h"

#define GRAY_TABLE_NO  1  // 1�Ǹ���

//
// ColorMatch/GrayTable�Υե�����̾
//
#define COLORMATCHDLL  "Brcolm32.dll"
#define COLORMATCHLUT  "BrLutCm.dat"
#define GRAYTABLEFILE  "brmfgray.bin"


//
// ColorMatch�����ξ���
//
#define COLORMATCH_NONE  0
#define COLORMATCH_GOOD  1
#define COLORMATCH_NG    2


//
// �ؿ��Υץ�ȥ��������
//
BOOL     LoadColorMatchDll( Brother_Scanner *this );
void     FreeColorMatchDll( Brother_Scanner *this );
void     InitColorMatchingFunc( Brother_Scanner *this, WORD nColorType, int nRgbDataType );
void     ExecColorMatchingFunc( Brother_Scanner *this, LPBYTE lpRgbData, long lRgbDataLen, long lLineCount );
void     CloseColorMatchingFunc( Brother_Scanner *this );
BOOL     LoadGrayTable( Brother_Scanner *this, BYTE GrayTableNo );
void     FreeGrayTable( Brother_Scanner *this );
HANDLE   SetupGrayAdjust( Brother_Scanner *this );
LONG     AdjustContrast( LONG lGrayVal, int sdc );
LONG     AdjustBright( LONG lGrayVal, int sdc );


#endif //_BROTHER_CMATCH_H_


//////// end of brother_cmatch.h ////////
