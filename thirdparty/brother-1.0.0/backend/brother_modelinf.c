/*
;==============================================================================
;	Copyright 2003, STRIDE,LTD. all right reserved.
;==============================================================================
;	�ե�����̾		: brother_modelinf.c
;	��ǽ����		: ��ǥ��������
;	������			: 2003.08.06
;	�õ�����		:
;==============================================================================
;	�ѹ�����
;	����		������	������
;==============================================================================
*/
/*==========================================*/
/*		include �ե�����					*/
/*==========================================*/

#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<ctype.h>

#include	"brother.h"
#include	"brother_dtype.h"
#include	"brother_modelinf.h"

/*==========================================*/
/* 		�������Ѵؿ��ץ�ȥ��������		*/
/*==========================================*/

static int GetHexInfo(char *,int *);
static int GetDecInfo(char *,int *);
static int GetModelNo(char *,char *);
static int NextPoint(char *);
static int GetSeriesNo(PMODELINF,int *);				/* ���꡼���ֹ����				 */
static void GetSupportReso(int,PMODELCONFIG);			/* ���ݡ��Ȳ����ټ���			 */
static void GetSupportScanMode(int,PMODELCONFIG);		/* ���ݡ���ScanMode����			 */
static void GetSupportScanSrc(int,PMODELCONFIG);		/* ���ݡ���ScanSrc����			 */
static void GetSupportScanAreaHeight(int,PMODELCONFIG);	/* ���ݡ����ɤ߼���ϰ�Ĺ����	 */
static void GetSupportScanAreaWidth(int,PMODELCONFIG);	/* ���ݡ����ɤ߼���ϰ�������	 */
static void GetGrayLebelName(int,PMODELCONFIG);			/* ���쥤��٥��ѥǡ���̾����	 */
static void GetColorMatchName(int,PMODELCONFIG);		/* ���顼�ޥå����ѥǡ���̾����*/
static int GetFaxResoEnable(PMODELCONFIG);				/* FAX�Ѳ����٥ե饰����		 */
static int GetNoUseColorMatch(PMODELCONFIG);			/* ColorMacth̵���ե饰����		 */
static int GetCompressEnbale(PMODELCONFIG);				/* ����ͭ���ե饰����			 */
static int GetLogFile(PMODELCONFIG);			/* ���ե�����ե饰���� */
static int GetInBuffSize(PMODELCONFIG);					/* ���ϥХåե�����������		 */

static int SectionNameCheck(LPCTSTR,char *);
static int  KeyNameCheckInt(LPCTSTR,char *,int *);
static int KeyNameCheckString(LPCTSTR, char *);
static void GetKeyValueString(LPTSTR, int,char *,int *);
static int AllSectionName(LPTSTR, int,char *,int *);
static int AllKeyName(LPTSTR,int,char *,int *);
static int GetModelInfoSize(int *,int *,char *);
static int GetModelInfoKeyValueSize(LPCTSTR,int *,char *);
static int AllReadModelInfo(LPTSTR, int, char *,int *);

static int ScanModeIDString(int, char *);
static int ResoIDInt(int, int *);
static int ScanSrcIDString(int, char *);

/*==========================================*/
/* 		���������ѿ����					*/
/*==========================================*/

static PMODELINF modelListStart;
static int modelListGetEnable = FALSE;


static const char bwString[]            = "Black & White";
static const char errDiffusionString[]  = "Gray[Error Diffusion]";
static const char tGrayString[]         = "True Gray";
static const char ColorString[]       = "24bit Color";
static const char ColorFastString[]   = "24bit Color[Fast]";

static const char fbString[] = "FlatBed";
static const char adfString[] = "Automatic Document Feeder";

