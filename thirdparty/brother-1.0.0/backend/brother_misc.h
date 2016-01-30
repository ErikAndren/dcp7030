///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
//	Source filename: brother_misc.h
//
//		Copyright(c) 1997-2000 Brother Industries, Ltd.  All Rights Reserved.
//
//
//	Abstract:
//			�Ƽ�ؿ����ʼ��ʸ��������ؿ��ˡ��إå���
//
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifndef _BROTHER_MISC_H_
#define _BROTHER_MISC_H_


//
// �ؿ��Υץ�ȥ��������
//
void    GetPathFromFileName( LPSTR lpszFileName );
void    MakePathName( LPSTR lpszPathName, LPSTR lpszAddName );
LPSTR   GetToken( LPSTR *lppszData );
WORD    StrToWord( LPSTR lpszText );
LPSTR   WordToStr( WORD wValue, LPSTR lpszTextTop );
short   StrToShort( LPSTR lpszText );
LPSTR   ShortToStr( short nValue, LPSTR lpszTextTop );
DWORD   StrToDword( LPSTR lpszText );
LPSTR   DwordToStr( DWORD dwValue, LPSTR lpszTextTop );

#endif //_BROTHER_MISC_H_


//////// end of brother_misc.h ////////
