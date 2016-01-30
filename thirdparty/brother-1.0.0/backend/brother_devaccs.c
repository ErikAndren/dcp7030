///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
//	Source filename: brother_devaccs.c
//
//		Copyright(c) 1997-2000 Brother Industries, Ltd.  All Rights Reserved.
//
//
//	Abstract:
//			Device�������������⥸�塼��
//
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <sys/time.h>
#include <signal.h>

#include <usb.h>

#include "brother_misc.h"
#include "brother_log.h"

#include "brother_devaccs.h"
#include "brother_mfccmd.h"
//
// �������Хåե�������
//
WORD  gwInBuffSize;

//
// Device�����������Υ����ॢ���Ȼ���
//
UINT  gnQueryTimeout;	// Query�ϥ��ޥ�ɤΥ쥹�ݥ󥹼������Υ����ॢ���Ȼ���
UINT  gnScanTimeout;	// ������󳫻ϡ����������Υ����ॢ���Ȼ���

//
// �����Хåե��Υϥ�ɥ�
//
static HANDLE  hReceiveBuffer  = NULL;

BOOL timeout_flg;

static int iReadStatus;			// ����ǡ����꡼�ɻ��Υ��ơ�����
static struct timeval save_tv;		// ���־�����¸�ѿ�(sec, msec)
static struct timezone save_tz;		// ���־�����¸�ѿ�(min)

//-----------------------------------------------------------------------------
//
//	Function name:	GetDeviceAccessParam
//
//
//	Abstract:
//		�ǥХ������������Τ����ɬ�פʥѥ�᡼�������ꤹ��
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
GetDeviceAccessParam( Brother_Scanner *this )
{
	//
	// �������Хåե��������μ���
	//
	gwInBuffSize  = this->modelConfig.wInBuffSize;

	//
	// Query�ϥ��ޥ�ɤΥ쥹�ݥ󥹼������Υ����ॢ���Ȼ��֤μ���
	//
	gnQueryTimeout = TIMEOUT_QUERYRES;

	//
	// ������󳫻ϡ����������Υ����ॢ���Ȼ��֤μ���
	//
	gnScanTimeout  = TIMEOUT_SCANNING * 1000;
}

