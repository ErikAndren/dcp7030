///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
//	Source filename: brother_cmatch.c
//
//		Copyright(c) 1997-2000 Brother Industries, Ltd.  All Rights Reserved.
//
//
//	Abstract:
//			���顼�ޥå��󥰽����⥸�塼��
//
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


#include <dlfcn.h>

#include "brother.h"
#include "brother_misc.h"
#include "brother_log.h"

#include "brother_cmatch.h"

//
// ColorMatch DLL̾
//
static char  szColorMatchDl[] = "libbrcolm.so";

extern HANDLE	hOrg;		//Gray Adjsut table from scanner

//-----------------------------------------------------------------------------
//
//	Function name:	LoadColorMatchDll
//
//
//	Abstract:
//		ColorMatch DLL����ɤ����ƴؿ��ؤΥݥ��󥿤��������
//
//
//	Parameters:
//		�ʤ�
//
//
//	Return values:
//		TRUE  = ���ｪλ
//		FALSE = ColorMatch DLL��¸�ߤ��ʤ������顼ȯ��
//
//-----------------------------------------------------------------------------
//	LoadColorMatchDll�ʵ�DllMain�ΰ�����
BOOL
LoadColorMatchDll( Brother_Scanner *this )
{
	BOOL  bResult = TRUE;


	this->cmatch.hColorMatch = dlopen ( szColorMatchDl, RTLD_LAZY );

	if( this->cmatch.hColorMatch != NULL ){
		//
		// ColorMatch�ե��󥯥���󡦥ݥ��󥿤μ���
		//
		this->cmatch.lpfnColorMatchingInit    = dlsym ( this->cmatch.hColorMatch, "ColorMatchingInit" );
		this->cmatch.lpfnColorMatchingEnd     = dlsym ( this->cmatch.hColorMatch, "ColorMatchingEnd" );
		this->cmatch.lpfnColorMatchingFnc     = dlsym ( this->cmatch.hColorMatch, "ColorMatching" );
		this->cmatch.nColorMatchStatus = COLORMATCH_NONE;

		if(  this->cmatch.lpfnColorMatchingInit == NULL || 
			 this->cmatch.lpfnColorMatchingEnd  == NULL || 
			 this->cmatch.lpfnColorMatchingFnc  == NULL )
		{
			// DLL�Ϥ��뤬�����ɥ쥹�����ʤ��Τϰ۾�
			dlclose ( this->cmatch.hColorMatch );
			this->cmatch.hColorMatch = NULL;
			bResult = FALSE;
		}
	}else{
		this->cmatch.lpfnColorMatchingInit    = NULL;
		this->cmatch.lpfnColorMatchingEnd     = NULL;
		this->cmatch.lpfnColorMatchingFnc     = NULL;
		bResult = FALSE;
	}
	return bResult;
}


//-----------------------------------------------------------------------------
//
//	Function name:	FreeColorMatchDll
//
//
//	Abstract:
//		ColorMatch DLL��������
//
//
//	Parameters:
//		�ʤ�
//
//
//	Return values:
//		�ʤ�
//
//-----------------------------------------------------------------------------
//
void
FreeColorMatchDll( Brother_Scanner *this )
{
	if( this->cmatch.hColorMatch != NULL ){
		//
		// ColorMatch DLL��������
		//
		dlclose ( this->cmatch.hColorMatch );
		this->cmatch.hColorMatch = NULL;
	}
}

//-----------------------------------------------------------------------------
//
//	Function name:	InitColorMatchingFunc
//
//
//	Abstract:
//		ColorMatch��������������
//
//
//	Parameters:
//		this
//		nColorType
//		nRgbDataType
//
//	Return values:
//		TRUE  = ���ｪλ
//		FALSE = ColorMatch DLL��¸�ߤ��ʤ������顼ȯ��
//
//-----------------------------------------------------------------------------
//	InitColorMatchingFunc�ʵ�PageTopProcess�ΰ�����
void
InitColorMatchingFunc( Brother_Scanner *this, WORD nColorType, int nRgbDataType )
{
	CMATCH_INIT  CMatchInit;
	BOOL  bCInitResult;
	char  szLutFilePathName[ MAX_PATH ];


	if( this->cmatch.lpfnColorMatchingInit == NULL || this->modelConfig.bNoUseColorMatch ){
		//
		// ColorMatch DLL��¸�ߤ��ʤ�����ColorMatch��ɬ�פʤ�
		//
		this->cmatch.nColorMatchStatus = COLORMATCH_NONE;
	}else{
		if( nColorType == COLOR_FUL ){
			if( this->cmatch.nColorMatchStatus == COLORMATCH_GOOD ){
				//
				// �ƽ������Ԥ�����˽�λ������¹�
				//
				(*this->cmatch.lpfnColorMatchingEnd)();
			}
			CMatchInit.nRgbLine      = nRgbDataType;
			CMatchInit.nPaperType    = MEDIA_PLAIN;
			CMatchInit.nMachineId    = 0;
			strcpy(szLutFilePathName, BROTHER_SANE_DIR);
			strcat(szLutFilePathName, BROTHER_GRAYCMDATA_DIR); 
			strcat(szLutFilePathName, this->modelConfig.szColorMatchName);
			CMatchInit.lpLutName     = szLutFilePathName;

			bCInitResult = (*this->cmatch.lpfnColorMatchingInit)( CMatchInit );

			if( bCInitResult == TRUE ){
				this->cmatch.nColorMatchStatus = COLORMATCH_GOOD;
				WriteLog( "ColorMatch initialize complete" );
			}else{
				this->cmatch.nColorMatchStatus = COLORMATCH_NG;
				WriteLog( "ColorMatch initialize fail" );
			}
		}else{
			//
			// ColorMatch��Ԥ�ɬ�פ��ʤ�
			//
			this->cmatch.nColorMatchStatus = COLORMATCH_NONE;
		}
	}
}


