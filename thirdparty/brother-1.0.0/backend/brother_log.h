///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
//	Source filename: brother_log.h
//
//		Copyright(c) 1997-2000 Brother Industries, Ltd.  All Rights Reserved.
//
//
//	Abstract:
//			���ե���������⥸�塼�롦�إå���
//
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifndef _BROTHER_LOG_H_
#define _BROTHER_LOG_H_


#define LOGFILENAME  "/BrMfc32.log"


#define RAWFILENAME  "/ScanData.raw"

#define LOGSTRMAXLEN  1024

//
// �ؿ��Υץ�ȥ��������
//
void  WriteLogFileString( LPSTR lpszLogStr );
void  WriteLog( LPSTR first, ... );
void  WriteLogScanCmd( LPSTR lpszId, LPSTR lpszCmd );
void  CloseLogFile( void );
void  OpenRawData( void );
void  CloseRawData( void );
void  SaveRawData( BYTE *lpRawData, int nDataSize );
void  GetLogSwitch( Brother_Scanner *this );


#endif //_BROTHER_LOG_H_


//////// end of brother_log.h ////////
