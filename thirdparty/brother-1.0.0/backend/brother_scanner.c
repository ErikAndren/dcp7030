///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
//	Source filename: brother_scanner.c
//
//		Copyright(c) 1995-2000 Brother Industries, Ltd.  All Rights Reserved.
//
//
//	Abstract:
//			�����������⥸�塼��
//
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <usb.h>

#include "sane/sane.h"

#include "brother_dtype.h"

#include "brother.h"
#include "brother_cmatch.h"
#include "brother_mfccmd.h"
#include "brother_devaccs.h"
#include "brother_log.h"

#include "brother_scandec.h"
#include "brother_deccom.h"

#include "brother_scanner.h"


#ifdef NO39_DEBUG
#include <sys/time.h>
#endif

//$$$$$$$$
LPBYTE  lpRxBuff = NULL;
LPBYTE  lpRxTempBuff = NULL;
DWORD   dwRxTempBuffLength = 0;
LPBYTE  lpFwTempBuff = NULL;
DWORD	dwFwTempBuffMaxSize= 0;
int     FwTempBuffLength = 0;
BOOL    bTxScanCmd = FALSE;
DWORD	dwRxBuffMaxSize= 0;


LONG lRealY = 0;

HANDLE	hGray;			//Gray table from brgray.bin

//$$$$$$$$

//
// �������ǡ���Ÿ�����������Ѵ��⥸�塼�����ѿ�
//
SCANDEC_OPEN   ImageProcInfo;
SCANDEC_WRITE  ImgLineProcInfo;
DWORD  dwImageBuffSize;
DWORD  dwWriteImageSize;
int    nWriteLineCount;


//
// ScanDec DLL ̾
//
static char  szScanDecDl[] = "libbrscandec.so";

BOOL	bReceiveEndFlg;


// Debug �ѳ����ѿ�
int nPageScanCnt;
int nReadCnt;


DWORD dwFWImageSize;
DWORD dwFWImageLine;
int nFwLenTotal;

//-----------------------------------------------------------------------------
//
//	Function name:	LoadScanDecDll
//
//
//	Abstract:
//		ScanDec DLL����ɤ����ƴؿ��ؤΥݥ��󥿤��������
//
//
//	Parameters:
//		�ʤ�
//
//
//	Return values:
//		TRUE  = ���ｪλ
//		FALSE = ScanDec DLL��¸�ߤ��ʤ������顼ȯ��
//
//-----------------------------------------------------------------------------
//	LoadColorMatchDll�ʵ�DllMain�ΰ�����
BOOL
LoadScanDecDll( Brother_Scanner *this )
{
	BOOL  bResult = TRUE;


	this->scanDec.hScanDec = dlopen ( szScanDecDl, RTLD_LAZY );

	if( this->scanDec.hScanDec != NULL ){
		//
		// scanDec�ե��󥯥���󡦥ݥ��󥿤μ���
		//
		this->scanDec.lpfnScanDecOpen      = dlsym ( this->scanDec.hScanDec, "ScanDecOpen" );
		this->scanDec.lpfnScanDecSetTbl    = dlsym ( this->scanDec.hScanDec, "ScanDecSetTblHandle" );
		this->scanDec.lpfnScanDecPageStart = dlsym ( this->scanDec.hScanDec, "ScanDecPageStart" );
		this->scanDec.lpfnScanDecWrite     = dlsym ( this->scanDec.hScanDec, "ScanDecWrite" );
		this->scanDec.lpfnScanDecPageEnd   = dlsym ( this->scanDec.hScanDec, "ScanDecPageEnd" );
		this->scanDec.lpfnScanDecClose     = dlsym ( this->scanDec.hScanDec, "ScanDecClose" );

		if(  this->scanDec.lpfnScanDecOpen == NULL || 
			 this->scanDec.lpfnScanDecSetTbl  == NULL || 
			 this->scanDec.lpfnScanDecPageStart  == NULL || 
			 this->scanDec.lpfnScanDecWrite  == NULL || 
			 this->scanDec.lpfnScanDecPageEnd  == NULL || 
			 this->scanDec.lpfnScanDecClose  == NULL )
		{
			// DLL�Ϥ��뤬�����ɥ쥹�����ʤ��Τϰ۾�
			dlclose ( this->scanDec.hScanDec );
			this->scanDec.hScanDec = NULL;
			bResult = FALSE;
		}
	}else{
		this->scanDec.lpfnScanDecOpen      = NULL;
		this->scanDec.lpfnScanDecSetTbl    = NULL;
		this->scanDec.lpfnScanDecPageStart = NULL;
		this->scanDec.lpfnScanDecWrite     = NULL;
		this->scanDec.lpfnScanDecPageEnd   = NULL;
		this->scanDec.lpfnScanDecClose     = NULL;
		bResult = FALSE;
	}
	return bResult;
}


//-----------------------------------------------------------------------------
//
//	Function name:	FreeScanDecDll
//
//
//	Abstract:
//		ScanDec DLL��������
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
FreeScanDecDll( Brother_Scanner *this )
{
	if( this->scanDec.hScanDec != NULL ){
		//
		// ColorMatch DLL��������
		//
		dlclose ( this->scanDec.hScanDec );
		this->scanDec.hScanDec = NULL;
	}
}


/********************************************************************************
 *																				*
 *	FUNCTION	ScanStart														*
 *																				*
 *	PURPOSE		������˥󥰳��Ͻ���											*
 *				�꥽�����ޥ͡������ư���£�ģ�򥪡��ץ󤹤롣				*
 *																				*
 *	IN		Brother_Scanner *this							
 *																				*
 *	OUT		̵��																*
 *																				*
 ********************************************************************************/