//-----------------------------------------------------------------------------
//
//	Function name:	OpenDevice
//
//
//	Abstract:
//		�ǥХ����򥪡��ץ󤹤�
//
//
//	Parameters:
//		�ʤ�
//
//	Return values:
//		TRUE  ����
//		FALSE �����ץ���
//
//-----------------------------------------------------------------------------
//
int
OpenDevice(usb_dev_handle *hScanner)
{
	int rc, nValue;
	int i;
	char data[BREQ_GET_LENGTH];

	rc = 0;

	WriteLog( "<<< OpenDevice start <<<\n" );

	for (i = 0; i < RETRY_CNT;i++) {

		rc = usb_control_msg(hScanner,       /* handle */
                    BREQ_TYPE,           /* request type */
                    BREQ_GET_OPEN,  /* request */    /* GET_OPEN */
                    BCOMMAND_SCANNER,/* value */      /* scanner  */
                    0,              /* index */
                    data, BREQ_GET_LENGTH,        /* bytes, size */
                    2000            /* Timeout */
		);
		if (rc >= 0) {
				break;
		}
	}
	if (rc < 0)
		return FALSE;

	// �ǥ�������ץ��Υ��������������������å�
	nValue = (int) data[0];
	if (nValue != BREQ_GET_LENGTH)
		return FALSE;

	// �ǥ�������ץ������פ��������������å�
	nValue = (int)data[1];
	if (nValue != BDESC_TYPE)
		return FALSE;

	// ���ޥ��ID���������������å�
	nValue = (int)data[2];
	if (nValue != BREQ_GET_OPEN)
		return FALSE;

	// ���ޥ�ɥѥ�᡼�����������������å�
	nValue = (int)*((WORD *)&data[3]);
	if (nValue & BCOMMAND_RETURN)
		return FALSE;

	if (nValue != BCOMMAND_SCANNER)
		return FALSE;

	// �ꥫ�Х꡼����
	{
	BYTE *lpBrBuff;
	int nResultSize;
	int iFirstData;

	struct timeval start_tv, tv;
	struct timezone tz;
	long   nSec, nUsec;
	long   nTimeOutSec, nTimeOutUsec;

	if (gettimeofday(&start_tv, &tz) == -1)
		return FALSE;

	lpBrBuff = (LPBYTE)MALLOC( 32000 );
	if (!lpBrBuff)
		return FALSE;

	if (gettimeofday(&start_tv, &tz) == -1) {
		FREE(lpBrBuff);
		return FALSE;
	}

	// �����ॢ�����ͤ���ñ�̤�׻����롣
	nTimeOutSec = 1;
	// �����ॢ�����ͤΥޥ�������ñ�̤�׻����롣
	nTimeOutUsec = 0;

	iFirstData = 1;
	nResultSize = 1;
	while (1) {
		if (gettimeofday(&tv, &tz) == 0) {
			if (tv.tv_usec < start_tv.tv_usec) {
				tv.tv_usec += 1000 * 1000 ;
				tv.tv_sec-- ;
			}
			nSec = tv.tv_sec - start_tv.tv_sec;
			nUsec = tv.tv_usec - start_tv.tv_usec;

			WriteLog( "OpenDevice Recovery nSec = %d Usec = %d\n", nSec, nUsec ) ;

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
			FREE(lpBrBuff);
			return FALSE;
		}
		
		usleep(30 * 1000); // 30ms �Ԥ�
		WriteLog( "OpenDevice Recovery Read start" );

		// �ǡ������ɤ߼Τ�
		nResultSize = usb_bulk_read(hScanner,
	        0x84,
	        lpBrBuff, 
	        32000,
	        2000
		);
		WriteLog( "OpenDevice Recovery Read end nResultSize = %d", nResultSize) ;

		// �ǡ���������֤ϡ������ޤ�ꥻ�åȤ��롣
		if (nResultSize > 0) { // �ǡ�����������

			// ����ܤϡ�Q���ޥ�ɤ�ȯ��
			if (iFirstData){
				WriteLog( "OpenDevice Recovery Q Command" );
				// Q-���ޥ�ɤ�ȯ��
				WriteDeviceData( hScanner, MFCMD_QUERYDEVINFO, strlen( MFCMD_QUERYDEVINFO ) );
				iFirstData = 0;
			}
			if (gettimeofday(&start_tv, &tz) == -1) {
				FREE(lpBrBuff);
				return FALSE;
			}
		}
	}
	FREE(lpBrBuff);

	} // �ꥫ�Х꡼����
	
	return TRUE;
}


//-----------------------------------------------------------------------------
//
//	Function name:	CloseDevice
//
//
//	Abstract:
//		�ǥХ����򥯥�������
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
CloseDevice( usb_dev_handle *hScanner )
{
	int rc;
	int i;
	char data[BREQ_GET_LENGTH];

	for (i = 0; i < RETRY_CNT;i++) {
		rc = usb_control_msg(hScanner,       /* handle */
                    BREQ_TYPE,           /* request type */
                    BREQ_GET_CLOSE,  /* request */    /* GET_OPEN */
                    BCOMMAND_SCANNER,/* value */      /* scanner  */
                    0,              /* index */
                    data, BREQ_GET_LENGTH,        /* bytes, size */
                    2000            /* Timeout */
		);
		if (rc >= 0)
			break;
	}

	return;
}


