#ifndef _TW_TYPEDEF__
#define _TW_TYPEDEF__

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <algorithm>
#include <vector>
#include <iterator>
#include <stdio.h>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

#define F_MIN(a,b) ((a)>(b)?(b):(a))
#define F_MAX(a,b) ((a)>(b)?(a):(b))

#define T2X_SYSTEM    'T'
#define GK_SYSTEM	 'G'

#ifndef USING_MODES
#define USING_MODES T2X_SYSTEM
#endif

#if USING_MODES!=GK_SYSTEM && USING_MODES!=T2X_SYSTEM
#error "USING_MODES must is GK_SYSTEM or T2X_SYSTEM"
#endif


#if USING_MODES == T2X_SYSTEM
#define KEYPOINTNUMS 50 //(50<=KEYPOINTNUMS<=200)
#define DOMEKEYPOINTNUMS 200 //(200<=DOMEKEYPOINTNUMS<=500)
#define VERSION_TYPE "T2X_0"
#elif USING_MODES== GK_SYSTEM
#define KEYPOINTNUMS 100 //(50<=KEYPOINTNUMS<=200)
#define DOMEKEYPOINTNUMS 300 //(200<=DOMEKEYPOINTNUMS<=500)
#define VERSION_TYPE "GK_0"
#endif


#define PICWRITE 0 //PICWRITE==1 表示记录拍摄的图像，PICWRITE！=1，表示不记录拍摄的图像 
#define PRINTOUT 1
#define SAMEPIC_FIND "samepic_finded"
#define SAMEPIC_NOTFIND "samepic_notfind"


typedef unsigned char uchar;
typedef unsigned short ushort;

typedef struct _KEYPOINT
{
	float x;
	float y;
	float val;//角点的强度
}KEYPOINT;
typedef struct _KEYPOINT * pKEYPOINT;


typedef struct _POINYXY
{
	float x;
	float y;
}POINYXY;
typedef struct _POINYXY  Point2f;


typedef struct _DESCRIP 
{
	unsigned char descrip[32];
}DESCRIP;
typedef struct _DESCRIP * pDESCRIP;

typedef struct _TEMPLATEDES 
{
	
	int pages_key_nums;
	char pages_srial_number[32];//书页的序列号
	void* descrip;//pages descripe DESCRIP
	void* ponit;//pages keypoint POINYXY
}TEMPLATEDES;
typedef struct _TEMPLATEDES * pTEMPLATEDES;


typedef struct _BOOKDESCRIP 
{
	
	 _BOOKDESCRIP()
	 {
		memset(versionType,0,8);
		strcpy(versionType,VERSION_TYPE);
	 }
	 char versionType[8];//添加版本类型号
	 
	 unsigned int filesize;
	 int nums_desc;//书本的页数
	 char srial_number[32];//书本的序列号
	 void *pDescrip;//指向书本模板描述指针
}BOOKDESCRIP;
typedef struct _BOOKDESCRIP *pBOOKDESCRIP;


typedef struct _Imagetype
{  
    int width;  
    int height;  
    unsigned char* imageData;  
}Imagetype;  
typedef struct _Imagetype *pImagetype;


typedef struct _Matchtype
{  
	int templateIndex;  
    int srcIndex;  
	unsigned int distance;  
}Matchtype;  
typedef struct _Matchtype *pMatchtype;


struct INDENTIFOptions {
	enum
	{
		DATABASE_KEY_MODE_MAX = 1, //以最大响应值的方式获取最靠前的数据特征点
		DATABASE_KEY_MODE_MEAN = 2//以均匀分布的方式获取特征数据
	};
	enum
	{
		IMAG_TEMPLATE_W =(565*2),//一以实际处理图片的宽度的2倍作为最大值
		DEST_TEMPLATE_H =(346*2)//一以实际处理图片的高度的2倍作为最大值
	};
    INDENTIFOptions()
        : omax(1)
        , nsublevels(1)
        , img_max_width(IMAG_TEMPLATE_W)
        , img_max_height(DEST_TEMPLATE_H)
        , dthreshold(0.0007f)//[0.005f,0.1f]
		, keymode(DATABASE_KEY_MODE_MEAN)
		, keysize(5)//[5,15],一个特征点的面积10*10
		, teamplatekeyPointNums(200)//[200,500]，模板数据库的特征点的数量
		, srckeyPointNums(50)//[50,200]，查找文件特征点的数量
		, indentCNNums(8)//[8 15] 要求能匹配到的特征点数量
        , dmatchratio(0.8f)//[0.5,0.85] 比值
		
    {
    }

    int omax;                       
    int nsublevels;     
    int img_max_width;                  
    int img_max_height; 
	float dthreshold;             
    int keymode;                  
    int keysize;       
    int teamplatekeyPointNums;             
    int srckeyPointNums;  
    int indentCNNums;	
    float dmatchratio;          

};

#ifdef __cplusplus
}
#endif

#endif