BOOL
ScanStart( Brother_Scanner *this )
{
	BOOL  bResult;

	int   rc;

	WriteLog( "" );
	WriteLog( ">>>>> Start Scanning >>>>>" );
	WriteLog( "nPageCnt = %d bEOF = %d  iProcessEnd = %d\n",
		this->scanState.nPageCnt, this->scanState.bEOF, this->scanState.iProcessEnd);

#if 1
	// debug for MASU
	dwFWImageSize = 0;
	dwFWImageLine = 0;
	nFwLenTotal = 0;
#endif
	this->scanState.nPageCnt++;
	this->scanState.bReadbufEnd=FALSE;
	this->scanState.bEOF=FALSE;	

	if( this->scanState.nPageCnt == 1){
		int nResoLine;

		bTxScanCmd = FALSE;
		lRealY = 0;		

		this->devScanInfo.wResoType  = this->uiSetting.wResoType;
		this->devScanInfo.wColorType = this->uiSetting.wColorType;
		

#ifndef DEBUG_No39
		/* open and prepare USB scanner handle */
		this->hScanner=usb_open(g_pdev->pdev);
		if (!this->hScanner)
			return SANE_STATUS_IO_ERROR;

		if (usb_claim_interface(this->hScanner, 1))
			return SANE_STATUS_IO_ERROR;

		if (usb_set_configuration(this->hScanner, 1))
			return SANE_STATUS_IO_ERROR;

		// �ǥХ��������ץ�
		rc= OpenDevice(this->hScanner);
		if (!rc)
			return SANE_STATUS_IO_ERROR;
#endif

		CnvResoNoToUserResoValue( &this->scanInfo.UserSelect, this->devScanInfo.wResoType );

		bResult = QueryScannerInfo( this );
		if (!bResult)
			return SANE_STATUS_INVAL;

		GetScanAreaParam( this );


		// ��������ϰϤ���Ƭ�����祹������ϰϤ�Ĺ������礭����票�顼�Ȥ��롣(for FlatBed)
		if ( (this->devScanInfo.wScanSource == MFCSCANSRC_FB) && 
			(this->scanInfo.ScanAreaMm.top >= (LONG)(this->devScanInfo.dwMaxScanHeight - 80)) )
			return SANE_STATUS_INVAL;


		bResult = StartDecodeStretchProc( this );
		if (!bResult)
			return SANE_STATUS_INVAL;

		//
		// ColorMatch����������
		//
		InitColorMatchingFunc( this, this->devScanInfo.wColorType, CMATCH_DATALINE_RGB );

		//
		// Decode/Stretch�����Υڡ�����������
		//
		if( this->devScanInfo.wColorType == COLOR_DTH || this->devScanInfo.wColorType == COLOR_TG ){
			//
			// Brightness/ContrastĴ���Τ���ν���
			//
			if( this->uiSetting.nBrightness == 0 && this->uiSetting.nContrast == 0 ){
				//
				// Brightness/Contrast���˥��󥿡��ͤξ�硢���ꥸ�ʥ��GrayTable����Ѥ���
				//
				hGray = this->cmatch.hGrayTbl;
			}else{
				//
				// Brightness/ContrastĴ���Τ����GrayTable����������
				//
				hGray = SetupGrayAdjust( this );
			}
		}else{
			hGray = NULL;
		}
		
		//
		// �����Хåե�����ݤ���
		//
		
		// ������¸�Хåե��˺���3�饤��ʬ��Ÿ���Ǥ���褦�˥�����ʤ���ǡ����ɤ߹���Хåե�����ݤ��롣
		if (this->devScanInfo.wColorType == COLOR_FUL || this->devScanInfo.wColorType == COLOR_FUL_NOCM )
			dwRxBuffMaxSize = (this->devScanInfo.ScanAreaByte.lWidth + 3) * 3; 
		else
			dwRxBuffMaxSize = (this->devScanInfo.ScanAreaByte.lWidth + 3);

		dwRxBuffMaxSize *= (3 + 1); // ����3�饤��ʬ�ϥ꡼�ɤ��뤿���4�饤��ʬ���ΰ�����
		
		if (dwRxBuffMaxSize < (DWORD)gwInBuffSize)
			dwRxBuffMaxSize = (DWORD)gwInBuffSize;
		
		lpRxBuff = (LPBYTE)AllocReceiveBuffer( dwRxBuffMaxSize  );
		lpRxTempBuff = (LPBYTE)MALLOC( dwRxBuffMaxSize  );

		if( lpRxBuff == NULL || lpRxTempBuff == NULL){
			//
			// �����Хåե��γ��ݤ˼��Ԥ����Τǥ��ꥨ�顼���֤�
			//
			return SANE_STATUS_NO_MEM;
		}

		dwRxTempBuffLength = 0;
		FwTempBuffLength = 0;

		// ������¸�Хåե��Υ������
			
		// ����6�饤��ΥХåե������
		dwFwTempBuffMaxSize = this->scanInfo.ScanAreaByte.lWidth * 6;
		dwFwTempBuffMaxSize *=2;

		// �������Ѵ���ɬ�פʾ�硢���礹��饤��������ޤ��ΰ����ݤ��롣
		nResoLine = this->scanInfo.UserSelect.wResoY / this->devScanInfo.DeviceScan.wResoY;
		if (nResoLine > 1) // �������Ѵ���ɬ�פʾ��
			dwFwTempBuffMaxSize *= nResoLine;

		// gwInBuffSize��꾯�ʤ����ϡ�gwInBuffSize�˹�碌�롣
		if (dwFwTempBuffMaxSize < (DWORD)gwInBuffSize)
			dwFwTempBuffMaxSize = (DWORD)gwInBuffSize;

		lpFwTempBuff = (LPBYTE)MALLOC( dwFwTempBuffMaxSize );
		WriteLog( " dwRxBuffMaxSize = %d, dwFwTempBuffMaxSize = %d, ", dwRxBuffMaxSize, dwFwTempBuffMaxSize );
		if( lpFwTempBuff == NULL ){
			//
			// ������¸�Хåե��γ��ݤ˼��Ԥ����Τǥ��ꥨ�顼���֤�
			//
			return SANE_STATUS_NO_MEM;
		}

		// �ڡ���������󳫻�
		if (!PageScanStart( this )) {
			ScanEnd( this);
			return SANE_STATUS_INVAL;
		}

		this->scanState.bScanning=TRUE;
		this->scanState.bCanceled=FALSE;
		this->scanState.iProcessEnd=0;
	}
	else {
		WriteLog( " PageStart scanState.iProcessEnd = %d, ", this->scanState.iProcessEnd );

		lRealY = 0;		
		dwRxTempBuffLength = 0;
		FwTempBuffLength = 0;

		if (this->devScanInfo.wScanSource == MFCSCANSRC_ADF && this->scanState.iProcessEnd == SCAN_MPS) {
			// ���Υڡ�����������
			// �ڡ���������󳫻�
			this->scanState.iProcessEnd=0;
			if (!PageScanStart( this )) {
				return SANE_STATUS_INVAL;
			}
			bResult = SANE_STATUS_GOOD;
		}
		else if (this->scanState.iProcessEnd == SCAN_EOF){
			// ���Υڡ������ʤ����
			bResult = SANE_STATUS_NO_DOCS;
			return bResult;
		}
		else {
			// ���������˥������ܥ��󤬲������줿���
			bResult = SANE_STATUS_DEVICE_BUSY;
			return bResult;
		}
	}

	// ���̥ǡ�����Ÿ�������γ��Ͻ���
	if (this->scanDec.lpfnScanDecSetTbl != NULL && this->scanDec.lpfnScanDecPageStart != NULL) {
		this->scanDec.lpfnScanDecSetTbl( hGray, NULL );
		bResult = this->scanDec.lpfnScanDecPageStart();
		if (!bResult)
			return SANE_STATUS_INVAL;
		else
			bResult = SANE_STATUS_GOOD;
	}
	else {
		return SANE_STATUS_INVAL;
	}

	nPageScanCnt = 0;	// DEBUG�ѥ����󥿽����

	nReadCnt = 0;	// DEBUG�ѥ����󥿽����

	return bResult;
}


//-----------------------------------------------------------------------------
//
//	Function name:	PageScanStart
//
//
//	Abstract:
//		�ڡ����������γ��Ͻ���
//
//
//	Parameters:
//		hWndDlg
//			Window�ϥ�ɥ�
//
//
//	Return values:
//		�ʤ�
//
//-----------------------------------------------------------------------------
//	PageScanStart�ʵ�PageScan�ΰ�����
BOOL
PageScanStart( Brother_Scanner *this )
{
	BOOL rc=FALSE;

	if( this->scanState.nPageCnt == 1 ){
		//send command only 1st page
		if( !bTxScanCmd ){
			//
			// Scan���ϥ��ޥ�ɤ���������Ƥ��ʤ�
			//
			char  szCmdStr[ MFCMAXCMDLENGTH ];
			int   nCmdStrLen;

			//
			// ���ޥ�������ե饰�ν����
			//
			bTxScanCmd = TRUE;		// Scan���ϥ��ޥ�������Ѥ�
			bTxCancelCmd = FALSE;	// Cancel���ޥ��̤����

			//
			// Scan���ϥ��ޥ������
			//
			nCmdStrLen = MakeupScanStartCmd( this, szCmdStr );
			if (WriteDeviceCommand( this->hScanner, szCmdStr, nCmdStrLen ))
				rc=TRUE;

			sleep(3);
			WriteLogScanCmd( "Write",  szCmdStr );
		}
	}else if( this->scanState.nPageCnt > 1 ){

		//
		// 2�ڡ����ܹԤΥ�����󳫻ϥ��ޥ��
		//
		if (WriteDeviceCommand( this->hScanner, MFCMD_SCANNEXTPAGE, strlen( MFCMD_SCANNEXTPAGE )))
			rc = TRUE;

		sleep(3);
		WriteLogScanCmd( "Write",  MFCMD_SCANNEXTPAGE );
	}

	return rc;
}


/********************************************************************************
 *										*
 *	FUNCTION	StatusChk						*
 *										*
 *	PURPOSE		���ơ����������ɤ����뤫�����å����롣				*
 *										*
 *	IN		char *lpBuff	�����å�����Хåե�				*
 *			int nDataSize	�Хåե�������				*
 *										*
 *	OUT		̵��							*
 *										*
 *      �����		TURE 	���ơ����������ɤ����Ĥ��ä���				*
 *			FALSE	���ơ����������ɤ����Ĥ���ʤ��ä���			*
 *										*
 ********************************************************************************/