//-----------------------------------------------------------------------------
//
//	Function name:	ReadDeviceData
//
//
//	Abstract:
//		�ǥХ�������ǡ�����꡼�ɤ���
//
//
//	Parameters:
//		lpRxBuffer
//			�꡼�ɥХåե��ؤΥݥ���
//
//		nReadSize
//			�꡼�ɡ�������
//
//
//	Return values:
//		0 >  ���ｪλ���ºݤ˥꡼�ɤ����Х��ȿ�
//		0 <= �꡼�ɼ��ԡ����顼������
//
//-----------------------------------------------------------------------------
//
int
ReadDeviceData( usb_dev_handle *hScanner, LPBYTE lpRxBuffer, int nReadSize )
{
	int  nResultSize = 0;
	int  nTimeOut = 2000;

	WriteLog( "ReadDeviceData Start nReadSize =%d\n", nReadSize ) ;

	if (iReadStatus > 0) { // ����Х��ȥ꡼�ɤΥ��ơ�������ͭ���ʾ��
		struct timeval tv;
		struct timezone tz;
		long   nSec, nUsec;


		if (gettimeofday(&tv, &tz) == 0) {
	  					
			if (tv.tv_usec < save_tv.tv_usec) {
				tv.tv_usec += 1000 * 1000 ;
				tv.tv_sec-- ;
			}
			nUsec = tv.tv_usec - save_tv.tv_usec;
			nSec = tv.tv_sec - save_tv.tv_sec;

			WriteLog( "ReadDeviceData iReadStatus = %d nSec = %d Usec = %d\n",iReadStatus, nSec, nUsec ) ;

			if (iReadStatus == 1) { // 1���ܤΥ���Х��ȥ꡼�ɤϡ�1s�Ԥ�
				if (nSec == 0) { // �¿��˰㤤���ʤ����ϡ�1�°ʲ��ΰ㤤�Ȥʤ�
					if (nUsec < 1000) // 1ms�ԤäƤʤ����
						usleep( 1000 - nUsec );
				}
			}
			else if (iReadStatus == 2) { // 2���ܰʹߤΥ���Х��ȥ꡼�ɤϡ�200s�Ԥ�
				if (nSec == 0) { // �¿��˰㤤���ʤ����ϡ�1�°ʲ��ΰ㤤�Ȥʤ�
					if (nUsec < 200 * 1000) // 200ms�ԤäƤʤ����
						usleep( 200 * 1000 - nUsec );
				}
				
			}
		}
	}

	nResultSize = usb_bulk_read(hScanner,
        0x84,
        lpRxBuffer,
        nReadSize,
	nTimeOut
	);

	WriteLog( " ReadDeviceData ReadEnd nResultSize = %d\n", nResultSize ) ;
	
	if (nResultSize == 0) {
		if (iReadStatus == 0) {
			iReadStatus = 1;
			gettimeofday(&save_tv, &save_tz);
		}
		else {
			iReadStatus = 2;
		}
	
	} else {
		iReadStatus = 0;
		return nResultSize; 
	}

	return nResultSize;
}
//-----------------------------------------------------------------------------
//
//	Function name:	ReadNonFixedData
//
//
//	Abstract:
//		�ǥХ�������ǡ�����꡼�ɤ���ʥ����ॢ���Ƚ��������
//
//
//	Parameters:
//		lpBuffer
//			�꡼�ɥǡ������Ǽ����Хåե��ؤΥݥ���
//
//		wReadSize
//			�꡼�ɤ���ǡ���������
//
//		dwTimeOut
//			�����ॢ���Ȼ��֡�ms��
//
//
//	Return values:
//		0 >  ���ｪλ���ºݤ˥꡼�ɤ����Х��ȿ�
//		0 = �����ॢ����
//		-1 = �꡼�ɥ��顼 
//
//	Note:
//		�ǡ����򣱥Х��ȤǤ���������齪λ���롣
//		�����ॢ���Ȥλ��֤ˤʤäƤ�����Ǥ��ʤ����ϡ���λ���롣
//-----------------------------------------------------------------------------
//
int
ReadNonFixedData( usb_dev_handle *hScanner, LPBYTE lpBuffer, WORD wReadSize, DWORD dwTimeOutMsec )
{
	int   nReadDataSize = 0;

	struct timeval start_tv, tv;
	struct timezone tz;
	long   nSec, nUsec;
	long   nTimeOutSec, nTimeOutUsec;

	iReadStatus = 0;

	if (gettimeofday(&start_tv, &tz) == -1)
		return FALSE;

	// �����ॢ�����ͤ���ñ�̤�׻����롣
	nTimeOutSec = dwTimeOutMsec / 1000; 
	// �����ॢ�����ͤΥޥ�������ñ�̤�׻����롣
	nTimeOutUsec = (dwTimeOutMsec - (1000 * nTimeOutSec)) * 1000;

	while(1){

		if (gettimeofday(&tv, &tz) == 0) {
			if (tv.tv_usec < start_tv.tv_usec) {
				tv.tv_usec += 1000 * 1000 ;
				tv.tv_sec-- ;
			}
			nSec = tv.tv_sec - start_tv.tv_sec;
			nUsec = tv.tv_usec - start_tv.tv_usec;

			if (nSec > nTimeOutSec) { // �����ॢ�����ͤ��ÿ�����礭�����h��ȴ���롣
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

		//
		// �ǡ������ɤߤ���
		//
		nReadDataSize = ReadDeviceData( hScanner, lpBuffer, wReadSize );
		if( nReadDataSize > 0 ){
			break;
		}
		else if (nReadDataSize < 0) {
			break;
		}

		usleep(20 * 1000); // 20ms�Ԥ�
	}

	return nReadDataSize;
}

//-----------------------------------------------------------------------------
//
//	Function name:	ReadFixedData
//
//
//	Abstract:
//		�ǥХ����������Ĺ�Υǡ�����꡼�ɤ���ʥ����ॢ���Ƚ��������
//
//
//	Parameters:
//		lpBuffer
//			�꡼�ɥǡ������Ǽ����Хåե��ؤΥݥ���
//
//		wReadSize
//			�꡼�ɤ���ǡ���������
//
//		dwTimeOut
//			�����ॢ���Ȼ��֡�ms��
//
//
//	Return values:
//		TRUE  = ���ｪλ
//		FALSE = �����ॢ���ȡʥ꡼�ɼ��ԡ�
//
//
//	Note:
//
//-----------------------------------------------------------------------------
//	ReadBidiFixedData�ʵ�ReadBidiComm32_q��
BOOL
ReadFixedData( usb_dev_handle *hScanner, LPBYTE lpBuffer, WORD wReadSize, DWORD dwTimeOutMsec )
{
	BOOL  bResult = TRUE;
	WORD  wReadCount = 0;
	int   nReadDataSize;

	struct timeval start_tv, tv;
	struct timezone tz;
	long   nSec, nUsec;
	long   nTimeOutSec, nTimeOutUsec;

	if (gettimeofday(&start_tv, &tz) == -1)
		return FALSE;

	// �����ॢ�����ͤ���ñ�̤�׻����롣
	nTimeOutSec = dwTimeOutMsec / 1000; 
	// �����ॢ�����ͤΥޥ�������ñ�̤�׻����롣
	nTimeOutUsec = (dwTimeOutMsec - (1000 * nTimeOutSec)) * 1000;

	while( wReadCount < wReadSize ){

		if (gettimeofday(&tv, &tz) == 0) {
			if (tv.tv_usec < start_tv.tv_usec) {
				tv.tv_usec += 1000 * 1000 ;
				tv.tv_sec-- ;
			}
			nSec = tv.tv_sec - start_tv.tv_sec;
			nUsec = tv.tv_usec - start_tv.tv_usec;

			if (nSec > nTimeOutSec) { // �����ॢ�����ͤ��ÿ�����礭�����h��ȴ���롣
				break;
			} 
			else if( nSec == nTimeOutSec) { // �����ॢ�����ͤ��ÿ���Ʊ�����
				if (nUsec >= nTimeOutUsec) { // �����ॢ�����ͤΥޥ������ÿ����礭������ȴ���롣
					break;
				}
			}
		}
		else {
			bResult = FALSE;
		}

		//
		// �ǡ������ɤߤ���
		//
		nReadDataSize = ReadDeviceData( hScanner, &lpBuffer[ wReadCount ], wReadSize - wReadCount );
		if( nReadDataSize > 0 ){
			wReadCount += nReadDataSize;
		}

		if( wReadCount >= wReadSize ) break;	// �꡼�ɤ���λ�����������ȴ����

		usleep(20 * 1000); // 20ms�Ԥ�
	}

	return bResult;
}

//-----------------------------------------------------------------------------
//
//	Function name:	ReadDeviceCommand
//
//
//	Abstract:
//		�ǥХ������饳�ޥ�ɤ�꡼�ɤ���
//
//
//	Parameters:
//		lpRxBuffer
//			�꡼�ɥХåե��ؤΥݥ���
//
//		nReadSize
//			�꡼�ɡ�������
//
//
//	Return values:
//		0 >  ���ｪλ���ºݤ˥꡼�ɤ����Х��ȿ�
//		0 <= �꡼�ɼ��ԡ����顼������
//
//-----------------------------------------------------------------------------
//
int
ReadDeviceCommand( usb_dev_handle *hScanner, LPBYTE lpRxBuffer, int nReadSize )
{
	int  nResultSize;


	nResultSize = ReadDeviceData( hScanner, lpRxBuffer, nReadSize );

	return nResultSize;
}


//-----------------------------------------------------------------------------
//
//	Function name:	WriteDeviceData
//
//
//	Abstract:
//		�ǥХ����˥ǡ�����饤�Ȥ���
//
//
//	Parameters:
//		lpTxBuffer
//			�饤�ȥǡ����ؤΥݥ���
//
//		nWriteSize
//			�饤�ȡ�������
//
//
//	Return values:
//		0 >  ���ｪλ���ºݤ˥饤�Ȥ����Х��ȿ�
//		0 <= �饤�ȼ��ԡ����顼������
//
//-----------------------------------------------------------------------------
//
int
WriteDeviceData( usb_dev_handle *hScanner, LPBYTE lpTxBuffer, int nWriteSize )
{
	int i;
	int  nResultSize = 0;

	for (i = 0; i < RETRY_CNT;i++) {
		nResultSize = usb_bulk_write(hScanner,
	        0x03,
	        lpTxBuffer,
	        nWriteSize,
	        2000
		);
		if ( nResultSize >= 0)
			break;
	}

	return nResultSize;
}


//-----------------------------------------------------------------------------
//
//	Function name:	WriteDeviceCommand
//
//
//	Abstract:
//		�ǥХ����˥��ޥ�ɤ�饤�Ȥ���
//
//
//	Parameters:
//		lpTxBuffer
//			���ޥ�ɤؤΥݥ���
//
//		nWriteSize
//			���ޥ�ɡ�������
//
//
//	Return values:
//		0 >  ���ｪλ���ºݤ˥饤�Ȥ����Х��ȿ�
//		0 <= �饤�ȼ��ԡ����顼������
//
//-----------------------------------------------------------------------------
//
int
WriteDeviceCommand( usb_dev_handle *hScanner, LPBYTE lpTxBuffer, int nWriteSize )
{
	int  nResultSize;


	nResultSize = WriteDeviceData( hScanner, lpTxBuffer, nWriteSize );

	return nResultSize;
}


//-----------------------------------------------------------------------------
//
//	Function name:	AllocReceiveBuffer
//
//
//	Abstract:
//		�����Хåե�����ݤ���
//
//
//	Parameters:
//		hWndDlg
//			����������Window�ϥ�ɥ�
//
//
//	Return values:
//		�����Хåե��ؤΥݥ���
//
//-----------------------------------------------------------------------------
//	AllocReceiveBuffer�ʵ�ScanStart�ΰ�����
HANDLE
AllocReceiveBuffer( DWORD  dwBuffSize )
{
	if( hReceiveBuffer == NULL ){
		//
		// �����Хåե��γ���
		//
		hReceiveBuffer = MALLOC( dwBuffSize );

		WriteLog( "ReceiveBuffer = %X, size = %d", hReceiveBuffer, dwBuffSize );
	}
	return hReceiveBuffer;
}


//-----------------------------------------------------------------------------
//
//	Function name:	FreeReceiveBuffer
//
//
//	Abstract:
//		�����Хåե����˴�����
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
//	FreeReceiveBuffer�ʵ�DRV_PROC/WM_DESTROY�ΰ�����
void
FreeReceiveBuffer( void )
{
	if( hReceiveBuffer != NULL ){
		FREE( hReceiveBuffer );
		hReceiveBuffer = NULL;

		WriteLog( "free ReceiveBuffer" );
	}
}


//////// end of brother_devaccs.c ////////
