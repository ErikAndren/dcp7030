///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
//	Source filename: brother_devinfo.c
//
//		Copyright(c) 1997-2000 Brother Industries, Ltd.  All Rights Reserved.
//
//
//	Abstract:
//			�ǥХ�����������⥸�塼��
//
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <pthread.h>

#include "brother.h"

#include "brother_cmatch.h"
#include "brother_mfccmd.h"
#include "brother_devaccs.h"
#include "brother_misc.h"
#include "brother_log.h"

#include "brother_devinfo.h"

#define QUERYTIMEOUY 5000	// 5000ms
//-----------------------------------------------------------------------------
//
//	Function name:	ExecQueryThread
//
//
//	Abstract:
//		�ǥХ��������������åɤμ¹�
//
//
//	Parameters:
//		lpQueryProc
//			����å��Ѵؿ��ؤΥݥ���
//
//		lpQueryPara
//			����å��Ѵؿ����Ϥ��ѥ�᡼��
//
//
//	Return values:
//		TRUE  = ���ｪλ
//		FALSE = �ǥХ�������μ����˼���
//
//-----------------------------------------------------------------------------
//
BOOL
ExecQueryThread( Brother_Scanner *this, void *lpQueryProc)
{
	pthread_t   tid;
        
        /*  ��������å�����  */
	if (pthread_create(&tid,NULL, lpQueryProc,(void*)this))
		return TRUE;

        /*  ��������åɽ�λ�Ԥ� */	
	if (pthread_join(tid,NULL)) 
		return TRUE;

	return TRUE;
}
// #define FOR_THREAD
//-----------------------------------------------------------------------------
//
//	Function name:	QueryDeviceInfo
//
//
//	Abstract:
//		�ǥХ����Υ�����ʡ��ӥǥ�ǽ�Ͼ�����������
//
//
//	Parameters:
//		�ʤ�
//
//
//	Return values:
//		TRUE  = �ǥХ�������μ����������ʥǥХ�������ξ�����Ǽ��
//		FALSE = �ǥХ�������μ����˼��ԡʥǥե���Ȥξ�����Ǽ��
//
//-----------------------------------------------------------------------------
//	QueryDeviceInfo�ʵ�sendQ��
BOOL
QueryDeviceInfo( Brother_Scanner *this )
{
	BOOL  bResult = FALSE;


	WriteLog( "Query device info" );

	if( this->mfcModelInfo.bQcmdEnable ){
		//
		// Q���ޥ�ɼ¹�
		//
#ifdef FOR_THREAD
		bResult = ExecQueryThread( this, QCommandProc );
#else
		bResult = QCommandProc( this );
#endif
	}

	if( bResult == FALSE )
		return bResult;

	if( this->mfcDeviceInfo.nColorType.val == 0 ){
		if( this->mfcModelInfo.bColorModel ){
			//
			// BY/EUR�ˤϤ��������äƤ��ʤ��Τǡ�default=BY�򥻥å�
			//
			this->mfcDeviceInfo.nColorType.val = MFCDEVINFCOLOR;
		}else{
			//
			// YL1��YL3/USA�ޤǤˤϤ��������äƤ��ʤ��Τǡ�default=YL�򥻥å�
			// PC������ʡ����ӥǥ�����ץ�����ͽ� '99��ǥ� ��
			// Q���ޥ�ɤν��ϥǡ����ե����ޥåȤ�ColorType�ι��
			// YL:00000111B �ȵ��Ҥ����뤬������ϸ���YL3/USA�ޤ�
			// �μµ�����ϥ����֤äƤ���
			//
			this->mfcDeviceInfo.nColorType.val = MFCDEVINFMONO;
		}
	}

	return bResult;
}