BOOL
StatusChk(char *lpBuff, int nDataSize)
{
	BOOL	rc=FALSE;	
	LPBYTE	pt = lpBuff;	

	while( 1 ) {
		BYTE headch;
		WORD length;

		if( nDataSize <= 0 )	break;	// ���ƤΥǡ����Ͻ�����ǽ(���ڤ��ɤ��������줿)

		headch = (BYTE)*pt;

		if ((char)headch < 0) {
			// STATUS,CTRL�ϥ�����
			// ����header����򻲾�
			
			rc=TRUE;
			WriteLog( ">>> StatusChk header=%2x <<<", headch);
			break;
		}else{
			if (headch == 0) {
				length = 1;
				pt += length;
			}
			else {
				// �����ǡ���

				if( nDataSize < 3 )
					length = 0;		// �����
				else
					// �饹���ǡ���Ĺ�μ���
					length = *(WORD *)( pt + 1 );	// format: [HEADER(1B)][LENGTH(intel 2B)][DATA...]
				
				if( nDataSize < ( length + 3) ){	// length+3 = head(1B)+length(2B)+data(length)
					break;
				}
				else{
					// 1lineʬ�Υǡ�������
					nDataSize -= length + 3;	// �����ǡ����� length+3 byte����
					pt += length + 3;			// ����header����򻲾�
				}
			}
			WriteLog( "Header=%2x  Count=%4d nDataSize=%d", (BYTE)headch, length, nDataSize );
		}
	}
	return rc;
}

#define READ_TIMEOUT 5*60*1000 // 5ʬ

#define NO39_DEBUG


#ifdef NO39_DEBUG
static struct timeval save_tv;		// ���־�����¸�ѿ�(sec, msec)
static struct timezone save_tz;		// ���־�����¸�ѿ�(min)

#endif

/********************************************************************************
 *										*
 *	FUNCTION	PageScan						*
 *										*
 *	PURPOSE		���ڡ���ʬ�򥹥���󤹤롣					*
 *										*
 *	����		Brother_Scanner *this	�� Brother_Scanner��¤��		*
 *			char *lpFwBuf		�� �����Хåե�			*
 *			int nMaxLen		�� �����Хåե�Ĺ			*
 *			int *lpFwLen		�� �����ǡ���Ĺ			*
 *										*
 *										*
 *										*
 ********************************************************************************/