#define MAX_PATH 256

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: init_model_info
;	��ǽ����		: �б���ǥ��������
;	����			: �ʤ�
;	�����			: �¹Է��(TRUE:����,FALSE:����ե����뤬¸�ߤ��ʤ�����[SupportModel]��¸�ߤ��ʤ�)
;	������			: 2003.08.06
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int init_model_info(void)
{
	char		*readModelInfo;
	char		*modelRecord;
	char		modelTypeNo[BUF_SIZE];
	char		*recordPoint;
	char		*readInfoPoint;
	PMODELINF	model;

	int	size;
	int	structSize;
	int	modelTypeSize;
	int	modelNameSize;
	int	recordLength;
	int	record;
	int	dummy;
	int	res;
	int	count;
	char szFileName[MAX_PATH];

	strcpy( szFileName, BROTHER_SANE_DIR );
	strcat( szFileName, INIFILE_NAME );
	modelListGetEnable = ReadModelInfoSize(SUPORT_MODEL_SECTION,NULL, &size, &record, szFileName);/* �������ե����뤫���ǥ����Υ���������� */
	if(modelListGetEnable != TRUE)																	/* �������������� */
		return modelListGetEnable;																	/* ���顼 return */
	if(NULL == (readModelInfo = MALLOC( size + record + 1)))										/* ��ǥ������Ǽ���뤿����ΰ����� */
	{
		/* �ΰ���ݤ˼��� */
		modelListGetEnable = FALSE;
		return modelListGetEnable;		/* ���顼 return */
	}
	modelListGetEnable = ReadModelInfo(SUPORT_MODEL_SECTION, readModelInfo, size, szFileName);	/* �������ե����뤫���ǥ�������� */
	if(modelListGetEnable != TRUE)																	/* ��ǥ����������� */
	{
		FREE(readModelInfo);			/* ���ݤ����ΰ賫�� */
		return modelListGetEnable;		/* ���顼 return */
	}
	if(NULL == (modelListStart = MALLOC( (structSize=sizeof(MODELINF)) * (record+1))))	/* ��ǥ������Ǽ���뤿����ΰ�����(�Ǹ�ϥ��ߡ�) */
	{
		/* �ΰ���ݤ˼��� */
		modelListGetEnable = FALSE;
		FREE(readModelInfo);			/* ���ݤ����ΰ賫�� */
		return modelListGetEnable;		/* ���顼 return */
	}
	model = modelListStart;
	readInfoPoint = readModelInfo;
	count = 0;
	while(1)
	{
		count++;												/* �쥳���ɿ��û� */
		model->next = NULL_C;									/* ���Υ�ǥ����Υݥ��󥿤�NULL����Ͽ */
		recordLength = strlen(readInfoPoint);					/* �쥳���ɤ�Ĺ������� */
		if(NULL == ( modelRecord = MALLOC(recordLength+1)))		/* 1�쥳���ɤ��ΰ���� */
		{
			/* ERR���� */
			(model-1)->next = NULL_C;
			exit_model_info();									/* �����ޤǳ��ݤ����ΰ�����Ƴ��� */
			modelListGetEnable = FALSE;
			break;
		}
		strcpy(modelRecord,readInfoPoint);						/* 1�쥳���ɤ�ʬ�� */
		readInfoPoint += recordLength+1;						/* �쥳���ɥݥ��󥿤�ʤ�� */
		recordPoint = modelRecord;								/* ����򤵤��ݥ��󥿤˥��å� */
		res = GetHexInfo(recordPoint,&(model->productID));		/* �ץ������ID���� */
		recordPoint += NextPoint(recordPoint);					/* �����ؤ��ݥ��󥿤�ʤ�� */
		res *= GetDecInfo(recordPoint,&(model->seriesNo));		/* ���꡼���ֹ���� */
		recordPoint += NextPoint(recordPoint);					/* �����ؤ��ݥ��󥿤�ʤ�� */
		res *= GetModelNo(recordPoint,modelTypeNo);				/* ��ǥ륿�����ֹ���� */
		modelTypeSize =0;
		if(res == TRUE)
			/* ��ǥ륿����̾���������� */
			res *= ReadModelInfoSize(MODEL_TYPE_NAME_SECTION,modelTypeNo, &modelTypeSize, &dummy, szFileName);
		if(NULL == (model->modelTypeName = MALLOC(modelTypeSize+1)) || res == FALSE)	/* ��ǥ륿����̾�ΰ���� */
		{
			/* ERR���� */
			if(res == FALSE && NULL != (model->modelTypeName))
				FREE(model->modelTypeName);
			FREE(modelRecord);
			(model-1)->next = NULL_C;
			exit_model_info();									/* �����ޤǳ��ݤ����ΰ�����Ƴ��� */
			modelListGetEnable = FALSE;
			break;
		}
		ReadInitFileString(MODEL_TYPE_NAME_SECTION,modelTypeNo,ERR_STRING,model->modelTypeName,modelTypeSize,szFileName);	/* ��ǥ륿����̾���� */
		if(NULL ==(recordPoint = strchr(recordPoint,',')) || 0 == strcmp(model->modelTypeName,ERR_STRING))	/* �����ؤ��ݥ��󥿤�ʤ�� */
		{
			/* Err���� */
			FREE(modelRecord);
			FREE(model->modelTypeName);
			(model-1)->next = NULL_C;
			exit_model_info();						/* �����ޤǳ��ݤ����ΰ�����Ƴ��� */
			modelListGetEnable = FALSE;
			break;
		}
		recordPoint++;
		if(NULL !=strchr(recordPoint,','))
		{
			/* Err���� */
			FREE(modelRecord);
			FREE(model->modelTypeName);
			(model-1)->next = NULL_C;
			exit_model_info();						/* �����ޤǳ��ݤ����ΰ�����Ƴ��� */
			modelListGetEnable = FALSE;
			break;
		}
		modelNameSize = strlen(recordPoint);								/* ��ǥ�̾���������� */
		if(*recordPoint == '\"' && *(recordPoint+modelNameSize-1) == '\"')	/* ��ǥ�̾��"�ǰϤޤ�Ƥ�����Ϥ��� */
		{
			*(recordPoint+modelNameSize-1) = NULL_C;
			recordPoint++;
			modelNameSize --;
		}
		if(NULL == (model->modelName = MALLOC(modelNameSize+1)))			/* ��ǥ�̾���ΰ���� */
		{
			/* Err���� */
			FREE(modelRecord);
			FREE(model->modelTypeName);
			(model-1)->next = NULL_C;
			exit_model_info();						/* �����ޤǳ��ݤ����ΰ�����Ƴ��� */
			modelListGetEnable = FALSE;
			break;
		}
		strcpy(model->modelName,recordPoint);		/* ��ǥ�̾���� */
		recordPoint += NextPoint(recordPoint)-1;	/* �����ؤ��ݥ��󥿤�ʤ�� */
		FREE(modelRecord);							/* �쥳���ɤ��ΰ���� */
		if(count >= record)							/* ��³���ֳ�ǧ */
		{
			/* ��λ���� */
			modelListGetEnable = TRUE;
			break;
		}
		model->next = model+1;						/* ���Υ�ǥ����Υݥ��󥿤���Ͽ */
		model = model->next;						/* ���Υ�ǥ����¤�Τ� */
	}

	FREE(readModelInfo);
	return modelListGetEnable;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: GetHexInfo
;	��ǽ����		: �쥳���ɤ�","�Ƕ��ڤ�줿���ι��ܤ�16�ʿ��������ͤ��������
;	����			: �쥳���ɥݥ��󥿡���Ǽ����ݥ���
;	�����			: ���(TRUE:����,FALSE:���顼)
;	������			: 2003.08.06
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/
int GetHexInfo(char *modelRecord,int *receiveInfo)
{
	char	para[BUF_SIZE];
	char	*comma_pt;
	int		res;

	res = FALSE;
	comma_pt = strchr(modelRecord,',');			/* ","�ΰ��֤�Ĵ�٤� */
	if(comma_pt != NULL)
	{
		*comma_pt = NULL_C;
		strcpy(para,modelRecord);
		*receiveInfo = strtol(para,(char **)(para+strlen(para)),16);	/* 16�ʿ����Ѵ����� */
		res = TRUE;
	}

	return res;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: GetDecInfo
;	��ǽ����		: �쥳���ɤ�","�Ƕ��ڤ�줿���ι��ܤ�10�ʿ��������ͤ��������
;	����			: �쥳���ɥݥ��󥿡���Ǽ����ݥ���
;	�����			: ���(TRUE:����,FALSE:���顼)
;	������			: 2003.08.06
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/
int GetDecInfo(char *modelRecord,int *receiveInfo)
{
	char	para[BUF_SIZE];
	char	*comma_pt;
	int		res;

	res = FALSE;
	comma_pt = strchr(modelRecord,',');			/* ","�ΰ��֤�Ĵ�٤� */
	if(comma_pt != NULL)
	{
		strcpy(para,modelRecord);
		*comma_pt = NULL_C;
		*receiveInfo = strtol(para,(char **)(para+strlen(para)),10);	/* 10�ʿ����Ѵ����� */
		res = TRUE;
	}

	return res;
}


/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: NextPoint
;	��ǽ����		: NULL�Ƕ��ڤ�줿���ι��ܤޤǤ��������
;	����			: ��ư������ݥ���
;	�����			: ���(TRUE:����,FALSE:���ι��ܤ��ʤ�)
;	������			: 2003.08.06
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/
int NextPoint(char *point)
{
	int length;

	if(1 <= (length = strlen(point)))		/* ʸ�����Ĺ����Ĵ�٤� */
		length++;							/* ʸ���󤬤����+1(���ι��ܤ�NULL�θ�) */
	else
		length = 0;							/* ʸ���󤬤ʤ���У��� */
	return length;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: GetModelNo
;	��ǽ����		: ��ǥ륿�����ֹ���������
;	����			: �쥳���ɥݥ��󥿡���Ǽ����ݥ���
;	�����			: ���(TRUE:����,FALSE:���顼)
;	������			: 2003.08.06
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int GetModelNo(char *modelRecord,char *modelTypeNo)					/* ��ǥ륿�����ֹ���� */
{
	int		length;
	int		res;

	res = FALSE;
	length = strcspn(modelRecord,",");				/* ","�ΰ��֤�Ĵ�٤� */
	if(length != 0)
	{
		strncpy(modelTypeNo,modelRecord,length);	/* ʸ����ι��ܤ������ԡ� */
		*(modelTypeNo+length) = NULL_C;				/* �Ǹ��NULL���ղ� */
		res = TRUE;
	}
	return res;
}


/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: get_model_info
;	��ǽ����		: �б���ǥ�������
;	����			: model_inf��¤�ΤΥݥ���
;	�����			: �������(TRUE:����,FALSE:����ե����뤬¸�ߤ��ʤ�����[SupportModel]��¸�ߤ��ʤ�)
;	������			: 2003.08.07
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int get_model_info(PMODELINF modelInfList)
{
	int	res;

	res = FALSE;
	if(modelListGetEnable == TRUE )
	{
		modelInfList->modelName = modelListStart->modelName;			/* ��ǥ�ꥹ�Ȥξ�����Ϥ� */
		modelInfList->modelTypeName = modelListStart->modelTypeName;	/* ��ǥ륿����̾���Ϥ� */
		modelInfList->next = modelListStart->next;						/* ���Υ�ǥ����¤�ΤΥݥ��󥿤��Ϥ� */
		modelInfList->productID = modelListStart->productID;			/* �ץ������ID���Ϥ� */
		modelInfList->seriesNo = modelListStart->seriesNo;				/* ���ꥢ��NO���Ϥ� */
		res = TRUE;
	}
	return res;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: exit_model_info
;	��ǽ����		: �б���ǥ����λ����
;	����			: �ʤ�
;	�����			: �������(TRUE:����,FALSE:����ե����뤬¸�ߤ��ʤ�����[SupportModel]��¸�ߤ��ʤ�)
;	������			: 2003.08.07
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int exit_model_info(void)
{
	MODELINF  modelInfList;
	PMODELINF model;
	int res;

	res = get_model_info(&modelInfList);
	if(res == TRUE)
	{
		model = &modelInfList;
		while(1)
		{
			FREE(model->modelName);			/* ��ǥ�̾���ΰ���� */
			FREE(model->modelTypeName);		/* ��ǥ륿����̾���ΰ���� */
			if(model->next == NULL)			/* ���Υ�ǥ빽¤�Τ����뤫? */
			{
				FREE(modelListStart);		/* �ʤ���С�����ǥ빽¤�Τ��ΰ���� */
				modelListGetEnable = FALSE;
				break;
			}
			model = model->next;			/* ���Υ�ǥ빽¤�Τ� */
		}
	}
	return res;

}



/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: get_model_config
;	��ǽ����		: �б���ǥ��������
;	����			: ��ǥ����¤�Υݥ���,��ǥ����깽¤�Υݥ���
;	�����			: �¹Է��(TRUE:����,FALSE:����ե����뤬¸�ߤ��ʤ�)
;	������			: 2003.08.06
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int get_model_config(PMODELINF modelInf,PMODELCONFIG modelConfig)
{
	int	res;
	int series;

	res = GetSeriesNo(modelInf,&series);			/* ���꡼���ֹ����				 */
	GetSupportReso(series,modelConfig);				/* ���ݡ��Ȳ����ټ���			 */
	GetSupportScanMode(series,modelConfig);			/* ���ݡ���ScanMode����			 */
	GetSupportScanSrc(series,modelConfig);			/* ���ݡ���ScanSrc����			 */
	GetSupportScanAreaHeight(series,modelConfig);	/* ���ݡ����ɤ߼���ϰ�Ĺ����	 */
	GetSupportScanAreaWidth(series,modelConfig);	/* ���ݡ����ɤ߼���ϰ�������	 */
	GetGrayLebelName(series,modelConfig);			/* ���쥤��٥��ѥǡ���̾����	 */
	GetColorMatchName(series,modelConfig);			/* ���顼�ޥå����ѥǡ���̾���� */
	res *= GetFaxResoEnable(modelConfig);			/* FAX�Ѳ����٥ե饰����		 */
	res *= GetNoUseColorMatch(modelConfig);			/* ColorMacth̵���ե饰����		 */
	res *= GetCompressEnbale(modelConfig);			/* ����ͭ���ե饰���� */
	res *= GetLogFile(modelConfig);				/* ���ե饰���� */

	res *= GetInBuffSize(modelConfig);				/* ���ϥХåե�����������		 */

	return res;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: GetSeriesNo
;	��ǽ����		: ���꡼���ֹ����
;	����			: ��ǥ����¤�Υݥ���,���꡼���ֹ��Ǽ�ݥ���
;	�����			: �ʤ�
;	������			: 2003.08.11
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/
int GetSeriesNo(PMODELINF modelInf,int *series)
{
	int res;

  	*series = modelInf->seriesNo;
	if(*series <= 0 || MAX_SERIES_NO < *series)		/* ���꡼���ֹ椬�۾�? */
		res = FALSE;								/* ���顼 */
	else
		res = TRUE;
	return res;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: GetSupportReso
;	��ǽ����		: ���ݡ��Ȳ����ټ���
;	����			: ���꡼���ֹ�,��ǥ����깽¤�Υݥ���
;	�����			: �ʤ�
;	������			: 2003.08.11
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/
void GetSupportReso(int series,PMODELCONFIG modelConfig)
{
	modelConfig->SupportReso.val   = 0x0000;						/* ����� */
	switch(series)
	{
		case	YL4_SF_TYPE:
			modelConfig->SupportReso.bit.bDpi100x100   = TRUE;		/*  100 x  100 dpi */
			modelConfig->SupportReso.bit.bDpi150x150   = TRUE;		/*  150 x  150 dpi */
			modelConfig->SupportReso.bit.bDpi200x200   = TRUE;		/*  200 x  200 dpi */
			modelConfig->SupportReso.bit.bDpi300x300   = TRUE;		/*  300 x  300 dpi */
			modelConfig->SupportReso.bit.bDpi400x400   = TRUE;		/*  400 x  400 dpi */
			modelConfig->SupportReso.bit.bDpi600x600   = TRUE;		/*  600 x  600 dpi */
			modelConfig->SupportReso.bit.bDpi1200x1200 = TRUE;		/* 1200 x 1200 dpi */
			modelConfig->SupportReso.bit.bDpi2400x2400 = FALSE;		/* 2400 x 2400 dpi */
			modelConfig->SupportReso.bit.bDpi4800x4800 = FALSE;		/* 4800 x 4800 dpi */
			modelConfig->SupportReso.bit.bDpi9600x9600 = FALSE;		/* 9600 x 9600 dpi */
			break;

		case	BHL_SF_TYPE:
		case	BHL2_SF_TYPE:
			modelConfig->SupportReso.bit.bDpi100x100   = TRUE;		/*  100 x  100 dpi */
			modelConfig->SupportReso.bit.bDpi150x150   = TRUE;		/*  150 x  150 dpi */
			modelConfig->SupportReso.bit.bDpi200x200   = TRUE;		/*  200 x  200 dpi */
			modelConfig->SupportReso.bit.bDpi300x300   = TRUE;		/*  300 x  300 dpi */
			modelConfig->SupportReso.bit.bDpi400x400   = TRUE;		/*  400 x  400 dpi */
			modelConfig->SupportReso.bit.bDpi600x600   = TRUE;		/*  600 x  600 dpi */
			modelConfig->SupportReso.bit.bDpi1200x1200 = TRUE;		/* 1200 x 1200 dpi */
			modelConfig->SupportReso.bit.bDpi2400x2400 = TRUE;		/* 2400 x 2400 dpi */
			modelConfig->SupportReso.bit.bDpi4800x4800 = FALSE;		/* 4800 x 4800 dpi */
			modelConfig->SupportReso.bit.bDpi9600x9600 = FALSE;		/* 9600 x 9600 dpi */
			break;

		default:
			modelConfig->SupportReso.bit.bDpi100x100   = TRUE;		/*  100 x  100 dpi */
			modelConfig->SupportReso.bit.bDpi150x150   = TRUE;		/*  150 x  150 dpi */
			modelConfig->SupportReso.bit.bDpi200x200   = TRUE;		/*  200 x  200 dpi */
			modelConfig->SupportReso.bit.bDpi300x300   = TRUE;		/*  300 x  300 dpi */
			modelConfig->SupportReso.bit.bDpi400x400   = TRUE;		/*  400 x  400 dpi */
			modelConfig->SupportReso.bit.bDpi600x600   = TRUE;		/*  600 x  600 dpi */
			modelConfig->SupportReso.bit.bDpi1200x1200 = TRUE;		/* 1200 x 1200 dpi */
			modelConfig->SupportReso.bit.bDpi2400x2400 = TRUE;		/* 2400 x 2400 dpi */
			modelConfig->SupportReso.bit.bDpi4800x4800 = TRUE;		/* 4800 x 4800 dpi */
			modelConfig->SupportReso.bit.bDpi9600x9600 = TRUE;		/* 9600 x 9600 dpi */
	}

	return;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: GetSupportScanMode
;	��ǽ����		: ���ݡ���ScanMode����
;	����			: ���꡼���ֹ�,��ǥ����깽¤�Υݥ���
;	�����			: �ʤ�
;	������			: 2003.08.11
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/
void GetSupportScanMode(int series,PMODELCONFIG modelConfig)
{
	modelConfig->SupportScanMode.val = 0x0000;								/* ����� */
	switch(series)
	{
		case	YL4_SF_TYPE:
		case	ZLE_SF_TYPE:
		case	ZL2_SF_TYPE:
			modelConfig->SupportScanMode.bit.bBlackWhite     = TRUE;		/* ����(���)		*/
			modelConfig->SupportScanMode.bit.bErrorDiffusion = TRUE;		/* ���Ȼ�			*/
			modelConfig->SupportScanMode.bit.bTrueGray       = TRUE;		/* ���졼��������	*/
			modelConfig->SupportScanMode.bit.b24BitColor     = FALSE;		/* 24bit���顼		*/
			modelConfig->SupportScanMode.bit.b24BitNoCMatch  = FALSE;		/* 24bit���顼��®��ColorMatch�ʤ���*/
			break;

		case	YL4_FB_DCP:
		case	ZLE_FB_DCP:
		case	ZL2_FB_DCP:
			modelConfig->SupportScanMode.bit.bBlackWhite     = TRUE;		/* ����(���)		*/
			modelConfig->SupportScanMode.bit.bErrorDiffusion = TRUE;		/* ���Ȼ�			*/
			modelConfig->SupportScanMode.bit.bTrueGray       = TRUE;		/* ���졼��������	*/
			modelConfig->SupportScanMode.bit.b24BitColor     = TRUE;		/* 24bit���顼		*/
			modelConfig->SupportScanMode.bit.b24BitNoCMatch  = TRUE;		/* 24bit���顼��®��ColorMatch�ʤ���*/
			break;

		default:
			modelConfig->SupportScanMode.bit.bBlackWhite     = TRUE;		/* ����(���)		*/
			modelConfig->SupportScanMode.bit.bErrorDiffusion = TRUE;		/* ���Ȼ�			*/
			modelConfig->SupportScanMode.bit.bTrueGray       = TRUE;		/* ���졼��������	*/
			modelConfig->SupportScanMode.bit.b24BitColor     = TRUE;		/* 24bit���顼		*/
			modelConfig->SupportScanMode.bit.b24BitNoCMatch  = FALSE;		/* 24bit���顼��®��ColorMatch�ʤ���*/
			break;
	}
	return;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: GetSupportScanSrc
;	��ǽ����		: ���ݡ���ScanSrc����
;	����			: ���꡼���ֹ�,��ǥ����깽¤�Υݥ���
;	�����			: �ʤ�
;	������			: 2003.08.11
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/
void GetSupportScanSrc(int series,PMODELCONFIG modelConfig)
{
	modelConfig->SupportScanSrc.val = 0x0000;					/* ����� */
	switch(series)
	{
		case	YL4_SF_TYPE:
		case	ZLE_SF_TYPE:
		case	ZL2_SF_TYPE:
		case	BHL_SF_TYPE:
		case	BHL2_SF_TYPE:
			modelConfig->SupportScanSrc.bit.FB     = FALSE;		/* FlatBed				*/
			modelConfig->SupportScanSrc.bit.ADF    = TRUE;		/* AutoDocumentFeeder	*/
			break;

		case	YL4_FB_DCP:
		case	ZLE_FB_DCP:
		case	ZL2_FB_DCP:
		case	BHL_FB_DCP:
		case	BHM_FB_TYPE:
		case	BHL2_FB_DCP:
			modelConfig->SupportScanSrc.bit.FB     = TRUE;		/* FlatBed				*/
			modelConfig->SupportScanSrc.bit.ADF    = TRUE;		/* AutoDocumentFeeder	*/
			break;

		case	BHMINI_FB_ONLY:
			modelConfig->SupportScanSrc.bit.FB     = TRUE;		/* FlatBed				*/
			modelConfig->SupportScanSrc.bit.ADF    = FALSE;		/* AutoDocumentFeeder	*/
			break;
	}
	return;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: GetSupportScanAreaHeight
;	��ǽ����		: ���ݡ����ɤ߹����ϰ�Ĺ����
;	����			: ���꡼���ֹ�,��ǥ����깽¤�Υݥ���
;	�����			: �ʤ�
;	������			: 2003.08.21
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/
void GetSupportScanAreaHeight(int series,PMODELCONFIG modelConfig)
{
	switch(series)
	{
		case	BHMINI_FB_ONLY:
			modelConfig->SupportScanAreaHeight = 297.0;
			break;
		default:
			modelConfig->SupportScanAreaHeight = 355.6;
			break;
	}
	return;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: GetSupportScanAreaWidth
;	��ǽ����		: ���ݡ����ɤ߹����ϰ�������
;	����			: ���꡼���ֹ�,��ǥ����깽¤�Υݥ���
;	�����			: �ʤ�
;	������			: 2003.08.11
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/
void GetSupportScanAreaWidth(int series,PMODELCONFIG modelConfig)
{
	switch(series)
	{
		case	BHL2_SF_TYPE:
		case	BHL2_FB_DCP:
			modelConfig->SupportScanAreaWidth = 210.0;
			break;

		case	ZL2_FB_DCP:
			modelConfig->SupportScanAreaWidth = 212.0;
			break;

		default:
			modelConfig->SupportScanAreaWidth = 208.0;
			break;
	}
	return;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: GetGrayLebelName
;	��ǽ����		: ���쥤��٥��ѥǡ���̾
;	����			: ���꡼���ֹ�,��ǥ����깽¤�Υݥ���
;	�����			: �ʤ�
;	������			: 2003.08.11
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/
void GetGrayLebelName(int series,PMODELCONFIG modelConfig)
{
	char	*name;

	switch(series)						/* ���꡼��No����Name����� */
	{
		case	YL4_SF_TYPE:
			name = YL4_SF_TYPE_NAME;
			break;
		case	YL4_FB_DCP:
			name = YL4_FB_DCP_NAME;
			break;
		case	ZLE_SF_TYPE:
			name = ZLE_SF_TYPE_NAME;
			break;
		case	ZLE_FB_DCP:
			name = ZLE_FB_DCP_NAME;
			break;
		case	ZL2_SF_TYPE:
			name = ZL2_SF_TYPE_NAME;
			break;
		case	ZL2_FB_DCP:
			name = ZL2_FB_DCP_NAME;
			break;
		case	BHL_SF_TYPE:
			name = BHL_SF_TYPE_NAME;
			break;
		case	BHL_FB_DCP:
			name = BHL_FB_DCP_NAME;
			break;
		case	BHM_FB_TYPE:
			name = BHM_FB_TYPE_NAME;
			break;
		case	BHMINI_FB_ONLY:
			name = BHMINI_FB_ONLY_NAME;
			break;
		case	BHL2_SF_TYPE:
			name = BHL2_SF_TYPE_NAME;
			break;
		case	BHL2_FB_DCP:
			name = BHL2_FB_DCP_NAME;
			break;
		default:
			name = NULL_S;				/* ���������Τ��ʤ����NULL */
	}
	strcpy(modelConfig->szGrayLebelName,name);
	return;
}


/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: GetColorMatchName
;	��ǽ����		: ���顼�ޥå����ѥǡ���̾
;	����			: ���꡼���ֹ�,��ǥ����깽¤�Υݥ���
;	�����			: �ʤ�
;	������			: 2003.08.11
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

void GetColorMatchName(int series,PMODELCONFIG modelConfig)
{
	char	*name;

	/* ���꡼���ֹ�ˤ��Name����� */
	switch(series)
	{
		case	YL4_FB_DCP:
			name = YL4_FB_DCP_CM_NAME;
			break;
		case	ZLE_FB_DCP:
			name = ZLE_FB_DCP_CM_NAME;
			break;
		case	ZL2_FB_DCP:
			name = ZL2_FB_DCP_CM_NAME;
			break;
		default:
			name = NULL_S;	/* ���������Τ��ʤ����NULL */
			break;
	}
	strcpy(modelConfig->szColorMatchName,name);
	return;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: GetFaxResoEnable
;	��ǽ����		: FAX�Ѳ����٥ե饰����
;	����			: ��ǥ����깽¤�Υݥ���
;	�����			: ��������(TRUE:����,FALSE:����)
;	������			: 2003.08.11
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int GetFaxResoEnable(PMODELCONFIG modelConfig)
{
	int	res;
	char szFileName[MAX_PATH];

	strcpy( szFileName, BROTHER_SANE_DIR );
	strcat( szFileName, INIFILE_NAME );
	modelConfig->bFaxResoEnable = (BYTE)ReadInitFileInt(DRIVER_SECTION,FAX_RESO_KEY,ERROR_INT,szFileName);
	if(modelConfig->bFaxResoEnable == 0 || modelConfig->bFaxResoEnable == 1)	/* �ͤ�����? */
		res = TRUE;				/* �������� */
	else
		res = FALSE;			/* �������� */
	return res;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: GetNoUseColorMatch
;	��ǽ����		: ColorMatch̵���ե饰����
;	����			: ��ǥ����깽¤�Υݥ���
;	�����			: ��������(TRUE:����,FALSE:����)
;	������			: 2003.08.11
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int GetNoUseColorMatch(PMODELCONFIG modelConfig)
{
	int	res;
	char szFileName[MAX_PATH];

	strcpy( szFileName, BROTHER_SANE_DIR );
	strcat( szFileName, INIFILE_NAME );
	modelConfig->bNoUseColorMatch = (BYTE)ReadInitFileInt(DRIVER_SECTION,NO_USE_CM_KEY,ERROR_INT,szFileName);
	if(modelConfig->bNoUseColorMatch == 0 || modelConfig->bNoUseColorMatch == 1)	/* �ǥե������? */
		res = TRUE;				/* �������� */
	else
		res = FALSE;			/* �������� */
	return res;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: GetCompressEnbale
;	��ǽ����		: ����ͭ���ե饰����
;	����			: ��ǥ����깽¤�Υݥ���
;	�����			: ��������(TRUE:����,FALSE:����)
;	������			: 2003.08.11
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int GetCompressEnbale(PMODELCONFIG modelConfig)
{
	int	res;
	char szFileName[MAX_PATH];

	strcpy( szFileName, BROTHER_SANE_DIR );
	strcat( szFileName, INIFILE_NAME );
	modelConfig->bCompressEnbale = (BYTE)ReadInitFileInt(DRIVER_SECTION,COMPRESS_KEY,ERROR_INT,szFileName);
	if(modelConfig->bCompressEnbale == 0 || modelConfig->bCompressEnbale ==1)	/* �ǥե������? */
		res = TRUE;				/* �������� */
	else
		res = FALSE;			/* �������� */
	return res;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: GetLogFile
;	��ǽ����		: ���ե�����ե饰����
;	����			: ��ǥ����깽¤�Υݥ���
;	�����			: ��������(TRUE:����,FALSE:����)
;	������			: 2003.08.11
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int GetLogFile(PMODELCONFIG modelConfig)
{
	int	res;
	char szFileName[MAX_PATH];

	strcpy( szFileName, BROTHER_SANE_DIR );
	strcat( szFileName, INIFILE_NAME );
	modelConfig->bLogFile = (BYTE)ReadInitFileInt(DRIVER_SECTION,LOGFILE_KEY,ERROR_INT,szFileName);
	if(modelConfig->bLogFile == 0 || modelConfig->bLogFile ==1)	/* �ǥե������? */
		res = TRUE;				/* �������� */
	else
		res = FALSE;			/* �������� */
	return res;
}
/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: GetInBuffSize
;	��ǽ����		: ���ϥХåե�����������
;	����			: ��ǥ����깽¤�Υݥ���
;	�����			: ��������(TRUE:����,FALSE:����)
;	������			: 2003.08.11
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int GetInBuffSize(PMODELCONFIG modelConfig)
{
	int	res;
	int bufsize;
	char szFileName[MAX_PATH];

	strcpy( szFileName, BROTHER_SANE_DIR );
	strcat( szFileName, INIFILE_NAME );
	bufsize = ReadInitFileInt(DRIVER_SECTION,IN_BUF_KEY,-1,szFileName );
	if( 0 <= bufsize || bufsize < WORD_MAX)		/* �Хåե������������ʲ���WORD�θ³��ʾ�? */
	{
		res = TRUE;							/* �������� */
		modelConfig->wInBuffSize = (WORD)bufsize;
	}
	else
	{
		res = FALSE;							/* �����۾� */
		modelConfig->wInBuffSize = 0;
	}
	return res;
}






/*
/////////////////////////////////////////////////
////////       read_ini_file�ؿ�         ////////
/////////////////////////////////////////////////
*/

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: ReadInitFileInt
;	��ǽ����		: Ini�ե������Key���ͤ������ˤ����֤�
;	����			: õ��SectionName,õ��lpKeyName,�ǥե������,�����ե�����̾
;	�����			: ��������Key����(���Ĥ���ʤ����ϥǥե������,�����Ǥʤ�����0)
;	������			: 2003.07.31
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int ReadInitFileInt( LPCTSTR lpAppName,		/*  address of section name */
					 LPCTSTR lpKeyName,		/*  address of key name */
					 int nDefault,			/*  return value if key name is not found */
					 LPCTSTR lpFileName)	/*  address of initialization filename */
{
	int		result;
	int		sectionFind;
	int		keyFind;
	FILE	*rfile;
	char	buf[BUF_SIZE];
	char	state;

	state = 0;
	result = nDefault;												/* �֤��ͤΥǥե���� */
	if(NULL == (rfile = fopen(lpFileName, "r")))	return result;	/* �ɤ߹��ߥե����뤬�����ʤ�:���顼:return */
	while(1)
	{
		if(feof(rfile))
		{
			break;													/* �ե����뤬����� */
		}
		else
		{
			if(NULL == fgets(buf,BUF_SIZE,rfile))		break;		/* �Ԥμ����˼���:��λ */
			if(state == SECTION_CHECK)
			{
				sectionFind = SectionNameCheck(lpAppName,buf);		/* SectionName��Ĵ�٤� */
				if(sectionFind == FIND )	state++;				/* ����Sectionͭ�� ���ι����� */
			}
			else
			{
				keyFind = KeyNameCheckInt(lpKeyName,buf,&result);	/* key���ͤ������ͤ��֤� */
				if(keyFind == FIND )	break;						/* ���Ĥ��ä��Τǽ�λ */
			}
		}
	}
//	fclose(rfile);										/* �ե�������Ĥ��� */
	return result;
}


/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: SectionNameCheck
;	��ǽ����		: ��������SectionName��õ��
;	����			: õ��SectionName,������ʸ����
;	�����			: �������(FINE:���Ĥ���,NOFINE:���Ĥ���ʤ�)
;	������			: 2003.07.31
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int SectionNameCheck(LPCTSTR lpAppName, char *buf)
{
	int		res;
	int		i;
	int		count;
	int		f_char;
	int		lp_char;
	char	*SectionNameEnd;

	res = NOFIND;
	if(*buf == '[' )
	{
		SectionNameEnd = strchr(buf,']');
		if(SectionNameEnd != NULL)
		{
			/* SectionName�Ǥ��� */
			*SectionNameEnd = NULL_C;				/*  ']'��NULL�� */

			for(i=1,count=0;i<BUF_SIZE; i++,count++)
			{
				f_char  = tolower(*(buf+i));			/* ��ʸ�����Ѵ� */
				lp_char = tolower(*(lpAppName+count));	/* ��ʸ�����Ѵ� */
				if(f_char != lp_char)		break;		/* ���פ��ʤ��Τǰ㤦 */
				else if(*(buf+i)== NULL_C)	res = FIND; /* �Ǹ��NULL */
			}
		}
	}
	return res;
}


/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: KeyNameCheckInt
;	��ǽ����		: ��������KeyName�����뤫Ĵ�١�����Key���ͤ������ͤ��֤�
;	����			: õ��KeyName,������ʸ����,�����ͳ�Ǽ��ݥ���
;	�����			: ��������Key����(���Ĥ���ʤ����ϥǥե������,�����Ǥʤ�����0)
;	������			: 2003.07.31
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int KeyNameCheckInt(LPCTSTR lpKeyName, char *buf, int *result)
{
	int		res;
	int		i;
	int		f_char;
	int		lp_char;
	char	*keyNameEnd;

	res= NOFIND;
	if(*buf != '[')									/* ��Ƭ��'['�Ǥʤ� */
	{
		keyNameEnd = strchr(buf,'=');
		if(keyNameEnd != NULL)
		{
			*keyNameEnd = NULL_C;					/*  '='��NULL�� */
			for(i=0;i<BUF_SIZE; i++)
			{
				f_char  = tolower(*(buf+i));		/* ��ʸ�����Ѵ� */
				lp_char = tolower(*(lpKeyName+i));	/* ��ʸ�����Ѵ� */
				if(f_char != lp_char)
				{
					break;
				}
				else if(*(buf+i)== NULL_C)
				{
					*result = atoi(keyNameEnd+1);		/* Key���ͤ������ͤ��Ѵ� */
					res = FIND;
				}
			}
		}
	}
	return res;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: ReadInitFileString
;	��ǽ����		: Ini�ե������Key���ͤ�ʸ����Ȥ����֤�
;	����			: õ��SectionName,õ��lpKeyName,�ǥե������,
;					: ��̤��Ǽ����Хåե�,�Хåե�������,�����ե�����̾
;	�����			: ��Ǽ����ʸ�����Ĺ��
;	������			: 2003.07.31
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/
int ReadInitFileString( LPCTSTR lpAppName,			/*  points to section name */
						LPCTSTR lpKeyName,			/*  points to key name */
						LPCTSTR lpDefault,			/*  points to default string */
						LPTSTR  lpReturnedString,	/*  points to destination buffer */
						int nSize,					/*  size of destination buffer */
						LPCTSTR lpFileName)			/*  points to initialization filename */
{
	int		result;
	int		count;
	int		sectionFind;
	int		checkEnd;
	FILE	*rfile;
	char	buf[BUF_SIZE];
	int		state;

	state = 0;
	count = 0;											/* lpAppName,lpKeyName��NULL�ξ��˻��� */
	strcpy(lpReturnedString,lpDefault);					/* buf�˥ǥե���Ȥ򥻥å� */
	result = strlen(lpDefault);							/* �֤��ͤ˥ǥե���Ȥ�Ĺ���򥻥å� */
	if(NULL != (rfile = fopen(lpFileName, "r")))		/* �ɤ߹��ߥե����뤬������ */
	{
		while(1)
		{
			if(feof(rfile))
			{
				break;
			}
			else
			{
				if(NULL == fgets(buf,BUF_SIZE,rfile))	/* ����ɤ߹��� */
				{
					*buf=NULL_C;
				}
				if(lpAppName == NULL_C)					/* lpAppName��NULL�ξ�� */
				{
					checkEnd = AllSectionName(lpReturnedString, nSize, buf,&count);	/* SectionName�ꥹ�Ȥ��֤� */
					if(feof(rfile) || checkEnd == END)					/* ��λ���� */
					{
						*(lpReturnedString + count) = NULL_C;			/* �Ǹ��2ʸ����NULL */
						if(count == 0)
							*(lpReturnedString + count+1) = NULL_C;
						else
							count--;									/* �Ǹ��NULL�Ͽ����ʤ� */
						result = count;
						break;
					}
				}
				else if(state == SECTION_CHECK)
				{
					sectionFind = SectionNameCheck(lpAppName,buf);		/* SectionName��Ĵ�٤� */
					if(sectionFind == FIND )	state++;				/* ����Sectionͭ�� ���ι�����*/
				}
				else
				{
					if(lpKeyName == NULL_C)
					{
						checkEnd = AllKeyName(lpReturnedString, nSize, buf,&count);	/* KeyName�ꥹ�Ȥ��֤� */
						if(feof(rfile) || checkEnd == END)						/* ��λ���� */
						{
							*(lpReturnedString + count) = NULL_C;				/* �Ǹ��2ʸ����NULL */
							if(count == 0)
								*(lpReturnedString + count + 1) = NULL_C;
							else
								count--;										/* �Ǹ��NULL�Ͽ����ʤ� */
							result = count;
							break;
						}
					}
					else
					{
						checkEnd = KeyNameCheckString(lpKeyName,buf);			/* key���ͤ�ʸ������֤� */
						if(checkEnd == END )
						{
							GetKeyValueString(lpReturnedString, nSize, buf,&result);
							break;
						}
					}
				}
			}
		}
//		fclose(rfile);										/* �ե�������Ĥ��� */
	}

	return result;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: KeyNameCheckString
;	��ǽ����		: KeyName̾�����פ��뤫Ĵ�٤�
;	����			: õ��KeyName,������ʸ����
;					: �Хåե�������,������FILE�ݥ���
;	�����			: ��Ǽ����ʸ�����Ĺ��
;	������			: 2003.07.31
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int KeyNameCheckString(LPCTSTR lpKeyName, char *buf)
{
	int		res;
	int		i;
	int		f_char;
	int		lp_char;
	char	*keyNameEnd;

	res= NOEND;
	if(*buf != '[')									/* ��Ƭ��'['�Ǥʤ� */
	{
		keyNameEnd = strchr(buf,'=');
		if(keyNameEnd != NULL)
		{
			*keyNameEnd = NULL_C;					/*  '='��NULL�� */
			for(i=0;i<BUF_SIZE; i++)
			{
				f_char  = tolower(*(buf+i));		/* ��ʸ�����Ѵ� */
				lp_char = tolower(*(lpKeyName+i));	/* ��ʸ�����Ѵ� */
				if(f_char != lp_char)	break;
				else if(*(buf+i)== NULL_C)	res = END;
			}
			*keyNameEnd = '=';						/*  �����᤹ */
		}
	}
	return res;
}


/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: GetkeyValueString
;	��ǽ����		: Key���ͤ�ʸ����Ǽ���
;	����			: ��̤��Ǽ����Хåե�,��Ǽ��Υ�����,KeyName�Τ���ʸ����,��Ǽʸ�����������ݥ���
;	�����			: �ʤ�
;	������			: 2003.08.04
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/
void GetKeyValueString(LPTSTR lpReturnedString, int nSize,char *buf,int *result)
{
	char	*keyValueStart;
	int		length;

	keyValueStart = strchr(buf,'=')+1;
	if((length = strlen(keyValueStart)) > (WORD)nSize)	/* Buf����������礭����������礭���� */
		*(keyValueStart+nSize-1) = NULL_C;
	if( *(keyValueStart+length-1)== LF && length != 0)
		*(keyValueStart+length-1) = NULL_C;				/* ���Ԥ�NULL�� */
	strcpy(lpReturnedString,(keyValueStart));			/* �ǡ������Ǽ */
	*result = strlen(lpReturnedString);					/* ��Ǽ�������٤Ƥ�ʸ�����Ĺ������� */
	return;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: AllSection
;	��ǽ����		: SectionName�Υꥹ�Ȥ��֤�
;	����			: ��̤��Ǽ����Хåե�,�Хåե�������,������ե�����ݥ���,��Ǽʸ�����������ݥ���
;	�����			: ������̡�����:0,�����������С�:1)
;	������			: 2003.07.31
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int AllSectionName(LPTSTR lpReturnedString, int nSize,char *buf,int *count)
{
	int		res;
	char	*SectionNameEnd;
	char	*movePoint;

	res = 0;
	if(*buf == '[' )
	{
		SectionNameEnd = strchr(buf,']');
		if(SectionNameEnd != NULL)
		{
			/* SectionName�Ǥ��� */
			*SectionNameEnd = NULL_C;						/*  ']'��NULL�� */
			if((*count) + strlen(buf+1) > (WORD)(nSize-2))	/* �Ǹ��2ʸ����NULL�ʤΤǳ�Ǽ�Ǥ������nSize-2 */
			{
				*(buf+1+(nSize-2)-(*count)) = NULL_C;		/* ��Ǽ�Ǥ����ϰϤθ��NULL */
				res = 1;
			}
			movePoint =(lpReturnedString+*count);			/* ��Ǽ�γ��ϰ��� */
			strcpy(movePoint,buf+1);						/* �ǡ������Ǽ */
			*count += strlen(buf+1)+1;						/* ����ʸ���������ʶ��ڤ��NULL�������� */
		}
	}
	return res;
}


/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: AllKey
;	��ǽ����		: KeyName�Υꥹ�Ȥ��֤�
;	����			: ��̤��Ǽ����Хåե�,�Хåե�������,������ե�����ݥ���,��Ǽʸ�����������ݥ���
;	�����			: ��Ǽ����ʸ�����Ĺ��
;	������			: 2003.07.31
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/
int AllKeyName(LPTSTR lpReturnedString, int nSize,char *buf,int *count)
{
	int		res;
	char	*KeyNameEnd;
	char	*movePoint;

	res = NOEND;
	if(*buf == '[' )										/* ����SectionName�ˤʤ�Τǽ�λ */
	{
		res = END;
	}
	else
	{
		KeyNameEnd = strchr(buf,'=');
		if(KeyNameEnd != NULL)
		{
			/* KeyName�Ǥ��� */
			*KeyNameEnd = NULL_C;							/*  '='��NULL�� */
			if((*count) + strlen(buf) > (WORD)(nSize-2))	/* �Ǹ��2ʸ����NULL�ʤΤǳ�Ǽ�Ǥ������nSize-2 */
			{
				*(buf+(nSize-2)-(*count)) = NULL_C;			/* ��Ǽ�Ǥ����ϰϤθ��NULL */
				res = END;
			}
			movePoint =(lpReturnedString+*count);			/* ��Ǽ�γ��ϰ��� */
			strcpy(movePoint,buf);							/* �ǡ������Ǽ */
			*count += strlen(buf)+1;						/* ����ʸ���������ʶ��ڤ��NULL�������� */
		}
	}
	return res;
}



/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: ReadModelInfoSize
;	��ǽ����		: �������ե����뤫���ǥ����Υ����������
;					: SectionName,KeyName�������Key���ͤΥ��������������
;	����			: �������������̾,�Хåե�������,�쥳���ɿ�,������ե�����̾
;	�����			: �������
;	������			: 2003.08.06
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/
int ReadModelInfoSize(LPCTSTR lpAppName, LPCTSTR lpKeyName, int *size, int *record, LPCTSTR lpFileName)
{
	int		result;
	int		sectionFind;
	int		checkEnd;
	FILE	*rfile;
	char	buf[BUF_SIZE];
	int		state;

	result = FALSE;
	*size = 0;
	*record = 0;
	state = SECTION_CHECK;
	if(!(rfile = fopen(lpFileName, "r")))		/* �ɤ߹��ߥե����뤬������ */
		return result;

	while(1)
	{
		if(feof(rfile))
		{
			break;
		}
		else
		{
			if(NULL ==fgets(buf,BUF_SIZE,rfile))
			{
				break;
			}
			if(state == SECTION_CHECK)
			{
				sectionFind = SectionNameCheck(lpAppName,buf);		/* SectionName��Ĵ�٤� */
				if(sectionFind == FIND )	state++;				/* ����Sectionͭ�� ���ι����� */
			}
			else
			{
				if(lpKeyName == NULL_C)
					checkEnd = GetModelInfoSize(size,record,buf);	/* ��ǥ����Υ��������֤� */
				else
					checkEnd = GetModelInfoKeyValueSize(lpKeyName,size,buf);	/* Key���ͤΥ��������֤� */

				if(checkEnd == END )	break;
			}
		}
	}

//	fclose(rfile);										/* �ե�������Ĥ��� */

	if(*size != 0)		result = TRUE;				/* �����������Ǥʤ���� */
	return result;
}



/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: GetModelInfoSize
;	��ǽ����		: ��ǥ����Υ��������֤�
;	����			: �Хåե�������,�쥳���ɿ�,������ʸ����
;	�����			: �������
;	������			: 2003.08.06
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int GetModelInfoSize(int *size,int *record,char *buf)
{
	int		res;
	int		length;

	res = NOEND;
	if(*buf == '[' )							/* ����SectionName�ˤʤ�Τǽ�λ */
	{
		res = END;
	}
	else
	{
		if((length = strlen(buf)) != 0)			/* Ĺ�������Ǥʤ� */
		{
			if(*buf != LF)						/* ���Ԥ����Ǥʤ���� */
			{
				*size += length;				/* Ĺ��-1(���Ԥ����Ĺ��)��û� */
				(*record)++;					/* �쥳���ɿ��û� */
			}
		}
	}
	return res;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: GetModelInfoKeyValueSize
;	��ǽ����		: Key���ͤΥ��������֤�
;	����			: �Хåե�������,������ʸ����
;	�����			: �������
;	������			: 2003.08.06
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int GetModelInfoKeyValueSize(LPCTSTR lpKeyName,int *size,char *buf)
{
	int		res;
	int		i;
	int		lp_char;
	int		f_char;
	char	*keyNameEnd;

	res = NOFIND;
	if(*buf == '[' )								/* ����SectionName�ˤʤ�Τǽ�λ */
	{
		res = FIND;
	}
	else
	{
		keyNameEnd = strchr(buf,'=');
		if(keyNameEnd != NULL)
		{
			*keyNameEnd = NULL_C;					/*  '='��NULL�� */
			for(i=0;i<BUF_SIZE; i++)
			{
				f_char  = tolower(*(buf+i));		/* ��ʸ�����Ѵ� */
				lp_char = tolower(*(lpKeyName+i));	/* ��ʸ�����Ѵ� */
				if(f_char != lp_char)
				{
					break;
				}
				else if(*(buf+i)== NULL_C)
				{
					/* KeyName�Ǥ��� */
					*size = strlen(keyNameEnd+1);
					res = FIND;
				}
			}
		}
	}
	return res;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: ReadModelInfo
;	��ǽ����		: �������ե����뤫���ǥ��������
;	����			: �������������̾,�Хåե�������,�쥳���ɿ�,������ե�����̾
;	�����			: �������
;	������			: 2003.08.06
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/
int ReadModelInfo(LPCTSTR lpAppName, LPTSTR lpReturnedString,int nSize, LPCTSTR lpFileName)
{
	int		result;
	int		count;
	int		sectionFind;
	int		checkEnd;
	FILE	*rfile;
	char	buf[BUF_SIZE];
	int		state;

	state = 0;
	count = 0;											/* lpAppName,lpKeyName��NULL�ξ��˻��� */
	result = FALSE;
	state = 0;
	if(NULL != (rfile = fopen(lpFileName, "r")))		/* �ɤ߹��ߥե����뤬������ */
	{
		while(1)
		{
			if(feof(rfile))
			{
				break;
			}
			else
			{
				if(NULL ==fgets(buf,BUF_SIZE,rfile))					/* ����ɤ߹��� */
				{
					*(lpReturnedString + count) = NULL_C;				/* �Ǹ��ʸ����NULL */
					if(count != 0)	count--;							/* �Ǹ��NULL�Ͽ����ʤ� */
					break;
				}
				if(state == SECTION_CHECK)
				{
					sectionFind = SectionNameCheck(lpAppName,buf);		/* SectionName��Ĵ�٤� */
					if(sectionFind == FIND )	state++;				/* ����Sectionͭ�� ���ι����� */
				}
				else
				{
					checkEnd = AllReadModelInfo(lpReturnedString, nSize, buf,&count);	/* KeyName�ꥹ�Ȥ��֤� */
					if(feof(rfile) || checkEnd == END)					/* ��λ���� */
					{
						*(lpReturnedString + count) = NULL_C;			/* �Ǹ��ʸ����NULL */
						if(count != 0)	count--;						/* �Ǹ��NULL�Ͽ����ʤ� */
						break;
					}
				}
			}
		}
//		fclose(rfile);				/* �ե�������Ĥ��� */
	}
	if(count != 0)	result = TRUE;	/* �����������Ǥʤ���� */
	return result;
}


/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: AllReadModelInfo
;	��ǽ����		: ��ǥ������������
;	����			: �Хåե�������,�쥳���ɿ�,������ʸ����
;	�����			: �������
;	������			: 2003.08.06
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int AllReadModelInfo(LPTSTR lpReturnedString, int nSize, char *buf,int *count)
{
	int		res;
	char	*movePoint;
	int		length;

	res = NOEND;
	if(*buf == '[' )										/* ����SectionName�ˤʤ�Τǽ�λ */
	{
		res = END;
	}
	else
	{
		if(*buf != LF)										/* ���Ԥ����Ǥʤ��ʤ� */
		{
			if((*count) + strlen(buf) > (WORD)(nSize-1))	/* �Ǹ��ʸ����NULL�ʤΤǳ�Ǽ�Ǥ������nSize-1 */
			{
				*(buf+(nSize-1)-(*count)) = NULL_C;			/* ��Ǽ�Ǥ����ϰϤθ��NULL */
				res = END;
			}
			movePoint =(lpReturnedString+*count);			/* ��Ǽ�γ��ϰ��� */
			length = strlen(buf);
			*count += strlen(buf);							/* ����ʸ���������ʶ��ڤ��NULL�������� */
			if(*(buf+length-1)== LF)						/* ���Ԥʤ�NULL�� */
				*(buf+length-1) = NULL_C;
			strcpy(movePoint,buf);							/* �ǡ������Ǽ */
		}
	}
	return res;
}





/*
/////////////////////////////////////////////////
////////       get_Suport_inf�ؿ�        ////////
/////////////////////////////////////////////////
*/

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: get_scanmode_string
;	��ǽ����		: ���ݡ���ScanMode�Υ�٥�����
;	����			: ���ݡ���ScanMode��¤��,��٥��Ǽ����ݥ���
;	�����			: ���ݡ��Ȥ�����ܿ�(0:���顼)
;	������			: 2003.08.18
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/
int get_scanmode_string(SCANMODELIST scanMode, const char **scanModeList)
{
	int i;
	int count=0;

	for(i=0;i<COLOR_TYPE_COUNT+1;i++)
	{
		scanModeList[i] = NULL;					/* ��Ǽ����ν���� */
	}

	if(scanMode.bit.bBlackWhite == TRUE)				/* Black&White���б�? */
	{
		scanModeList[count] = bwString;
		count++;
	}
	if(scanMode.bit.bErrorDiffusion == TRUE)			/* Gray(ErrorDiffusion)���б�? */
	{
		scanModeList[count] = errDiffusionString;
		count++;
	}
	if(scanMode.bit.bTrueGray == TRUE)					/* TrueGray���б�? */
	{
		scanModeList[count] = tGrayString;
		count++;
	}
	if(scanMode.bit.b24BitColor == TRUE)				/* 24bitColor���б�? */
	{
		scanModeList[count] = ColorString;
		count++;
	}
	if(scanMode.bit.b24BitNoCMatch == TRUE)		    	/* 24bitColorFast���б�? */
	{
		scanModeList[count] = ColorFastString;
		count++;
	}
	return count;


}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: get_reso_int
;	��ǽ����		: ���ݡ��Ȳ����٤Υ�٥�����
;	����			: ���ݡ��Ȳ����ٹ�¤��,��٥��Ǽ����ݥ���
;	�����			: ���ݡ��Ȥ�����ܿ�(0:���顼)
;	������			: 2003.08.18
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/
int get_reso_int(RESOLIST reso, int *resoList)
{
	int i;
	int count;

	for(i=0;i<RESO_COUNT+1;i++)
	{
		resoList[i] = 0;					/* ��Ǽ����ν���� */
	}

	count=1;
	if(reso.bit.bDpi100x100 == TRUE)				/* Dpi100x100���б�? */
	{
		resoList[count] = DPI100x100;
		count++;
	}
	if(reso.bit.bDpi150x150 == TRUE)				/* Dpi150x150���б�? */
	{
		resoList[count] = DPI150x150;
		count++;
	}
	if(reso.bit.bDpi200x200 == TRUE)				/* Dpi200x200���б�? */
	{
		resoList[count] = DPI200x200;
		count++;
	}
	if(reso.bit.bDpi300x300 == TRUE)				/* Dpi300x300���б�? */
	{
		resoList[count] = DPI300x300;
		count++;
	}
	if(reso.bit.bDpi400x400 == TRUE)				/* Dpi400x400���б�? */
	{
		resoList[count] = DPI400x400;
		count++;
	}
	if(reso.bit.bDpi600x600 == TRUE)				/* Dpi600x600���б�? */
	{
		resoList[count] = DPI600x600;
		count++;
	}
	if(reso.bit.bDpi1200x1200 == TRUE)				/* Dpi1200x1200���б�? */
	{
		resoList[count] = DPI1200x1200;
		count++;
	}
	if(reso.bit.bDpi2400x2400 == TRUE)				/* Dpi2400x2400���б�? */
	{
		resoList[count] = DPI2400x2400;
		count++;
	}
	if(reso.bit.bDpi4800x4800 == TRUE)				/* Dpi4800x4800���б�? */
	{
		resoList[count] = DPI4800x4800;
		count++;
	}
	if(reso.bit.bDpi9600x9600 == TRUE)				/* Dpi9600x9600���б�? */
	{
		resoList[count] = DPI9600x9600;
		count++;
	}
	resoList[0] = count-1;	/* ���ܿ�����Ƭ�˥��åȤ��롣 */
	
	return count;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: get_scansrc_string
;	��ǽ����		: ���ݡ���ScanSrc��¤�Τ�ʸ��������
;	����			: ���ݡ���ScanSrc��¤��,��٥��Ǽ����ݥ���
;	�����			: ���ݡ��Ȥ�����ܿ�(0:���顼)
;	������			: 2003.08.18
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/
int get_scansrc_string(SCANSRCLIST scanSrc, const char **scanSrcList)
{
	int i;
	int count=0;

	for(i=0;i<SCAN_SRC_COUNT+1;i++)
	{
		scanSrcList[i] = NULL;				/* ��Ǽ����ν���� */
	}

	if(scanSrc.bit.FB == TRUE)						/* FB���б�? */
	{
		scanSrcList[count] = fbString;
		count++;
	}
	if(scanSrc.bit.ADF == TRUE)						/* ADF���б�? */
	{
		scanSrcList[count] = adfString;
		count++;
	}
	return count;
}


/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: get_scanmode_id
;	��ǽ����		: ʸ���󤫤���פ��륹�����⡼�ɤ�ID�ֹ���������
;	����			: ScanMode��ʸ����
;	�����			: ScanMode��ID(-1:���顼)
;	������			: 2003.08.19
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int get_scanmode_id(const char *scanmode)
{
	int res;

	if( 0 ==strcmp(scanmode,bwString))				/* Black&White? */
		res = COLOR_BW;
	else if( 0 == strcmp(scanmode, errDiffusionString))		/* Gray(ErrorDiffusion)? */
		res = COLOR_ED;
	else if( 0 == strcmp(scanmode, tGrayString))			/* TrueGray? */
		res = COLOR_TG;
	else if( 0 == strcmp(scanmode, ColorString))				/* 24bitColor? */
		res = COLOR_FUL;
	else if( 0 == strcmp(scanmode, ColorFastString))				/* 24bitColorFast? */
		res = COLOR_FUL_NOCM;
	else
		res = -1;		/* ���������Τ��ʤ���Х��顼 */

	return res;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: get_reso_id
;	��ǽ����		: ���ݡ��Ȳ����٤�����פ���ID�ֹ���������
;	����			: ���ݡ��Ȳ����٤�ʸ����
;	�����			: ���ݡ��Ȳ����٤�ID(-1:���顼)
;	������			: 2003.08.19
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int get_reso_id(const int reso)
{
	int res;

	if( reso == DPI100x100)				/* 100x100? */
		res = RES100X100;
	else if( reso == DPI150x150)		/* 150x150? */
		res = RES150X150;
	else if( reso == DPI200x200)		/* 200x200? */
		res = RES200X200;
	else if( reso == DPI300x300)		/* 300x300? */
		res = RES300X300;
	else if( reso == DPI400x400)		/* 400x400? */
		res = RES400X400;
	else if( reso == DPI600x600)		/* 600x600? */
		res = RES600X600;
	else if( reso == DPI1200x1200)		/* 1200x1200? */
		res = RES1200X1200;
	else if( reso == DPI2400x2400)		/* 2400x2400? */
		res = RES2400X2400;
	else if( reso == DPI4800x4800)		/* 4800x4800? */
		res = RES4800X4800;
	else if( reso == DPI9600x9600)		/* 9600x9600? */
		res = RES9600X9600;
	else
		res = -1;				/* ���������Τ��ʤ���Х��顼 */

	return res;
}


/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: get_scanmode_id
;	��ǽ����		: ʸ���󤫤���פ��륹�����⡼�ɤ�ID�ֹ���������
;	����			: ScanSrc��ʸ����
;	�����			: ScanSrc��ID(-1:���顼)
;	������			: 2003.08.19
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int get_scansrc_id(const char *scanSrc)
{
	int res;

	if( 0 ==strcmp(scanSrc,fbString))				/* FlatBed? */
		res = SCANSRC_FB;
	else if( 0 == strcmp(scanSrc, adfString))		/* Auto Document Feeder? */
		res = SCANSRC_ADF;
	else
		res = -1;		/* ���������Τ��ʤ���Х��顼 */

	return res;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: get_scanmode_listcnt
;	��ǽ����		: ScanModeʸ����ꥹ�Ȥ��ֹ�μ���
;	����			: ScanModeʸ����ꥹ�ȤΥݥ���,��������ScanModeID
;	�����			: ��������ScanModeʸ����ꥹ�Ȥ��ֹ�(-1:���顼)
;	������			: 2003.08.20
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int get_scanmode_listcnt(const char **scanModeList, int scanModeID)
{
	char IDString[MAX_STRING];
	int  count = 0;
	int  res;

	res = ScanModeIDString(scanModeID, IDString);
	if(res == TRUE)
	{
		while(1)
		{
			if(!scanModeList[count])
			{
				count = -1;
				break;
			}
			else if( 0 == strcmp(scanModeList[count],IDString))
			{
				break;
			}
			count++;
		}
	}
	else
	{
		count = -1;
	}
	return count;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: ScanModeIDString
;	��ǽ����		: ScanModeID���б�����ʸ����μ���
;	����			: ScanModeID,��Ǽ��ʸ����ݥ���
;	�����			: ����:TRUE , ����:FALSE
;	������			: 2003.08.20
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int ScanModeIDString(int scanModeID, char *IDString)
{
	int res;

	res = TRUE;
	switch(scanModeID)
	{
		case	COLOR_BW:
			strcpy(IDString, bwString);
			break;
		case	COLOR_ED:
			strcpy(IDString, errDiffusionString);
			break;
		case	COLOR_TG:
			strcpy(IDString, tGrayString);
			break;
		case	COLOR_FUL:
			strcpy(IDString, ColorString);
			break;
		case	COLOR_FUL_NOCM:
			strcpy(IDString, ColorFastString);
			break;
		default:
			res = FALSE;
	}
	return res;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: get_reso_listcnt
;	��ǽ����		: �����ٿ��ͥꥹ�Ȥ������ֹ�μ���
;	����			: �����ٿ��ͥꥹ�ȤΥݥ���,��������resoID
;	�����			: ������������ٿ��ͥꥹ�Ȥ��ֹ�(-1:���顼)
;	������			: 2003.08.20
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int get_reso_listcnt(int *resoList, int resoID)
{
	int  IDInt;
	int  count = 1;
	int  res;

	res = ResoIDInt(resoID, &IDInt);
	if(res == TRUE)
	{
		while( count <= resoList[0])
		{
			if(!resoList[count])
			{
				count = -1;
				break;
			}
			else if( resoList[count] == IDInt)
			{
				break;
			}
			count++;
		}
	}
	else
	{
		count = -1;
	}
	return count;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: ResoIDInt
;	��ǽ����		: resoID���б�������ͤμ���
;	����			: resoID,���ͤγ�Ǽ��
;	�����			: ����:TRUE , ����:FALSE
;	������			: 2003.08.20
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int ResoIDInt(int resoID, int *IDInt)
{
	int res;

	res = TRUE;
	switch(resoID)
	{
		case	RES100X100:
			*IDInt = DPI100x100;
			break;
		case	RES150X150:
			*IDInt = DPI150x150;
			break;
		case	RES200X200:
			*IDInt = DPI200x200;
			break;
		case	RES300X300:
			*IDInt = DPI300x300;
			break;
		case	RES400X400:
			*IDInt = DPI400x400;
			break;
		case	RES600X600:
			*IDInt = DPI600x600;
			break;
		case	RES1200X1200:
			*IDInt = DPI1200x1200;
			break;
		case	RES2400X2400:
			*IDInt = DPI2400x2400;
			break;
		case	RES4800X4800:
			*IDInt = DPI4800x4800;
			break;
		case	RES9600X9600:
			*IDInt = DPI9600x9600;
			break;
		default:
			res = FALSE;
	}
	return res;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: get_scansrc_listcnt
;	��ǽ����		: ScanSrcʸ����ꥹ�Ȥ��ֹ�μ���
;	����			: ScanSrcʸ����ꥹ�ȤΥݥ���,��������scanSrcID
;	�����			: ��������ScanSrcʸ����ꥹ�Ȥ��ֹ�(-1:���顼)
;	������			: 2003.08.20
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int get_scansrc_listcnt(const char **scanSrcList, int scanSrcID)
{
	char IDString[MAX_STRING];
	int  count = 0;
	int  res;

	res = ScanSrcIDString(scanSrcID, IDString);
	if(res == TRUE)
	{
		while(1)
		{
			if(!scanSrcList[count])
			{
				count = -1;
				break;
			}
			else if( 0 == strcmp(scanSrcList[count],IDString))
			{
				break;
			}
			count++;
		}
	}
	else
	{
		count = -1;
	}
	return count;
}

/*
;------------------------------------------------------------------------------
;	�⥸�塼��̾	: ScanSrcIDString
;	��ǽ����		: scanSrcID���б�����ʸ����μ���
;	����			: scanSrcID,��Ǽ��ʸ����ݥ���
;	�����			: ����:TRUE , ����:FALSE
;	������			: 2003.08.20
;	�õ�����		:
;------------------------------------------------------------------------------
;	�ѹ�����
;	����		������	������
;------------------------------------------------------------------------------
*/

int ScanSrcIDString(int scanSrcID, char *IDString)
{
	int res;

	res = TRUE;
	switch(scanSrcID)
	{
		case	SCANSRC_FB:
			strcpy(IDString, fbString);
			break;
		case	SCANSRC_ADF:
			strcpy(IDString, adfString);
			break;
		default:
			res = FALSE;
	}
	return res;
}