//-----------------------------------------------------------------------------
//
//	Function name:	QueryScannerInfo
//
//
//	Abstract:
//		�桼����������٤��Ф���ǥХ����Υ�����ʾ�����������
//
//
//	Parameters:
//		nDevPort
//			�ǥХ����μ��̻�
//
//		lpTwDev
//			DataSrouce�������ؤΥݥ���
//
//
//	Return values:
//		TRUE  = ������ʾ���μ����������ʥǥХ�������ξ�����Ǽ��
//		FALSE = ������ʾ���μ����˼��ԡʥǥե���Ȥξ�����Ǽ��
//
//-----------------------------------------------------------------------------
//
BOOL
QueryScannerInfo( Brother_Scanner *this )
{
	BOOL   bResult = FALSE;


	WriteLog( "Query scanner info" );

	//
	// �ǥХ�����PC�������ץ�ȥ��뤬2000ǯ�ǰʹߤǡ�
	// �������䤤��碌�򥵥ݡ��Ȥ��Ƥ�����ϡ�I���ޥ�ɼ¹�
	//
#ifdef FOR_THREAD
	bResult = ExecQueryThread( this, QueryScanInfoProc);
#else
	bResult = QueryScanInfoProc( this );
#endif

	if( bResult == FALSE ){
		//
		// I���ޥ�ɼ��ԡ�I���ޥ�ɤ����ݡ��Ȥ���Ƥ��ʤ����
		// ������ʾ���Υǥե�����ͤ�����
		//
		CnvResoNoToDeviceResoValue( this, this->devScanInfo.wResoType, this->devScanInfo.wColorType );
		SetDefaultScannerInfo( this );
	}
	return bResult;
}
//-----------------------------------------------------------------------------
//
//	Function name:	SetDefaultScannerInfo
//
//
//	Abstract:
//		������ʡ��ѥ�᡼���θ����ͤ�����
//
//
//	Parameters:
//		�ʤ�
//
//
//	Return values:
//		�ʤ�
//
//
//	Note:
//		YL1/YL2/YL3,BY1/BY2/BY2FB,ZL/ZLFB��I���ޥ�ɤ��б����Ƥ��ʤ��Τǡ�
//		�����͡ʥǥե���ȡˤ���Ѥ���
//		I���ޥ�ɼ��Ԥξ���ǥե�����ͤ�١����Ȥ���
//
//-----------------------------------------------------------------------------
//
void
SetDefaultScannerInfo( Brother_Scanner *this )
{
	WriteLog( "Set default scanner info" );
	//
	// ������󥽡����� ADF �Ȥ���
	this->devScanInfo.wScanSource = MFCSCANSRC_ADF;

	//
	// �ɤ߼���������0.1mmñ�̡�
	this->devScanInfo.dwMaxScanWidth = MFCSCANMAXWIDTH;

	//
	// �ɤ߼�����ɥåȿ�
	switch( this->devScanInfo.DeviceScan.wResoX ){
		case 100:
			this->devScanInfo.dwMaxScanPixels = MFCSCAN200MAXPIXEL / 2;
			break;

		case 150:
			this->devScanInfo.dwMaxScanPixels = MFCSCAN300MAXPIXEL / 2;
			break;

		case 200:
			this->devScanInfo.dwMaxScanPixels = MFCSCAN200MAXPIXEL;
			break;

		case 300:
			this->devScanInfo.dwMaxScanPixels = MFCSCAN300MAXPIXEL;
			break;
	}

	//
	// FB�ɤ߼�����Ĺ��0.1mmñ�̡�
	this->devScanInfo.dwMaxScanHeight = MFCSCANFBHEIGHT;

	//
	// FB�ɤ߼�����饹����
	switch( this->devScanInfo.DeviceScan.wResoY ){
		case 100:
			this->devScanInfo.dwMaxScanRaster = MFCSCAN400FBRASTER / 4;
			break;

		case 200:
			this->devScanInfo.dwMaxScanRaster = MFCSCAN400FBRASTER / 2;
			break;

		case 400:
			this->devScanInfo.dwMaxScanRaster = MFCSCAN400FBRASTER;
			break;

		case 150:
			this->devScanInfo.dwMaxScanRaster = MFCSCAN600FBRASTER / 4;
			break;

		case 300:
			this->devScanInfo.dwMaxScanRaster = MFCSCAN600FBRASTER / 2;
			break;

		case 600:
			this->devScanInfo.dwMaxScanRaster = MFCSCAN600FBRASTER;
			break;
	}
}
//-----------------------------------------------------------------------------
//
//	Function name:	QCommandProc
//
//
//	Abstract:
//		Q-���ޥ�ɽ����ʥ���å��Ѵؿ���
//
//
//	Parameters:
//		lpParameter
//			̤����
//
//
//	Return values:
//		TRUE  = ���ｪλ
//		FALSE = �ǥХ���ǽ�Ͼ���Υ꡼�ɼ���
//
//-----------------------------------------------------------------------------
//	QCommandProc�ʵ�SendQ_do��
DWORD
QCommandProc( void *lpParameter )
{
	Brother_Scanner *this;
	BOOL bResult = FALSE;
	DWORD  dwQcmdTimeOut;
	
	int nReadSize;
	char *pReadBuf;

	this=(Brother_Scanner *)lpParameter;

	WriteLog( "Start Q-command proc" );

	//
	// Q-���ޥ�ɤ�ȯ��
	//
	WriteDeviceCommand( this->hScanner, MFCMD_QUERYDEVINFO, strlen( MFCMD_QUERYDEVINFO ) );

	//
	// �����ॢ���Ȼ��֤�����
	//
	dwQcmdTimeOut = QUERYTIMEOUY ;

	nReadSize = sizeof( MFCDEVICEHEAD ) + sizeof( MFCDEVICEINFO );
	pReadBuf = MALLOC( nReadSize + 0x100);
	if (!pReadBuf)
		return FALSE;

	if (ReadNonFixedData( this->hScanner, pReadBuf, nReadSize + 0x100, dwQcmdTimeOut )){
		this->mfcDevInfoHeader.wDeviceInfoID = (*(WORD *)pReadBuf);
		this->mfcDevInfoHeader.nInfoSize = *(BYTE *)(pReadBuf+2);
		this->mfcDevInfoHeader.nProtcolType = *(BYTE *)(pReadBuf+3);

		memset( &this->mfcDeviceInfo, 0, sizeof( MFCDEVICEINFO ) );
		this->mfcDeviceInfo.nColorType.val = *(BYTE *)(pReadBuf+5); 
		this->mfcDeviceInfo.nHardwareVersion = *(BYTE *)(pReadBuf+13); 
		this->mfcDeviceInfo.nMainScanDpi = *(BYTE *)(pReadBuf+14); 
		this->mfcDeviceInfo.nPaperSizeMax = *(BYTE *)(pReadBuf+15); 
		bResult=TRUE;
	}	
	else {
		//
		// Q���ޥ�ɱ����Υ꡼�ɼ���
		//
		WriteLog( "SENDQ : read err@timeout " );
	}

	FREE( pReadBuf );

	return bResult;
}