//-----------------------------------------------------------------------------
//
//	Function name:	ExecColorMatchingFunc
//
//
//	Abstract:
//		ColorMatch������¹Ԥ���
//
//
//	Parameters:
//		this
//			Brother_Scanner��¤�ΤΥݥ���
//		lpRgbData
//			RGB�ǡ����ؤΥݥ���
//
//		lRgbDataLen
//			RGB�ǡ����Υ�����
//
//		lLineCount
//			RGB�ǡ����Υ饹����
//
//
//	Return values:
//		�ʤ�
//
//-----------------------------------------------------------------------------
//	ExecColorMatchingFunc�ʵ�ProcessMain�ΰ�����
void
ExecColorMatchingFunc( Brother_Scanner *this, LPBYTE lpRgbData, long lRgbDataLen, long lLineCount )
{
	if( this->cmatch.lpfnColorMatchingFnc != NULL && this->cmatch.nColorMatchStatus == COLORMATCH_GOOD ){
		if( (*this->cmatch.lpfnColorMatchingFnc)( lpRgbData, lRgbDataLen, lLineCount ) == FALSE ){
			WriteLog( "ColorMatching fail" );
		}
	}
}


//-----------------------------------------------------------------------------
//
//	Function name:	CloseColorMatchingFunc
//
//
//	Abstract:
//		ColorMatch������λ����
//
//
//	Parameters:
//		�ʤ�
//
//
//	Return values:
//		�ʤ�
//
//-----------------------------------------------------------------------------
//	CloseColorMatchingFunc�ʵ�DRV_PROC�ΰ�����
void
CloseColorMatchingFunc( Brother_Scanner *this )
{
	if( this->cmatch.lpfnColorMatchingEnd != NULL ){
		if( this->cmatch.nColorMatchStatus == COLORMATCH_GOOD ){
			(*this->cmatch.lpfnColorMatchingEnd)();
		}
	}
	this->cmatch.nColorMatchStatus = COLORMATCH_NONE;
}


//-----------------------------------------------------------------------------
//
//	Function name:	LoadGrayTable
//
//
//	Abstract:
//		GrayTable����ɤ���
//
//
//	Parameters:
//		�ʤ�
//
//
//	Return values:
//		TRUE  = ���ｪλ
//		FALSE = GrayTable��¸�ߤ��ʤ������顼ȯ��
//
//-----------------------------------------------------------------------------
//	LoadGrayTable�ʵ�OpenDS�ΰ�����
BOOL
LoadGrayTable( Brother_Scanner *this, BYTE GrayTableNo )
{
	char      szGrayBinPathName[ MAX_PATH ];
	FILE      *hGrayBinFile;
	LONG      lFilePtr;
	size_t       nReadSize = 0;
	LPBYTE    lpGrayTable;
	BOOL      bResult = FALSE;


	strcpy( szGrayBinPathName, BROTHER_SANE_DIR );
	strcat( szGrayBinPathName, BROTHER_GRAYCMDATA_DIR); 
	strcat( szGrayBinPathName, this->modelConfig.szGrayLebelName);

	if( GrayTableNo > 0 ){
		hGrayBinFile = fopen( szGrayBinPathName, "rb" );
		if( hGrayBinFile != NULL ){

			this->cmatch.hGrayTbl = MALLOC( 256 );
			if( this->cmatch.hGrayTbl != NULL ){

				lpGrayTable = (LPBYTE)this->cmatch.hGrayTbl;
				lFilePtr = ( GrayTableNo - 1 ) * 512;

				nReadSize = fread( lpGrayTable, 1, 256, hGrayBinFile );
				if( nReadSize == 256 ){
					bResult = TRUE;
				}else{
					nReadSize = 0;
				}
			}
			fclose( hGrayBinFile );

			if( nReadSize <= 0 ){
				if ( this->cmatch.hGrayTbl) {
					FREE( this->cmatch.hGrayTbl );
					this->cmatch.hGrayTbl=NULL;
				}
			}
		}
	}
	return bResult;
}