int
PageScan( Brother_Scanner *this, char *lpFwBuf, int nMaxLen, int *lpFwLen )
{
	WORD	wData=0;	// �����ǡ����������ʥХ��ȿ���
	WORD	wDataLineCnt=0;	// �����ǡ����Υ饤���
	int	nAnswer=0;
	int	rc;
	LPBYTE  lpRxTop;
	WORD	wProcessSize;
		
	int	nReadSize;
	LPBYTE  lpReadBuf;
	int	nMinReadSize; // �Ǿ��꡼�ɥ�����

#ifdef NO39_DEBUG
	struct timeval start_tv, tv;
	struct timezone tz;
	long   nSec, nUsec;
#endif
	if (!this->scanState.bScanning) {
		rc = SANE_STATUS_IO_ERROR;
		return rc;
	}
	if (this->scanState.bCanceled) { //����󥻥����
		WriteLog( "Page Canceled" );

		rc = SANE_STATUS_CANCELLED;
		this->scanState.bScanning=FALSE;
		this->scanState.bCanceled=FALSE;
		this->scanState.nPageCnt = 0;

		return rc;
	}

	nPageScanCnt++;
	WriteLog( ">>> PageScan Start <<< cnt=%d nMaxLen=%d\n", nPageScanCnt, nMaxLen);

#ifdef NO39_DEBUG
	if (gettimeofday(&tv, &tz) == 0) {
  					
		if (tv.tv_usec < save_tv.tv_usec) {
			tv.tv_usec += 1000 * 1000 ;
			tv.tv_sec-- ;
		}
		nUsec = tv.tv_usec - save_tv.tv_usec;
		nSec = tv.tv_sec - save_tv.tv_sec;

		WriteLog( " No39 nSec = %d Usec = %d\n", nSec, nUsec ) ;
	}
#endif


	WriteLog( "scanInfo.ScanAreaSize.lWidth = [%d]", this->scanInfo.ScanAreaSize.lWidth );
	WriteLog( "scanInfo.ScanAreaSize.lHeight = [%d]", this->scanInfo.ScanAreaSize.lHeight );
	WriteLog( "scanInfo.ScanAreaByte.lWidth = [%d]", this->scanInfo.ScanAreaByte.lWidth );
	WriteLog( "scanInfo.ScanAreaByte.lHeight = [%d]", this->scanInfo.ScanAreaByte.lHeight );

	WriteLog( "devScanInfo.ScanAreaSize.lWidth = [%d]", this->devScanInfo.ScanAreaSize.lWidth );
	WriteLog( "devScanInfo.ScanAreaSize.lHeight = [%d]", this->devScanInfo.ScanAreaSize.lHeight );
	WriteLog( "devScanInfo.ScanAreaByte.lWidth = [%d]", this->devScanInfo.ScanAreaByte.lWidth );
	WriteLog( "devScanInfo.ScanAreaByte.lHeight = [%d]", this->devScanInfo.ScanAreaByte.lHeight );

	
	memset(lpFwBuf, 0x00, nMaxLen);	//  �����Хåե��򥼥��ꥢ���Ƥ�����
	*lpFwLen = 0;

	if ( (!this->scanState.iProcessEnd) && ( FwTempBuffLength < nMaxLen) ) { 
	// ���ơ����������ɤ�������Ƥ��ʤ����Ǥ��������Хåե����������������¸�Хåե��Υǡ���Ĺ�����������

	// ��¸�ǡ����Хåե��˥ǡ�����¸�ߤ�����ϡ������ǡ����Хåե��˥��ԡ����롣
	memmove( lpRxBuff, lpRxTempBuff, dwRxTempBuffLength );	// ��Ƭ������ǡ�������
	wData += dwRxTempBuffLength;	// ��Ǽ�ǡ���length������

	lpRxTop = lpRxBuff;
	
	// ������¸�Хåե��˺���3�饤��ʬ��Ÿ���Ǥ���褦�˥�����ʤ���ǡ����ɤ߹��ࡣ
	if (this->devScanInfo.wColorType == COLOR_FUL || this->devScanInfo.wColorType == COLOR_FUL_NOCM )
		nMinReadSize = (this->devScanInfo.ScanAreaByte.lWidth + 3) * 3; 
	else
		nMinReadSize = (this->devScanInfo.ScanAreaByte.lWidth + 3);

	nMinReadSize *= 3; // ����3�饤��ʬ�ϥ꡼�ɤ��롣
	if ( !this->scanState.bReadbufEnd ) {
		for (rc=0 ; wData < nMinReadSize;)
		{
			nReadSize = dwRxBuffMaxSize - (dwRxTempBuffLength + wData);
			lpReadBuf = lpRxTop+wData;

			nReadCnt++;
			WriteLog( "Read request size is %d, (dwRxTempBuffLength = %d)", gwInBuffSize - dwRxTempBuffLength, dwRxTempBuffLength );
			WriteLog( "PageScan ReadNonFixedData Cnt = %d", nReadCnt );
			
			rc = ReadNonFixedData( this->hScanner, lpReadBuf, nReadSize, READ_TIMEOUT );
			if (rc < 0) {
				this->scanState.bReadbufEnd = TRUE;
				WriteLog( "  bReadbufEnd =TRUE" );
				break;
			}
			else if (rc > 0){
				wData += rc;

				if (StatusChk(lpRxBuff, wData)) { // ���ơ����������ɤ���������������å����롣
					this->scanState.bReadbufEnd = TRUE;
					WriteLog( "bReadbufEnd =TRUE" );
					break;
				}
			}
		}
	}

	WriteLog( "Read data size is %d, (dwRxTempBuffLength = %d)", wData, dwRxTempBuffLength );

	WriteLog( "Adjusted wData = %d, (dwRxTempBuffLength = %d)", wData, dwRxTempBuffLength );

	if (wData != 0)
	// �ǡ�����饤��ñ�̤ޤǤ˶��ڤ�
	{
	LPBYTE  pt = lpRxBuff;
	int nFwTempBuffMaxLine;
	int nResoLine;

	// �������륤�᡼���ǡ�������(�ɥå�)
	if (this->devScanInfo.wColorType == COLOR_FUL || this->devScanInfo.wColorType == COLOR_FUL_NOCM ) {
 		nFwTempBuffMaxLine = (dwFwTempBuffMaxSize / 2 - FwTempBuffLength) / this->scanInfo.ScanAreaByte.lWidth;
		nFwTempBuffMaxLine *= 3; 
	}
	else {
	 	nFwTempBuffMaxLine = (dwFwTempBuffMaxSize / 2 - FwTempBuffLength) / this->scanInfo.ScanAreaByte.lWidth;
	}
	nResoLine= this->scanInfo.UserSelect.wResoY / this->devScanInfo.DeviceScan.wResoY;
	if (nResoLine > 1)
		nFwTempBuffMaxLine /= nResoLine;

	dwRxTempBuffLength = wData;
	for (wDataLineCnt=0; wDataLineCnt < nFwTempBuffMaxLine;){
		BYTE headch;

		if( dwRxTempBuffLength <= 0 )	break;	// ���ƤΥǡ����Ͻ�����ǽ(���ڤ��ɤ��������줿)

		headch = (BYTE)*pt;
		if ((char)headch < 0) {
			// STATUS,CTRL�ϥ�����
			dwRxTempBuffLength --;			// CTRL�ϥ����ɤ�1byte����
			pt++;					// ����header����򻲾�
			
			wDataLineCnt+=3;
		}else{
			if (headch == 0) {
				dwRxTempBuffLength -= 1;
				pt += 1;
				wDataLineCnt++;
			}
			else {
				// �����ǡ���
				WORD length;

				if( dwRxTempBuffLength < 3 )
					length = 0;		// �����
				else
					// �饹���ǡ���Ĺ�μ���
					length = *(WORD *)( pt + 1 );	// format: [HEADER(1B)][LENGTH(intel 2B)][DATA...]
				
				if( dwRxTempBuffLength < (DWORD)( length + 3) ){	// length+3 = head(1B)+length(2B)+data(length)
					break;
				}
				else{
					// 1lineʬ�Υǡ�������
					dwRxTempBuffLength -= length + 3;	// �����ǡ����� length+3 byte����
					pt += length + 3;			// ����header����򻲾�
					wDataLineCnt++;
				}
			}
		}
	} // end of for(;;)
	wData -= dwRxTempBuffLength;	// Ÿ�������˲󤹥ǡ������飱�饤��̤���Υǡ��������

	// �饹���ǡ�����Ÿ������
#ifdef NO39_DEBUG
	if (gettimeofday(&start_tv, &tz) == -1)
		return FALSE;
#endif

	nAnswer = ProcessMain( this, wData, wDataLineCnt, lpFwTempBuff+FwTempBuffLength, &FwTempBuffLength, &wProcessSize );

#ifdef NO39_DEBUG
	if (gettimeofday(&tv, &tz) == 0) {
		if (tv.tv_usec < start_tv.tv_usec) {
			tv.tv_usec += 1000 * 1000 ;
			tv.tv_sec-- ;
		}
		nSec = tv.tv_sec - start_tv.tv_sec;
		nUsec = tv.tv_usec - start_tv.tv_usec;

		WriteLog( " PageScan ProcessMain Time %d sec %d Us", nSec, nUsec );
		
	}

#endif

	
	if ((dwRxTempBuffLength > 0) || (wProcessSize < wData)) {
		dwRxTempBuffLength += (wData - wProcessSize); 
		memmove( lpRxTempBuff, lpRxBuff+wProcessSize, dwRxTempBuffLength );	// �Ĥ�ǡ�������¸
	}

	if ( nAnswer == SCAN_EOF || nAnswer == SCAN_MPS )  {
		// �Ǹ�Υڡ����ǡ����ξ��
		if( lRealY > 0 ){

			ImgLineProcInfo.pWriteBuff = lpFwTempBuff+FwTempBuffLength;
			ImgLineProcInfo.dwWriteBuffSize = dwImageBuffSize;

			dwWriteImageSize = this->scanDec.lpfnScanDecPageEnd( &ImgLineProcInfo, &nWriteLineCount );
			if( nWriteLineCount > 0 ){
				FwTempBuffLength += dwWriteImageSize;
				lRealY += nWriteLineCount;
			}

#if 1	// DEBUG for MASU
			dwFWImageSize += dwWriteImageSize;
			dwFWImageLine += nWriteLineCount;
			WriteLog( "DEBUG for MASU (PageScan) dwFWImageSize  = %d dwFWImageLine = %d", dwFWImageSize, dwFWImageLine );
			WriteLog( "  PageScan End1 nWriteLineCount = %d", nWriteLineCount );
#endif
		}
		// ���ơ����������ɤ�������¸�Хåե������ä����ᡢ���֤�Ф��Ƥ���
		this->scanState.iProcessEnd = nAnswer;
		WriteLog( " PageScan scanState.iProcessEnd = %d, ", this->scanState.iProcessEnd );
	}

	} 
	else { // wData == 0
		if (FwTempBuffLength == 0 && dwRxTempBuffLength == 0) {
			nAnswer = SCAN_EOF;
			WriteLog( "<<<<< PageScan [Read Error End]  <<<<<" );
		}
	}
	WriteLog( "ProcessMain End dwRxTempBuffLength = %d", dwRxTempBuffLength );

	} // if ( (!this->scanState.iProcessEnd) || ( FwTempBuffLength > nMaxLen) ) 

	if (this->scanState.iProcessEnd) { // ������¸�Хåե��˥��ơ����������ɤ�������Ƥ�����
		WriteLog( "<<<<< PageScan Status Code Read!!!" );
		nAnswer = this->scanState.iProcessEnd;
	}

	/* �����Хåե���������¸�Хåե��ˤ��륤�᡼���ǡ����򥳥ԡ����롣*/
	WriteLog( "<<<<< PageScan FwTempBuffLength = %d", FwTempBuffLength );

	if ( FwTempBuffLength > nMaxLen )
		*lpFwLen = nMaxLen;
	else
		*lpFwLen = FwTempBuffLength;

	FwTempBuffLength -= *lpFwLen ;

	memmove( lpFwBuf, lpFwTempBuff, *lpFwLen);	// ������¸�Хåե����������Хåե��إ��ԡ����롣
	memmove( lpFwTempBuff, lpFwTempBuff+*lpFwLen, FwTempBuffLength ); // �Ĥ����¸�ǡ�������Ƭ�˰�ư���롣	

	rc = SANE_STATUS_GOOD;
	
#ifdef NO39_DEBUG
	gettimeofday(&save_tv, &save_tz);
#endif
	if ( nAnswer == SCAN_EOF || nAnswer == SCAN_MPS )  {

		if (FwTempBuffLength != 0 ) { 
			return rc;
		}
		else {	
			// ���ꤷ���ǡ���Ĺ�����������ǡ���Ĺ�����ʤ���硢�Ĥ�Υǡ���Ĺ�����Ȥ��ƥ��åȤ��롣		
			if( lRealY < this->scanInfo.ScanAreaSize.lHeight ){
				// ���ꤷ��Ĺ����꾯�ʤ��ͤξ��֤ǡ��ڡ�������ɥ��ơ������Ȥʤä����
				int nHeightLen = this->scanInfo.ScanAreaSize.lHeight - lRealY;
				int nSize = this->scanInfo.ScanAreaByte.lWidth * nHeightLen; 
				int nMaxSize = nMaxLen - *lpFwLen;
				int nMaxLine;
				int nVal;
	
				if (this->devScanInfo.wColorType < COLOR_TG)
					nVal = 0x00;
				else
					nVal = 0xFF;
					
				if ( nSize < nMaxSize ) {
					memset(lpFwBuf+*lpFwLen, nVal, nSize);
					*lpFwLen += nSize;
					lRealY += nHeightLen;
					WriteLog( "PageScan AddSpace End lRealY = %d, nHeightLen = %d nSize = %d nMaxSize = %d *lpFwLen = %d",
					lRealY, nHeightLen, nSize, nMaxSize, *lpFwLen );
				}
				else {
					memset(lpFwBuf+*lpFwLen, nVal, nMaxSize);
					nMaxLine = nMaxSize / this->scanInfo.ScanAreaByte.lWidth;
					*lpFwLen += this->scanInfo.ScanAreaByte.lWidth * nMaxLine;
					lRealY += nMaxLine;
					WriteLog( "PageScan AddSpace lRealY = %d, nHeightLen = %d nSize = %d nMaxSize = %d *lpFwLen = %d",
					lRealY, nHeightLen, nSize, nMaxSize, *lpFwLen );
				}
			}
		}
	}

	switch( nAnswer ){
		case SCAN_CANCEL:
			WriteLog( "Page Canceled" );

			this->scanState.nPageCnt = 0;
			rc = SANE_STATUS_CANCELLED;
			this->scanState.bScanning=FALSE;
			this->scanState.bCanceled=FALSE;
			break; 

		case SCAN_EOF:
			WriteLog( "Page End" );
			WriteLog( "  nAnswer = %d lRealY = %d", nAnswer, lRealY );

			if( lRealY != 0 ) {
				// ������¸�Хåե��˥ǡ���������֤ϡ�SANE_STATUS_GOOD���֤���
				if (*lpFwLen == 0) {
					this->scanState.bEOF=TRUE;
					this->scanState.bScanning=FALSE;
					rc = SANE_STATUS_EOF;
				}
			}
			else {
				// �ǡ�������̵����EOF�ξ�硢���顼�Ȥ��롣
				rc = SANE_STATUS_IO_ERROR;
			}
			break; 
		case SCAN_MPS:
			// ������¸�Хåե��˥ǡ���������֤ϡ�SANE_STATUS_GOOD���֤���
			if (*lpFwLen == 0) {
				this->scanState.bEOF=TRUE;
				rc = SANE_STATUS_EOF;
			}
			break; 
		case SCAN_NODOC:
			rc = SANE_STATUS_NO_DOCS;
			break; 
		case SCAN_DOCJAM:
			rc = SANE_STATUS_JAMMED;
			break; 
		case SCAN_COVER_OPEN:
			rc = SANE_STATUS_COVER_OPEN;
			break; 
		case SCAN_SERVICE_ERR:
			rc = SANE_STATUS_IO_ERROR;
			break; 
	}

	nFwLenTotal += *lpFwLen;
	WriteLog( "<<<<< PageScan End <<<<< nFwLenTotal = %d lpFwLen = %d ",nFwLenTotal, *lpFwLen);

	return rc;
}
void
ReadTrash( Brother_Scanner *this )
{
	// �ǡ������ɤ߼Τ�
	// 1�ô֥���ǡ�����³�����顢�ǡ���̵����Ƚ�Ǥ��롣

	BYTE *lpBrBuff;
	int nResultSize;

	struct timeval start_tv, tv;
	struct timezone tz;
	long   nSec, nUsec;
	long   nTimeOutSec, nTimeOutUsec;

	if (gettimeofday(&start_tv, &tz) == -1)
		return ;

	lpBrBuff = (LPBYTE)MALLOC( 32000 );
	if (!lpBrBuff)
		return;

	if (gettimeofday(&start_tv, &tz) == -1) {
		FREE(lpBrBuff);
		return ;
	}
	// �����ॢ�����ͤ���ñ�̤�׻����롣
	nTimeOutSec = 1;
	// �����ॢ�����ͤΥޥ�������ñ�̤�׻����롣
	nTimeOutUsec = 0;

	nResultSize = 1;
	while (1) {
		if (gettimeofday(&tv, &tz) == 0) {
			if (tv.tv_usec < start_tv.tv_usec) {
				tv.tv_usec += 1000 * 1000 ;
				tv.tv_sec-- ;
			}
			nSec = tv.tv_sec - start_tv.tv_sec;
			nUsec = tv.tv_usec - start_tv.tv_usec;

			WriteLog( "AbortPageScan Read nSec = %d Usec = %d\n", nSec, nUsec ) ;

			if (nSec > nTimeOutSec) { // �����ॢ�����ͤ��ÿ�����礭������ȴ���롣
				break;
			} 
			else if( nSec == nTimeOutSec) { // �����ॢ�����ͤ��ÿ���Ʊ�����
				if (nUsec >= nTimeOutUsec) { // �����ॢ�����ͤΥޥ������ÿ����礭������ȴ���롣
					break;
				}
			}
		}
		else {
			break;
		}
		
		usleep(30 * 1000); // 30ms �Ԥ�

		// �ǡ������ɤ߼Τ�
		nResultSize = usb_bulk_read(this->hScanner,
	        0x84,
	        lpBrBuff, 
	        32000,
	        2000
		);
		WriteLog( "AbortPageScan Read end nResultSize = %d", nResultSize) ;

		// �ǡ���������֤ϡ������ޤ�ꥻ�åȤ��롣
		if (nResultSize > 0) { // �ǡ�����������

			if (gettimeofday(&start_tv, &tz) == -1)
				break;
		}
	}
	FREE(lpBrBuff);

}
//-----------------------------------------------------------------------------
//
//	Function name:	AbortPageScan
//
//
//	Abstract:
//		�ڡ�������������߽���
//
//
//	Parameters:
//
//	Return values:
//
//-----------------------------------------------------------------------------
//	AbortPageScan�ʵ�PageScan�ΰ�����
void
AbortPageScan( Brother_Scanner *this )
{
	WriteLog( ">>>>> AbortPageScan Start >>>>>" );

	//
	// Cancel���ޥ�ɤ�����
	//
	SendCancelCommand(this->hScanner);
	this->scanState.bCanceled=TRUE;
	ReadTrash( this );

	WriteLog( "<<<<< AbortPageScan End <<<<<" );

	return;
}