//-----------------------------------------------------------------------------
//
//	Function name:	QueryScanInfoProc
//
//
//	Abstract:
//		I-���ޥ�ɽ����ʥ���å��Ѵؿ���
//		�桼����������٤��Ф���ǥХ����Υ�����ʾ�����������
//
//
//	Parameters:
//		lpParameter
//			�ǥХ����μ��̻�
//
//
//	Return values:
//		TRUE  = ���ｪλ
//		FALSE = ����������Υ꡼�ɼ���
//
//-----------------------------------------------------------------------------
//
DWORD
QueryScanInfoProc(
	void *lpParameter )
{
	Brother_Scanner *this;
	char  szCmdStr[ MFCMAXCMDLENGTH ];
	int   CmdLength;
	BOOL  bResult = FALSE;
	DWORD  dwQcmdTimeOut;
	WORD   wReadSize;
	int nReadSize, nRealReadSize;
	char *pReadBuf;

	this=(Brother_Scanner *)lpParameter;

	WriteLog( "Start I-command proc" );

	this = (Brother_Scanner *)lpParameter;

	//
	// I-���ޥ�ɤ�ȯ��
	//
	CmdLength = MakeupScanQueryCmd( this, szCmdStr );
	WriteLogScanCmd( "Write", szCmdStr );
	WriteDeviceCommand( this->hScanner, szCmdStr, CmdLength );

	//
	// �����ॢ���Ȼ��֤�����
	//
	dwQcmdTimeOut = QUERYTIMEOUY;

	nReadSize = 100;
	pReadBuf = MALLOC( nReadSize + 0x100 );
	if (!pReadBuf)
		return FALSE;

	nRealReadSize = ReadNonFixedData( this->hScanner, pReadBuf, nReadSize + 0x100, dwQcmdTimeOut );
	if ( nRealReadSize < 2) {
		//
		// I���ޥ�ɱ����Υ꡼�ɼ���
		//
		WriteLog( "SENDI : read err@timeout [%d]", nRealReadSize );
	}
	else {
		LPSTR  lpDataBuff;
 
		lpDataBuff = pReadBuf+2; // ���������ΰ�ʬ���ݥ��󥿤�ʤ�롣
		wReadSize = nRealReadSize-2;
		//
		// ������ʾ���Υ꡼��
		//
		bResult = TRUE;
		// �ǡ����ν�����Zero���ɲä���ʸ����Ȥ��ư�����褦�ˤ���
		*( lpDataBuff + wReadSize ) = '\0';
		WriteLog( "  Response is [%s]", lpDataBuff );
		//
		// ������󤹤�²����٤μ���
		this->devScanInfo.DeviceScan.wResoX = StrToWord( GetToken( &lpDataBuff ) );
		this->devScanInfo.DeviceScan.wResoY = StrToWord( GetToken( &lpDataBuff ) );
		if( this->devScanInfo.DeviceScan.wResoX == 0 || this->devScanInfo.DeviceScan.wResoY == 0 ){
			//
			// �²����٤��۾���
			bResult = FALSE;
		}
		//
		// ������󥽡����μ���
		this->devScanInfo.wScanSource = StrToWord( GetToken( &lpDataBuff ) );
		//
		// �ɤ߼��������ξ���������0.1mmñ�̡��ɥåȿ���
		this->devScanInfo.dwMaxScanWidth  = StrToWord( GetToken( &lpDataBuff ) ) * 10;
		this->devScanInfo.dwMaxScanPixels = StrToWord( GetToken( &lpDataBuff ) );
		if( this->devScanInfo.dwMaxScanWidth == 0 || this->devScanInfo.dwMaxScanPixels == 0 ){
			//
			// �ɤ߼����������󤬰۾���
			bResult = FALSE;
		}
		//
		// FB�ɤ߼�����Ĺ�ξ���������0.1mmñ�̡��饹������
		this->devScanInfo.dwMaxScanHeight = StrToWord( GetToken( &lpDataBuff ) ) * 10;
		this->devScanInfo.dwMaxScanRaster = StrToWord( GetToken( &lpDataBuff ) );
		
		bResult = TRUE;
	}
	FREE( pReadBuf );
	
	return bResult;
}