//-----------------------------------------------------------------------------
//
//	Function name:	FreeGrayTable
//
//
//	Abstract:
//		GrayTable��������
//
//
//	Parameters:
//		�ʤ�
//
//
//	Return values:
//		�ʤ�
//
//-----------------------------------------------------------------------------
//
void
FreeGrayTable( Brother_Scanner *this )
{
	if( this->cmatch.hGrayTbl != NULL ){
		//
		// GrayTable��������
		//
		FREE( this->cmatch.hGrayTbl );
		this->cmatch.hGrayTbl = NULL;
	}
}


//-----------------------------------------------------------------------------
//
//	Function name:	SetupGrayAdjust
//
//
//	Abstract:
//		���Ĵ����Brightness/Contrast���ѥơ��֥�γ��ݤȽ����
//
//
//	Parameters:
//		�ʤ�
//
//
//	Return values:
//		���Ĵ���ѥơ��֥�Υϥ�ɥ�
//
//-----------------------------------------------------------------------------
//	SetupGrayAdjust�ʵ�SetBitmapInfo�ΰ�����
HANDLE
SetupGrayAdjust( Brother_Scanner *this )
{
	HANDLE   hGrayAdj;
	LPBYTE   lpGray;
	LPBYTE   lpGrayTbl;
	int      nBrightness;
	int      nContrast;
	LONG     lGrayVal;
	LONG     lIndex;

	hGrayAdj = MALLOC(256); 
	if( hGrayAdj ){
		lpGray = (LPBYTE)hGrayAdj;

		WriteLog( "malloc hGrayAdj, size = 256" );

		nBrightness = this->uiSetting.nBrightness;
		nContrast   = this->uiSetting.nContrast;

		if( this->cmatch.hGrayTbl == NULL ){
			//
			// GrayTable�ե����뤬¸�ߤ��ʤ�
			//
			for( lIndex = 0; lIndex < 256; lIndex++ ){
				lGrayVal = (LONG)lIndex;
				if( nContrast ){
					lGrayVal = AdjustContrast( lGrayVal, nContrast );
				}
				if( nBrightness ){
					lGrayVal = AdjustBright( lGrayVal, nBrightness );
				}
				if( lGrayVal > 255 ){
					lGrayVal = 255;
				}else if( lGrayVal < 0 ){
					lGrayVal = 0;
				}
				*lpGray++ = (BYTE)lGrayVal;
			}
		}else{
			lpGrayTbl = (LPBYTE)this->cmatch.hGrayTbl;

			for( lIndex = 0; lIndex < 256; lIndex++ ){
				lGrayVal = (LONG)*lpGrayTbl++;
				if( nContrast ){
					lGrayVal = AdjustContrast( lGrayVal, nContrast );
				}
				if( nBrightness ){
					lGrayVal = AdjustBright( lGrayVal, nBrightness );
				}
				if( lGrayVal > 255 ){
					lGrayVal = 255;
				}else if( lGrayVal < 0 ){
					lGrayVal = 0;
				}
				*lpGray++ = (BYTE)lGrayVal;
			}
		}
	}else{
		//
		// ������ݤ˼��Ԥ����Τǡ�Brightness/ContrastĴ����̵��
		//
		hGrayAdj = this->cmatch.hGrayTbl;
	}
	return hGrayAdj;
}


LONG
AdjustContrast( LONG lGrayVal, int sdc )
{
	LONG  lGamma;

	lGamma = (LONG)sdc;
	lGamma <<= 2;

	if( lGamma >= 0 ){
		lGrayVal = ( ( 255 * ( lGrayVal - 128 ) ) / ( 255 - lGamma ) ) + 128;
	}else{
		lGrayVal = ( ( ( 255 + lGamma ) * ( lGrayVal - 128 ) ) / 255 ) + 128;
	}
	return lGrayVal;
}


LONG
AdjustBright( LONG lGrayVal, int sdc )
{
	LONG  lGamma;

	lGamma = (LONG)sdc;
	lGamma <<= 1;
	lGrayVal += lGamma;

	return lGrayVal;
}


//////// end of DsCMatch.c ////////