/********************************************************************************
 *																				*
 *	FUNCTION	ScanEnd															*
 *																				*
 *	PURPOSE		������󥻥å����λ����										*
 *				�������������׵��Ԥ�										*
 *																				*
 *	IN		HWND	hdlg	���������ܥå����ϥ�ɥ�							*
 *																				*
 *	OUT		̵��																*
 *																				*
 ********************************************************************************/
void
ScanEnd( Brother_Scanner *this )
{
	BOOL  bResult;

	this->scanState.nPageCnt = 0;
	bTxScanCmd = FALSE;		// Scan���ϥ��ޥ�������ե饰�Υ��ꥢ

#ifndef DEBUG_No39
	if ( this->hScanner != NULL ) {
		CloseDevice(this->hScanner);
		usb_release_interface(this->hScanner, 1);
		usb_close(this->hScanner);
		this->hScanner=NULL;
	}
#endif
	if( hGray && ( hGray != this->cmatch.hGrayTbl ) ){
		FREE( hGray );
		hGray = NULL;
		WriteLog( "free hGray" );
	}

	FreeReceiveBuffer();
	if (lpRxTempBuff) {
		FREE(lpRxTempBuff);
		lpRxTempBuff = NULL;
	}
	if (lpFwTempBuff) {
		FREE(lpFwTempBuff);
		lpFwTempBuff = NULL;
	}
	//
	// Decode/Stretch������λ
	//
	bResult = this->scanDec.lpfnScanDecClose();

	WriteLog( "<<<<< Terminate Scanning <<<<<" );

}