//
// �ɤ߼��������б�ɽ
//
//					ZL��			BY��			YL��(BW)		YL��(Gray)
//  100 x  100 dpi	[ 100, 100 ],	{ 200, 100 },	{ 200, 100 },	{ 200, 100 }
//  150 x  150 dpi	{ 150, 150 },	{ 150, 150 },	{ 150, 150 },	{ 150, 150 }
//  200 x  100 dpi	{ 200, 100 },	{ 200, 100 },	{ 200, 100 },	{ 200, 100 }
//  200 x  200 dpi	{ 200, 200 },	{ 200, 200 },	{ 200, 200 },	{ 200, 200 }
//  200 x  400 dpi	{ 200, 400 },	{ 200, 400 },	{ 200, 400 },	{ 200, 400 }
//  300 x  300 dpi	{ 300, 300 },	{ 300, 300 },	{ 300, 300 },	{ 300, 300 }
//  400 x  400 dpi	{ 200, 400 },	{ 200, 400 },	{ 200, 400 },	{ 200, 400 }
//  600 x  600 dpi	[ 300, 600 ],	[ 300, 600 ],	{ 200, 200 },	{ 200, 400 }
//  800 x  800 dpi	{ 200, 400 },	{ 200, 400 },	{ 200, 400 },	{ 200, 400 }
// 1200 x 1200 dpi	{ 300, 600 }	{ 300, 600 }	{ 300, 600 }	{ 300, 600 }

//
// �ǥХ����μ²����٥ơ��֥�
// ZL�ϡ�������100dpi,200dpi,300dpi��ǥ�
//
static RESOLUTION  tblDecScanReso100[] = 
{
	{ 100, 100 },	//  100 x  100 dpi
	{ 150, 150 },	//  150 x  150 dpi
	{ 200, 100 },	//  200 x  100 dpi
	{ 200, 200 },	//  200 x  200 dpi
	{ 200, 400 },	//  200 x  400 dpi
	{ 300, 300 },	//  300 x  300 dpi
	{ 200, 400 },	//  400 x  400 dpi
	{ 300, 600 },	//  600 x  600 dpi
	{ 200, 400 },	//  800 x  800 dpi
	{ 300, 600 },	// 1200 x 1200 dpi
	{ 300, 600 },	// 2400 x 2400 dpi
	{ 300, 600 },	// 4800 x 4800 dpi
	{ 300, 600 }	// 9600 x 9600 dpi
};

//
// �ǥХ����μ²����٥ơ��֥�
// BY/New-YL�ϡ�������200dpi,300dpi��ǥ�
//
static RESOLUTION  tblDecScanReso300[] = 
{
	{ 200, 100 },	//  100 x  100 dpi
	{ 150, 150 },	//  150 x  150 dpi
	{ 200, 100 },	//  200 x  100 dpi
	{ 200, 200 },	//  200 x  200 dpi
	{ 200, 400 },	//  200 x  400 dpi
	{ 300, 300 },	//  300 x  300 dpi
	{ 200, 400 },	//  400 x  400 dpi
	{ 300, 600 },	//  600 x  600 dpi
	{ 200, 400 },	//  800 x  800 dpi
	{ 300, 600 },	// 1200 x 1200 dpi
	{ 300, 600 },	// 2400 x 2400 dpi
	{ 300, 600 },	// 4800 x 4800 dpi
	{ 300, 600 }	// 9600 x 9600 dpi
};

//
// �ǥХ����μ²����٥ơ��֥�
// YL�ϡ�������200dpi��ǥ�ʣ��͡�
//
static RESOLUTION  tblDecScanReso200BW[] = 
{
	{ 200, 100 },	//  100 x  100 dpi
	{ 150, 150 },	//  150 x  150 dpi
	{ 200, 100 },	//  200 x  100 dpi
	{ 200, 200 },	//  200 x  200 dpi
	{ 200, 400 },	//  200 x  400 dpi
	{ 300, 300 },	//  300 x  300 dpi
	{ 200, 400 },	//  400 x  400 dpi
	{ 200, 200 },	//  600 x  600 dpi
	{ 200, 400 },	//  800 x  800 dpi
	{ 200, 400 },	// 1200 x 1200 dpi
	{ 200, 400 },	// 2400 x 2400 dpi
	{ 200, 400 },	// 4800 x 4800 dpi
	{ 200, 400 }	// 9600 x 9600 dpi
};

//
// �ǥХ����μ²����٥ơ��֥�
// YL�ϡ�������200dpi��ǥ��¿�͡�
//
static RESOLUTION  tblDecScanReso200Gray[] = 
{
	{ 200, 100 },	//  100 x  100 dpi
	{ 150, 150 },	//  150 x  150 dpi
	{ 200, 100 },	//  200 x  100 dpi
	{ 200, 200 },	//  200 x  200 dpi
	{ 200, 400 },	//  200 x  400 dpi
	{ 300, 300 },	//  300 x  300 dpi
	{ 200, 400 },	//  400 x  400 dpi
	{ 200, 400 },	//  600 x  600 dpi
	{ 200, 400 },	//  800 x  800 dpi
	{ 200, 400 },	// 1200 x 1200 dpi
	{ 200, 400 },	// 2400 x 2400 dpi
	{ 200, 400 },	// 4800 x 4800 dpi
	{ 200, 400 }	// 9600 x 9600 dpi
};


//-----------------------------------------------------------------------------
//
//	Function name:	CnvResoNoToDeviceResoValue
//
//
//	Abstract:
//		�ǥХ����μ²����٤��������
//
//
//	Parameters:
//		nResoNo
//			�����٥������ֹ�
//
//		nColorType
//			���顼�������ֹ�
//
//
//	Return values:
//		�ʤ�
//
//-----------------------------------------------------------------------------
//
void
CnvResoNoToDeviceResoValue( Brother_Scanner *this, WORD nResoNo, WORD nColorType )
{
	if( nResoNo > RES9600X9600 ){
		nResoNo = RES9600X9600;
	}
	if( !this->mfcModelInfo.bColorModel && ( this->mfcDeviceInfo.nMainScanDpi == 1 ) ){
		//
		// YL�ϡ�������200dpi��ǥ�
		//
		if( nColorType == COLOR_BW || nColorType == COLOR_ED ){
			this->devScanInfo.DeviceScan.wResoX = tblDecScanReso200BW[ nResoNo ].wResoX;
			this->devScanInfo.DeviceScan.wResoY = tblDecScanReso200BW[ nResoNo ].wResoY;
		}else{
			this->devScanInfo.DeviceScan.wResoX = tblDecScanReso200Gray[ nResoNo ].wResoX;
			this->devScanInfo.DeviceScan.wResoY = tblDecScanReso200Gray[ nResoNo ].wResoY;
		}
	}else if( this->mfcDeviceInfo.nMainScanDpi == 2 ){
		//
		// BY/New-YL�ϡ�������200dpi,300dpi��ǥ�
		//
		this->devScanInfo.DeviceScan.wResoX = tblDecScanReso300[ nResoNo ].wResoX;
		this->devScanInfo.DeviceScan.wResoY = tblDecScanReso300[ nResoNo ].wResoY;

	}else{
		//
		// ZL�ϡ�������100dpi,200dpi,300dpi��ǥ�
		//
		this->devScanInfo.DeviceScan.wResoX = tblDecScanReso100[ nResoNo ].wResoX;
		this->devScanInfo.DeviceScan.wResoY = tblDecScanReso100[ nResoNo ].wResoY;
	}
}


//////// end of brother_devinfo.c ////////