//-----------------------------------------------------------------------------
//
//	Function name:	GetScanAreaParam
//
//
//	Abstract:
//		�ɤ߼���ϰϡ�0.1mmñ�̡ˤ���
//		�������ѥ�᡼���Ѥ��ɤ߼���ϰϾ�������
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
//	GetScanAreaParam�ʵ�GetScanDot��
void 
GetScanAreaParam( Brother_Scanner *this )
{
	LPAREARECT  lpScanAreaDot;
	LONG    lUserResoX,   lUserResoY;
	LONG    lDeviceResoX, lDeviceResoY;


	//
	// UI���꤫��scanInfo��¤�Τ˾���򥳥ԡ�
	//
	this->scanInfo.ScanAreaMm = this->uiSetting.ScanAreaMm;

	//
	// �ɤ߼���ϰϺ�ɸ��0.1mmñ�̤���dotñ�̤ؤ��Ѵ�
	//
	lUserResoX   = this->scanInfo.UserSelect.wResoX;
	lUserResoY   = this->scanInfo.UserSelect.wResoY;
	lDeviceResoX = this->devScanInfo.DeviceScan.wResoX;
	lDeviceResoY = this->devScanInfo.DeviceScan.wResoY;

	// �桼��������ɤ߼���ϰϺ�ɸ��dotñ�̡�
	lpScanAreaDot = &this->scanInfo.ScanAreaDot;
	lpScanAreaDot->left   = this->scanInfo.ScanAreaMm.left   * lUserResoX / 254L;
	lpScanAreaDot->right  = this->scanInfo.ScanAreaMm.right  * lUserResoX / 254L;
	lpScanAreaDot->top    = this->scanInfo.ScanAreaMm.top    * lUserResoY / 254L;
	lpScanAreaDot->bottom = this->scanInfo.ScanAreaMm.bottom * lUserResoY / 254L;

	// �������ѥ�᡼���Ѥ��ɤ߼���ϰϺ�ɸ��dotñ�̡�
	lpScanAreaDot = &this->devScanInfo.ScanAreaDot;
	lpScanAreaDot->left   = this->scanInfo.ScanAreaMm.left   * lDeviceResoX / 254L;
	lpScanAreaDot->right  = this->scanInfo.ScanAreaMm.right  * lDeviceResoX / 254L;
	lpScanAreaDot->top    = this->scanInfo.ScanAreaMm.top    * lDeviceResoY / 254L;
	lpScanAreaDot->bottom = this->scanInfo.ScanAreaMm.bottom * lDeviceResoY / 254L;

	//
	// �������ѥ�᡼���Ѥ��ɤ߼���ϰϺ�ɸ��dotñ�̡ˤ����
	//
	GetDeviceScanArea( this, lpScanAreaDot );

	//
	// �ɤ߼���ϰϤΥ�������dotñ�̡ˤ����
	//
	this->devScanInfo.ScanAreaSize.lWidth  = lpScanAreaDot->right  - lpScanAreaDot->left;
	this->devScanInfo.ScanAreaSize.lHeight = lpScanAreaDot->bottom - lpScanAreaDot->top;

	//
	// �ɤ߼���ϰϤΥ�������byteñ�̡ˤ����
	//
	if( this->devScanInfo.wColorType == COLOR_BW || this->devScanInfo.wColorType == COLOR_ED ){
		this->devScanInfo.ScanAreaByte.lWidth = ( this->devScanInfo.ScanAreaSize.lWidth + 7 ) / 8;
	}else{
		this->devScanInfo.ScanAreaByte.lWidth = this->devScanInfo.ScanAreaSize.lWidth;
	}
	this->devScanInfo.ScanAreaByte.lHeight = this->devScanInfo.ScanAreaSize.lHeight;

	WriteLog(" brother_scanner.c GetScanAreaParam LOG START !!");

	WriteLog( "scanInfo.ScanAreaMm.left = [%d]", this->scanInfo.ScanAreaMm.left );
	WriteLog( "scanInfo.ScanAreaMm.right = [%d]", this->scanInfo.ScanAreaMm.right );
	WriteLog( "scanInfo.ScanAreaMm.top = [%d]", this->scanInfo.ScanAreaMm.top );
	WriteLog( "scanInfo.ScanAreaMm.bottom = [%d]", this->scanInfo.ScanAreaMm.bottom );

	WriteLog( "lUserResoX = [%d]", lUserResoX );
	WriteLog( "lUserResoY = [%d]", lUserResoY );

	WriteLog( "scanInfo.ScanAreaSize.lWidth = [%d]", this->scanInfo.ScanAreaSize.lWidth );
	WriteLog( "scanInfo.ScanAreaSize.lHeight = [%d]", this->scanInfo.ScanAreaSize.lHeight );
	WriteLog( "scanInfo.ScanAreaByte.lWidth = [%d]", this->scanInfo.ScanAreaByte.lWidth );
	WriteLog( "scanInfo.ScanAreaByte.lHeight = [%d]", this->scanInfo.ScanAreaByte.lHeight );

	WriteLog( "lDeviceResoX = [%d]", lDeviceResoX );
	WriteLog( "lDeviceResoY = [%d]", lDeviceResoY );

	WriteLog( "devScanInfo.ScanAreaSize.lWidth = [%d]", this->devScanInfo.ScanAreaSize.lWidth );
	WriteLog( "devScanInfo.ScanAreaSize.lHeight = [%d]", this->devScanInfo.ScanAreaSize.lHeight );
	WriteLog( "devScanInfo.ScanAreaByte.lWidth = [%d]", this->devScanInfo.ScanAreaByte.lWidth );
	WriteLog( "devScanInfo.ScanAreaByte.lHeight = [%d]", this->devScanInfo.ScanAreaByte.lHeight );

	WriteLog(" brother_scanner.c GetScanAreaParam LOG END !!");
}


//-----------------------------------------------------------------------------
//
//	Function name:	StartDecodeStretchProc
//
//
//	Abstract:
//		�������ǡ���Ÿ�����������Ѵ��⥸�塼�����������
//
//
//	Parameters:
//		�ʤ�
//
//
//	Return values:
//		TRUE  = ���ｪλ
//		FALSE = ���������
//
//-----------------------------------------------------------------------------
//
BOOL
StartDecodeStretchProc( Brother_Scanner *this )
{
	BOOL  bResult = TRUE;

	//
	// �饹���ǡ������������
	//
	ImageProcInfo.nInResoX  = this->devScanInfo.DeviceScan.wResoX;
	ImageProcInfo.nInResoY  = this->devScanInfo.DeviceScan.wResoY;
	ImageProcInfo.nOutResoX = this->scanInfo.UserSelect.wResoX;
	ImageProcInfo.nOutResoY = this->scanInfo.UserSelect.wResoY;
	ImageProcInfo.dwInLinePixCnt = this->devScanInfo.ScanAreaSize.lWidth;

	ImageProcInfo.nOutDataKind = SCODK_PIXEL_RGB;

#if 1 // Black&White���������Ǥ��ʤ�t�Զ����б�
	ImageProcInfo.bLongBoundary = FALSE;	// 4�Х��ȶ����򤷤ʤ�
#else
	ImageProcInfo.bLongBoundary = TRUE;
#endif
	//
	// ���顼�����פ�����
	//
	switch( this->devScanInfo.wColorType ){
		case COLOR_BW:			// Black & White
			ImageProcInfo.nColorType = SCCLR_TYPE_BW;
			break;

		case COLOR_ED:			// Error Diffusion Gray
			ImageProcInfo.nColorType = SCCLR_TYPE_ED;
			break;

		case COLOR_DTH:			// Dithered Gray
			ImageProcInfo.nColorType = SCCLR_TYPE_DTH;
			break;

		case COLOR_TG:			// True Gray
			ImageProcInfo.nColorType = SCCLR_TYPE_TG;
			break;

		case COLOR_256:			// 256 Color
			ImageProcInfo.nColorType = SCCLR_TYPE_256;
			break;

		case COLOR_FUL:			// 24bit Full Color
			ImageProcInfo.nColorType = SCCLR_TYPE_FUL;
			break;

		case COLOR_FUL_NOCM:	// 24bit Full Color(do not colormatch)
			ImageProcInfo.nColorType = SCCLR_TYPE_FULNOCM;
			break;
	}

	//
	// �������ǡ���Ÿ�����������Ѵ��⥸�塼������
	//
	if (this->scanDec.lpfnScanDecOpen) {
		bResult = this->scanDec.lpfnScanDecOpen( &ImageProcInfo );
		WriteLog( "Result from ScanDecOpen is %d", bResult );
	}
	
	dwImageBuffSize = ImageProcInfo.dwOutWriteMaxSize;
	WriteLog( "ScanDec Func needs maximum size is %d", dwImageBuffSize );

	//
	// �������ǡ���Ÿ�����������Ѵ���Υǡ������򥻥å�
	//
	this->scanInfo.ScanAreaSize.lWidth = ImageProcInfo.dwOutLinePixCnt;
	this->scanInfo.ScanAreaByte.lWidth = ImageProcInfo.dwOutLineByte;

	this->scanInfo.ScanAreaSize.lHeight = this->devScanInfo.ScanAreaSize.lHeight;

	if( this->devScanInfo.DeviceScan.wResoY != this->scanInfo.UserSelect.wResoY ){
		//
		// ���硿�̾������ɤ߼��Ĺ����������
		//
		this->scanInfo.ScanAreaSize.lHeight *= this->scanInfo.UserSelect.wResoY;
		this->scanInfo.ScanAreaSize.lHeight /= this->devScanInfo.DeviceScan.wResoY;
	}
	this->scanInfo.ScanAreaByte.lHeight = this->scanInfo.ScanAreaSize.lHeight;

	return bResult;
}


//-----------------------------------------------------------------------------
//
//	Function name:	GetDeviceScanArea
//
//
//	Abstract:
//		�������ѥ�᡼���Ѥ��ɤ߼���ϰϺ�ɸ��dotñ�̡ˤ����
//
//
//	Parameters:
//		lpScanAreaDot
//			��������ϰϤκ�ɸ��dotñ�̡ˤؤΥݥ���
//
//
//	Return values:
//		�ʤ�
//
//-----------------------------------------------------------------------------
//	GetDeviceScanArea�ʵ�GetScanDot�ΰ�����
void
GetDeviceScanArea( Brother_Scanner *this, LPAREARECT lpScanAreaDot )
{
	LONG    lMaxScanPixels;
	LONG    lMaxScanRaster;
	LONG    lTempWidth;


	lMaxScanPixels = this->devScanInfo.dwMaxScanPixels;

	if( this->devScanInfo.wScanSource == MFCSCANSRC_FB ){
		//
		// ������󥽡�����FB�ξ��
		// �ǥХ������������������饹����������
		//
		lMaxScanRaster = this->devScanInfo.dwMaxScanRaster;
	}else{
		//
		// ������󥽡�����ADF�ξ�硢14inch������
		//
		lMaxScanRaster = 14 * this->devScanInfo.DeviceScan.wResoY;
	}
	//
	// �ɤ߼���ϰϱ�¦������
	//
	if( lpScanAreaDot->right > lMaxScanPixels ){
		lpScanAreaDot->right = lMaxScanPixels;
	}
	lTempWidth = lpScanAreaDot->right - lpScanAreaDot->left;
	if( lTempWidth < 16 ){
	    //
	    // �ɤ߼������16dot̤���ξ��
	    //
	    lpScanAreaDot->right = lpScanAreaDot->left + 16;
	    if( lpScanAreaDot->right > lMaxScanPixels ){
			//
			// �ɤ߼���ϰϱ�¦���ɤ߼��³���Ķ�������
			// ��ü����ˤ����ɤ߼���ϰϺ�¦�����
			//
			lpScanAreaDot->right = lMaxScanPixels;
			lpScanAreaDot->left  = lMaxScanPixels - 16;
		}
	}
	//
	// �ɤ߼���ϰϲ�¦������
	//
	if( lpScanAreaDot->bottom > lMaxScanRaster ){
		lpScanAreaDot->bottom = lMaxScanRaster;
	}

	//
	// �̾凉�����ξ�硢16dotñ�̤˴ݤ��
	//   0-7 -> 0, 8-15 -> 16
	//
	lpScanAreaDot->left  = ( lpScanAreaDot->left  + 0x8 ) & 0xFFF0;
	lpScanAreaDot->right = ( lpScanAreaDot->right + 0x8 ) & 0xFFF0;

}


/********************************************************************************
 *										*
 *	FUNCTION	ProcessMain						*
 *										*
 *	IN		Brother_Scanner Brother_Scanner��¤�ΤΥϥ�ɥ�		*
 *			WORD		�ǡ�����					*
 *			char *		���ϥХåե� 				*
 *			int *		���ϥǡ������Υݥ���			*
 *										*
 *	OUT		̵��							*
 *										* 
 *	COMMENT		������ʤ����ɹ�����ǡ������Ф��ƽ�����Ԥ���			*
 *			�Хåե��ˤϥ饤��ñ�̤ǥǡ��������äƤ����ΤȤ��롣		*
 *										*
 ********************************************************************************/
int
ProcessMain(Brother_Scanner *this, WORD wByte, WORD wDataLineCnt, char * lpFwBuf, int *lpFwBufcnt, WORD *lpProcessSize)
{
	LPBYTE	lpScn = lpRxBuff;
	LPBYTE	lpScnEnd;
	int	answer = SCAN_GOOD;
	char	Header;
	DWORD	Dcount;
	LONG	count;
	LPBYTE	lpSrc;
	LPBYTE	lpHosei;
	WORD	wLineCnt;
#ifdef NO39_DEBUG
	struct timeval start_tv, tv;
	struct timezone tz;
	long   nSec, nUsec;
#endif

	WriteLog( ">>>>> ProcessMain Start >>>>> wDataLineCnt=%d", wDataLineCnt);

	lpScn = lpRxBuff;

	lpScnEnd = lpScn + wByte;
	WriteLog( "lpFwBuf = %X, lpScn = %X, wByte = %d, lpScnEnd = %X", lpFwBuf, lpScn, wByte, lpScnEnd );

	if (this->devScanInfo.wColorType == COLOR_FUL || this->devScanInfo.wColorType == COLOR_FUL_NOCM) {
		if (wDataLineCnt)
		wDataLineCnt = (wDataLineCnt / 3 * 3);
	}
	for( wLineCnt=0; wLineCnt < wDataLineCnt; wLineCnt++ ){
		//
		//	Header <- �饤��إå����Х���
		//
		Header = *lpScn++;

		if( Header < 0 ){
			//
			//	Status code
			//
			WriteLog( "Header=%2x  ", (BYTE)Header );

			answer = GetStatusCode( Header );
			break;	//break for
		}else{
			//
			//	Scanned Data
			//
			if( Header == 0 ){
				//
				//	White line
				//
				WriteLog( "Header=%2x  while line", (BYTE)Header );
				WriteLog( "\tlpFwBufp = %X, lpScn = %X", lpFwBuf, lpScn );

				if( lpFwBuf ){
					lRealY++;
					lpFwBuf += this->scanInfo.ScanAreaByte.lWidth;
				}
			}else{
				//
				//	Scanner data
				//
				Dcount = (DWORD)*( (WORD *)lpScn );	// �ǡ���Ĺ
				count  = (WORD)Dcount;		// �ǡ���Ĺ
				lpScn += 2;
				lpSrc = lpScn;
				lpHosei = 0;	// Hosei�ä�....

				WriteLog( "Header=%2x  Count=%4d", (BYTE)Header, count );
				WriteLog( "\tlpFwBuf = %X, lpScn = %X", lpFwBuf, lpScn );

				if( lpFwBuf ){
					//
					// �饹���ǡ���Ÿ�����������Ѵ��⥸�塼�����ѿ�������
					//
					SetupImgLineProc( Header );
					ImgLineProcInfo.pLineData      = lpSrc;
					ImgLineProcInfo.dwLineDataSize = count;
					ImgLineProcInfo.pWriteBuff     = lpFwBuf;
					//
					// �饹���ǡ���Ÿ�����������Ѵ�
					//
#ifdef NO39_DEBUG
	if (gettimeofday(&start_tv, &tz) == -1)
		return FALSE;
#endif

					dwWriteImageSize = this->scanDec.lpfnScanDecWrite( &ImgLineProcInfo, &nWriteLineCount );
					WriteLog( "\tlpFwBuf = %X, WriteSize = %d, LineCount = %d, RealY = %d", lpFwBuf, dwWriteImageSize, nWriteLineCount, lRealY );

#ifdef NO39_DEBUG
	if (gettimeofday(&tv, &tz) == 0) {
		if (tv.tv_usec < start_tv.tv_usec) {
			tv.tv_usec += 1000 * 1000 ;
			tv.tv_sec-- ;
		}
		nSec = tv.tv_sec - start_tv.tv_sec;
		nUsec = tv.tv_usec - start_tv.tv_usec;

		WriteLog( " ProcessMain ScanDecWrite Time %d sec %d Us", nSec, nUsec );
		
	}
#endif

					if( nWriteLineCount > 0 ){
						*lpFwBufcnt += dwWriteImageSize;
#if 1	// DEBUG for MASU
						dwFWImageSize += dwWriteImageSize;
						dwFWImageLine += nWriteLineCount;
						WriteLog( "DEBUG for MASU (ProcessMain) dwFWImageSize  = %d dwFWImageLine = %d", dwFWImageSize, dwFWImageLine );
#endif

						if( this->mfcModelInfo.bColorModel && ! this->modelConfig.bNoUseColorMatch && this->devScanInfo.wColorType == COLOR_FUL ){
							int  i;
							for( i = 0; i < nWriteLineCount; i++ ){
								ExecColorMatchingFunc( this, lpFwBuf, this->scanInfo.ScanAreaByte.lWidth, 1 );
								lpFwBuf += dwWriteImageSize / nWriteLineCount;
							}
						}else{
							lpFwBuf += dwWriteImageSize;
						}
						lRealY += nWriteLineCount;
					}
				}
				lpScn += Dcount;
			}

		}//end of if *lpScn<0
	}//end of for
	*lpProcessSize = (WORD)(lpScn - lpRxBuff);

	WriteLog( "<<<<< ProcessMain End <<<<<" );

	return answer;
}


//-----------------------------------------------------------------------------
//
//	Function name:	SetupImgLineProc
//
//
//	Abstract:
//		�饹���ǡ���Ÿ�����������Ѵ��⥸�塼�����ѿ������ꤹ��
//
//
//	Parameters:
//		chLineHeader
//			�ɤ߹�����饹���ǡ����Υ饤��إå�
//
//
//	Return values:
//		�ʤ�
//
//-----------------------------------------------------------------------------
//
void
SetupImgLineProc( BYTE chLineHeader )
{
	BYTE  chColorMode;
	BYTE  chCompression;


	//
	// �饤��ǡ����Υ��顼�����פ�����
	//
	chColorMode = chLineHeader & 0x1C;
	switch( chColorMode ){
		case 0x00:	// ���ͥǡ���
			ImgLineProcInfo.nInDataKind = SCIDK_MONO;
			break;

		case 0x04:	// Red�ǡ���
			ImgLineProcInfo.nInDataKind = SCIDK_R;
			break;

		case 0x08:	// Green�ǡ���
			ImgLineProcInfo.nInDataKind = SCIDK_G;
			break;

		case 0x0C:	// Blue�ǡ���
			ImgLineProcInfo.nInDataKind = SCIDK_B;
			break;

		case 0x10:	// ���ǽ缡(RGB)�ǡ���
			ImgLineProcInfo.nInDataKind = SCIDK_RGB;
			break;

		case 0x14:	// ���ǽ缡(BGR)�ǡ���
			ImgLineProcInfo.nInDataKind = SCIDK_BGR;
			break;

		case 0x1C:	// 256�����顼�ǡ���
			ImgLineProcInfo.nInDataKind = SCIDK_256;
			break;
	}

	//
	// �饤��ǡ����ΰ��̥⡼�ɤ�����
	//
	chCompression = chLineHeader & 0x03;
	if( chCompression == 2 ){
		// Packbits compression
		ImgLineProcInfo.nInDataComp = SCIDC_PACK;
	}else{
		ImgLineProcInfo.nInDataComp = SCIDC_NONCOMP;
	}

	//
	// ����¾�Υѥ�᡼��������
	//
#if 1
	ImgLineProcInfo.bReverWrite     = FALSE;
#else
	ImgLineProcInfo.bReverWrite     = TRUE;
#endif
	ImgLineProcInfo.dwWriteBuffSize = dwImageBuffSize;
}


//-----------------------------------------------------------------------------
//
//	Function name:	GetStatusCode
//
//
//	Abstract:
//		�饤�󥹥ơ����������顼�����ɽ���
//
//
//	Parameters:
//		nLineHeader
//			�饤��إå���
//
//
//
//	Return values:
//		���������Υ��ơ���������
//
//-----------------------------------------------------------------------------
//
int
GetStatusCode( BYTE nLineHeader )
{
	int   answer;

	switch( nLineHeader ){
		case 0x80:	//terminate
			WriteLog( "\tPage end Detect" );
			answer = SCAN_EOF;
			break;

		case 0x81:	//Page end
			WriteLog( "\tNextPage Detect" );
			answer = SCAN_MPS;
			break;

		case 0xE3:	//MF14	MFC cancel scan because of timeout
		case 0x83:	//cancel acknowledge
			WriteLog( "\tCancel acknowledge" );
			answer = SCAN_CANCEL;
			break;

		case 0xC2:	//no document
			WriteLog( "\tNo document" );
			answer = SCAN_NODOC;	// if no document, don't send picture
			break;

		case 0xC3:	//document jam
			WriteLog( "\tDocument JAM" );
			answer = SCAN_DOCJAM;
			break;

		case 0xC4:	//Cover Open
			WriteLog( "\tCover open" );
			answer = SCAN_COVER_OPEN;
			break;

		case 0xE5:	//
		case 0xE6:	//
		case 0xE7:	//
		default:	//service error
			WriteLog( "\tService Error\n" );
			answer = SCAN_SERVICE_ERR;
			break;
	}

	return answer;
}


//-----------------------------------------------------------------------------
//
//	Function name:	CnvResoNoToUserResoValue
//
//
//	Abstract:
//		�����٥����פ�������١ʿ��͡ˤ����
//
//
//	Parameters:
//		nResoNo
//			�����٥������ֹ�
//		pScanInfo
//			���������ξ���
//
//	Return values:
//		�ʤ�
//
//-----------------------------------------------------------------------------
//
void
CnvResoNoToUserResoValue( LPRESOLUTION pUserSelect, WORD nResoNo )
{
	switch( nResoNo ){
		case RES100X100:	// 100 x 100 dpi
			pUserSelect->wResoX = 100;
			pUserSelect->wResoY = 100;
			break;

		case RES150X150:	// 150 x 150 dpi
			pUserSelect->wResoX = 150;
			pUserSelect->wResoY = 150;
			break;

		case RES200X100:	// 200 x 100 dpi
			pUserSelect->wResoX = 200;
			pUserSelect->wResoY = 100;
			break;

		case RES200X200:	// 200 x 200 dpi
			pUserSelect->wResoX = 200;
			pUserSelect->wResoY = 200;
			break;

		case RES200X400:	// 200 x 400 dpi
			pUserSelect->wResoX = 200;
			pUserSelect->wResoY = 400;
			break;

		case RES300X300:	// 300 x 300 dpi
			pUserSelect->wResoX = 300;
			pUserSelect->wResoY = 300;
			break;

		case RES400X400:	// 400 x 400 dpi
			pUserSelect->wResoX = 400;
			pUserSelect->wResoY = 400;
			break;

		case RES600X600:	// 600 x 600 dpi
			pUserSelect->wResoX = 600;
			pUserSelect->wResoY = 600;
			break;

		case RES800X800:	// 800 x 800 dpi
			pUserSelect->wResoX = 800;
			pUserSelect->wResoY = 800;
			break;

		case RES1200X1200:	// 1200 x 1200 dpi
			pUserSelect->wResoX = 1200;
			pUserSelect->wResoY = 1200;
			break;

		case RES2400X2400:	// 2400 x 2400 dpi
			pUserSelect->wResoX = 2400;
			pUserSelect->wResoY = 2400;
			break;

		case RES4800X4800:	// 4800 x 4800 dpi
			pUserSelect->wResoX = 4800;
			pUserSelect->wResoY = 4800;
			break;

		case RES9600X9600:	// 9600 x 9600 dpi
			pUserSelect->wResoX = 9600;
			pUserSelect->wResoY = 9600;
			break;
	}
}


//////// end of brother_scanner.c ////////
