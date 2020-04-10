#include "identifyPictures.h"
#include "huiben.h"

#if USING_MODES == T2X_SYSTEM
#include "cmptkeypoint.h"
#include "computeDescriptors.h"
#include "gausss.h"
#include "tpyedef_orbfast.h"
#endif

#if PICWRITE==1
#include "mkdir_io.h"
#include "c_bmp.h"
#endif

#pragma warning(disable:4996)
using namespace std;

static INDENTIFOptions identifyoptions;
static char retStr[32+1]={0};//sizeof(BOOKDESCRIP::srial_number)
static bool SortByM4(const Matchtype &v1, const Matchtype &v2)//注意：本函数的参数的类型一定要与vector中元素的类型一致  
{
	return v1.distance < v2.distance;//升序排列  
} 

void *getPagesDescribe(Imagetype* pGrayimage, int *cont)
{

	if (pGrayimage == NULL) return NULL;
	if (pGrayimage->height <= 0 || pGrayimage->width <= 0) return NULL;
	unsigned char* preturnData = NULL;
	int retcn=0,i;
	int tempkeysize ;
	unsigned int addjiaoyan=0;
	KEYPOINT *keypoints_dest = new KEYPOINT[identifyoptions.srckeyPointNums];
	DESCRIP *dstDesc = new DESCRIP[identifyoptions.srckeyPointNums];
	if (keypoints_dest == NULL || dstDesc == NULL ) 
	{
		if (keypoints_dest != NULL) delete[] keypoints_dest;
		if (dstDesc != NULL) delete[] dstDesc;
		return NULL;
	}

	tempkeysize = detectAndCompute(pGrayimage, keypoints_dest, dstDesc, identifyoptions.srckeyPointNums);
	if (tempkeysize > 10)
	{
		retcn = sizeof(int) + 32 * sizeof(char) + tempkeysize * sizeof(DESCRIP) + tempkeysize * sizeof(POINYXY)+sizeof(addjiaoyan);
		preturnData= (unsigned char *)malloc(retcn);
		if (preturnData != NULL)
		{
			pTEMPLATEDES temp = (pTEMPLATEDES)preturnData;
			
			memset(preturnData, 0,retcn);
			
			temp->pages_key_nums = tempkeysize;
			memset(temp->pages_srial_number,0,32);
			strcpy(temp->pages_srial_number, VERSION_TYPE);

			memcpy(preturnData + sizeof(int) + 32 * sizeof(char), dstDesc, tempkeysize * sizeof(DESCRIP));
			POINYXY * tep = (POINYXY *)(preturnData + sizeof(int) + 32 * sizeof(char)+tempkeysize * sizeof(DESCRIP));
			for (int i = 0; i < tempkeysize; i++)
			{
				tep[i].x = keypoints_dest[i].x;
				tep[i].y = keypoints_dest[i].y;
			}

		}
		for(i=0;i<retcn-4;i++)
		{
			addjiaoyan +=preturnData[i];
		}
		preturnData[i++]=(addjiaoyan)&0xff;
		preturnData[i++]=(addjiaoyan>>8)&0xff;
		preturnData[i++]=(addjiaoyan>>16)&0xff;
		preturnData[i++]=(addjiaoyan>>24)&0xff;
	}
	if (keypoints_dest != NULL) delete[] keypoints_dest;
	if (dstDesc != NULL) delete[] dstDesc;
	*cont = retcn;
	return preturnData;

}

int initIdentify(INDENTIFOptions *options)
{
	if(options->indentCNNums>=8&&options->indentCNNums<=15)
	{
		identifyoptions.indentCNNums = options->indentCNNums;
	}
	else
	{
		return -1;
	}
	if(options->srckeyPointNums>=KEYPOINTNUMS&&options->indentCNNums<=DOMEKEYPOINTNUMS)
	{
		identifyoptions.srckeyPointNums = options->srckeyPointNums;
	}
	else
	{
		return -1;
	}
	if(options->dmatchratio>0.5f&&options->dmatchratio<0.85f)
	{
		identifyoptions.dmatchratio = options->dmatchratio;
	}
	else
	{
		return -1;
	}
	if(options->teamplatekeyPointNums<500&&options->teamplatekeyPointNums>=DOMEKEYPOINTNUMS)
	{
		identifyoptions.teamplatekeyPointNums = options->teamplatekeyPointNums;
	}
	else
	{
		return -1;
	}
	return 1;
}

 //读二进制文件
int readDesripFromfile(string str, pBOOKDESCRIP pBooksDesc)
{
	FILE *fid;
	fid = fopen(str.c_str(), "rb");
	if (fid == NULL)
	{
		printf("read file failed!\n");
		return 0;
	}
	//获取文件大小
	long tempSize;
	fseek(fid, 0, SEEK_END);
	long lSize = ftell(fid);
	rewind(fid);
	fread(&pBooksDesc->versionType, sizeof(pBooksDesc->versionType)+sizeof(pBooksDesc->filesize)+sizeof(pBooksDesc->nums_desc)+sizeof(pBooksDesc->srial_number), 1, fid);
	tempSize = pBooksDesc->filesize;
	if(strcmp(pBooksDesc->versionType,VERSION_TYPE)!=0)
	{
		printf("Library version mismatch:%s\n",pBooksDesc->versionType);
		return 0;
	}
	if (lSize != tempSize + sizeof(pBooksDesc->versionType)+sizeof(pBooksDesc->filesize)+sizeof(pBooksDesc->nums_desc)+sizeof(pBooksDesc->srial_number))
	{
		printf("file is damage off!\n");
		return 0;
	}
	printf("versionType=%s,lSize=%d,tempSize=%d\n", pBooksDesc->versionType,lSize, tempSize);
	//开辟存储空间
	pBooksDesc->pDescrip = malloc(tempSize);
	if (pBooksDesc->pDescrip == NULL)
	{
		printf("malloc memory spach failed!\n");

		return 0;
	}
	fread(pBooksDesc->pDescrip, sizeof(char), tempSize, fid);
	fclose(fid);
	return lSize;
}

const char *identifybooks(Imagetype* pGrayimag, pBOOKDESCRIP pBOOKDescribe, pBOOKDESCRIP pCONTENTESDescribe)
{
	static int firstSearchNums = 0;
	static int pagesflag = -1;
	static int flag0 = 0;
	static int runnums = 0;

	if (pGrayimag == NULL) return NULL;
	if (pGrayimag->height <= 0 || pGrayimag->width <= 0) return NULL;
	const char * preturnData = NULL;
	int i, j;
	int pagesKeyNums;
	int tempkeysize;
	int cn = 0;
	POINYXY *pScr = new POINYXY[identifyoptions.srckeyPointNums];
	POINYXY *pDest = new POINYXY[identifyoptions.srckeyPointNums];
	KEYPOINT *keypoints_dest = new KEYPOINT[identifyoptions.srckeyPointNums];
	DESCRIP *dstDesc = new DESCRIP[identifyoptions.srckeyPointNums];
	Matchtype *matchval = new Matchtype[identifyoptions.srckeyPointNums];

	if (pScr == NULL || pDest == NULL|| keypoints_dest == NULL || dstDesc == NULL || matchval == NULL) 
	{
		if (pScr != NULL) delete[] pScr;
		if (pDest != NULL) delete[] pDest;
		if (keypoints_dest != NULL) delete[] keypoints_dest;
		if (dstDesc != NULL) delete[] dstDesc;
		if (matchval != NULL)delete[] matchval;
		return NULL;
	}
	

	flag0 = detetcPIC_three(pGrayimag);
	if(flag0==1&&pagesflag==1)
	{
		if (pScr != NULL) delete[] pScr;
		if (pDest != NULL) delete[] pDest;
		if (keypoints_dest != NULL) delete[] keypoints_dest;
		if (dstDesc != NULL) delete[] dstDesc;
		if (matchval != NULL)delete[] matchval;
		return SAMEPIC_FIND;
	}
#if PRINTOUT==1
	printf("detetcPIC_three flag0 =%d pagesflag=%d\n", flag0, pagesflag);
#endif

#if PICWRITE==1
		static int picont = 0;
		char picontstr[256];
		ClImage img_2;
		CreateDir(SD_PIC_DIR);
		img_2.channels = 1;
		img_2.height = pGrayimag->height;
		img_2.width = pGrayimag->width;
		img_2.imageData = pGrayimag->imageData;
		sprintf(picontstr, "%sb_%d.bmp",SD_PIC_DIR,picont);
		clSaveImage(picontstr, &img_2);
		picont++;
#if PRINTOUT==1
		printf("picontstr = %s\n", picontstr);
#endif
#endif

	tempkeysize = detectAndCompute(pGrayimag, keypoints_dest, dstDesc, identifyoptions.srckeyPointNums);
	if (pBOOKDescribe->pDescrip != NULL)
	{
		pTEMPLATEDES ptempDesc = (pTEMPLATEDES)pBOOKDescribe->pDescrip;
		int pages = pBOOKDescribe->nums_desc;
		DESCRIP **IndixtempsDesc = new DESCRIP*[pages];
		POINYXY **IndixtempsKeyc = new POINYXY*[pages];
		int *pkeyNums = new int[pages];
		char *p = (char *)ptempDesc;
		if (IndixtempsDesc == NULL || IndixtempsKeyc == NULL || pkeyNums == NULL) 
		{
			if (IndixtempsDesc != NULL) delete IndixtempsDesc;
			if (IndixtempsKeyc != NULL) delete IndixtempsKeyc;
			if (pkeyNums != NULL) delete pkeyNums;
			goto retn;
		}
		
		for (i = 0; i < pages; i++)
		{
			pagesKeyNums = ptempDesc->pages_key_nums;
			pkeyNums[i] = pagesKeyNums;
			p += sizeof(ptempDesc->pages_key_nums) + sizeof(ptempDesc->pages_srial_number);//36
			IndixtempsDesc[i] = (DESCRIP *)p;
			p += pagesKeyNums * sizeof(DESCRIP);
			IndixtempsKeyc[i] = (POINYXY *)p;
			p += pagesKeyNums * sizeof(POINYXY);
			ptempDesc = (pTEMPLATEDES)p;

		}
		for (i = firstSearchNums; i < pages; i++)
		{
			cn = hanmmingmatch(IndixtempsDesc[i], pkeyNums[i], dstDesc, tempkeysize, matchval, identifyoptions.dmatchratio);
#if PRINTOUT==1
			printf("book:In pages %d find Similar characteristics is %d;\n", i+1, cn);
#endif
			if (cn >= identifyoptions.indentCNNums)
			{

				std::sort(matchval, matchval + cn - 1, SortByM4);
				for (j = 0; j < identifyoptions.indentCNNums; j++)
				{
					pDest[j].x = keypoints_dest[matchval[j].srcIndex].x;
					pDest[j].y = keypoints_dest[matchval[j].srcIndex].y;
					pScr[j].x = IndixtempsKeyc[i][matchval[j].templateIndex].x;
					pScr[j].y = IndixtempsKeyc[i][matchval[j].templateIndex].y;

				}
				if (cmpKeypoint(pDest, pScr, identifyoptions.indentCNNums) == 1)
				{
					memset(retStr, 0, sizeof(pBOOKDescribe->srial_number) + 1);
					memcpy(retStr, ((char *)IndixtempsDesc[i]) - sizeof(pBOOKDescribe->srial_number), sizeof(pBOOKDescribe->srial_number));
					preturnData = retStr;
					firstSearchNums = i;
					runnums = 0;
					if (IndixtempsDesc != NULL) delete IndixtempsDesc;
					if (IndixtempsKeyc != NULL) delete IndixtempsKeyc;
					if (pkeyNums != NULL) delete pkeyNums;
					pagesflag = 1;
					goto retn;
				}
			}
		}
		for (i = 0; i < firstSearchNums; i++)
		{
			cn = hanmmingmatch(IndixtempsDesc[i], pkeyNums[i], dstDesc, tempkeysize, matchval, identifyoptions.dmatchratio);
#if PRINTOUT==1
			printf("book:In pages %d find Similar characteristics is %d;\n", i+1, cn);
#endif
			if (cn >= identifyoptions.indentCNNums)
			{
				std::sort(matchval, matchval + cn - 1, SortByM4);
				for (j = 0; j < identifyoptions.indentCNNums; j++)
				{
					pDest[j].x = keypoints_dest[matchval[j].srcIndex].x;
					pDest[j].y = keypoints_dest[matchval[j].srcIndex].y;
					pScr[j].x = IndixtempsKeyc[i][matchval[j].templateIndex].x;
					pScr[j].y = IndixtempsKeyc[i][matchval[j].templateIndex].y;

				}
				if (cmpKeypoint(pDest, pScr, identifyoptions.indentCNNums) == 1)
				{
					memset(retStr, 0, sizeof(pBOOKDescribe->srial_number) + 1);
					memcpy(retStr, ((char *)IndixtempsDesc[i]) - sizeof(pBOOKDescribe->srial_number), sizeof(pBOOKDescribe->srial_number));
					preturnData = retStr;
					firstSearchNums = i;
					runnums = 0;
					if (IndixtempsDesc != NULL) delete[] IndixtempsDesc;
					if (IndixtempsKeyc != NULL) delete[] IndixtempsKeyc;
					if (pkeyNums != NULL) delete[] pkeyNums;
					pagesflag = 1;
					goto retn;
				}
			}
		}
		runnums++;
		if (IndixtempsDesc != NULL) delete[] IndixtempsDesc;
		if (IndixtempsKeyc != NULL) delete[] IndixtempsKeyc;
		if (pkeyNums != NULL) delete[] pkeyNums;
	}
	if (pCONTENTESDescribe->pDescrip != NULL && ((runnums > 3) || (pBOOKDescribe->pDescrip == NULL)))
	{
		pTEMPLATEDES ptempDesc = (pTEMPLATEDES)pCONTENTESDescribe->pDescrip;
		int pages = pCONTENTESDescribe->nums_desc;
		DESCRIP **IndixtempsDesc = new DESCRIP*[pages];
		POINYXY **IndixtempsKeyc = new POINYXY*[pages];
		int *pkeyNums = new int[pages];
		char *p = (char *)ptempDesc;
		if (IndixtempsDesc == NULL || IndixtempsKeyc == NULL || pkeyNums == NULL) 
		{
			if (IndixtempsDesc != NULL) delete IndixtempsDesc;
			if (IndixtempsKeyc != NULL) delete IndixtempsKeyc;
			if (pkeyNums != NULL) delete pkeyNums;
			goto retn;
		}
		
		for (i = 0; i < pages; i++)
		{
			pagesKeyNums = ptempDesc->pages_key_nums;
			pkeyNums[i] = pagesKeyNums;
			p += sizeof(ptempDesc->pages_key_nums) + sizeof(ptempDesc->pages_srial_number);//36
			IndixtempsDesc[i] = (DESCRIP *)p;
			p += pagesKeyNums * sizeof(DESCRIP);
			IndixtempsKeyc[i] = (POINYXY *)p;
			p += pagesKeyNums * sizeof(POINYXY);
			ptempDesc = (pTEMPLATEDES)p;

		}
		for (i = 0; i < pages; i++)
		{
			cn = hanmmingmatch(IndixtempsDesc[i], pkeyNums[i], dstDesc, tempkeysize, matchval, identifyoptions.dmatchratio);
#if PRINTOUT==1
			printf("contentes:In pages %d find Similar characteristics is %d;\n", i+1, cn);
#endif
			if (cn >= identifyoptions.indentCNNums)
			{
				std::sort(matchval, matchval + cn, SortByM4);
				for (j = 0; j < identifyoptions.indentCNNums; j++)
				{
					pDest[j].x = keypoints_dest[matchval[j].srcIndex].x;
					pDest[j].y = keypoints_dest[matchval[j].srcIndex].y;
					pScr[j].x = IndixtempsKeyc[i][matchval[j].templateIndex].x;
					pScr[j].y = IndixtempsKeyc[i][matchval[j].templateIndex].y;

				}
				if (cmpKeypoint(pDest, pScr, identifyoptions.indentCNNums) == 1)
				{
					memset(retStr, 0, sizeof(pCONTENTESDescribe->srial_number) + 1);
					memcpy(retStr, ((char *)IndixtempsDesc[i]) - sizeof(pCONTENTESDescribe->srial_number), sizeof(pCONTENTESDescribe->srial_number));
					preturnData = retStr;
					runnums = 0;
					if (IndixtempsDesc != NULL) delete[] IndixtempsDesc;
					if (IndixtempsKeyc != NULL) delete[] IndixtempsKeyc;
					if (pkeyNums != NULL) delete[] pkeyNums;
					pagesflag = 1;
					goto retn;
				}
			}
		}
		if (IndixtempsDesc != NULL) delete[] IndixtempsDesc;
		if (IndixtempsKeyc != NULL) delete[] IndixtempsKeyc;
		if (pkeyNums != NULL) delete[] pkeyNums;
	}
	pagesflag = -1; 
retn:
	if (pScr != NULL) delete[] pScr;
	if (pDest != NULL) delete[] pDest;
	if (keypoints_dest != NULL) delete[] keypoints_dest;
	if (dstDesc != NULL) delete[] dstDesc;
	if (matchval != NULL)delete[] matchval;
	if(flag0==1&&preturnData==NULL)
	{
		return SAMEPIC_NOTFIND;
	}
	return preturnData;
 }

int upBookList(string fileBookList, string fileDownloadDesc)
 {
	BOOKDESCRIP Downloadescribe,fileout;
	FILE *infid,*outfid;
	
	TEMPLATEDES tempdesc;
	outfid = fopen(fileDownloadDesc.c_str(), "rb");
	if (outfid == NULL)
	{
		printf("upBookList:%s is not opened!\n",fileDownloadDesc);
		return 0;
	}
	//获取文件大小
	long outempSize;
	fseek(outfid, 0, SEEK_END);
	long outlSize = ftell(outfid);
	rewind(outfid);
	fread((void*)&fileout, sizeof(fileout.versionType)+sizeof(fileout.filesize)+sizeof(fileout.nums_desc)+sizeof(fileout.srial_number), 1, outfid);
	if(strcmp(fileout.versionType,VERSION_TYPE)!=0)
	{
		printf("error:upBookList:Library version mismatch:%s\n",fileout.versionType);
		fclose(outfid);
		return 0;
	}
	fread(&tempdesc,sizeof(tempdesc.pages_key_nums) + sizeof(tempdesc.pages_srial_number),1,outfid);
	memcpy(tempdesc.pages_srial_number,fileout.srial_number,32);
	outempSize = fileout.filesize;
	if (outlSize != outempSize +sizeof(fileout.versionType)+ sizeof(fileout.filesize) + sizeof(fileout.nums_desc) + sizeof(fileout.srial_number))
	{
		printf("error:upBookList:%s file is damage off!\n",fileDownloadDesc);
		fclose(outfid);
		return 0;
	}
	if(tempdesc.pages_key_nums<50 ||tempdesc.pages_key_nums>500)
	{
		printf("error:upBookList:tempdesc.pages_key_nums=%d\n",tempdesc.pages_key_nums);
		fclose(outfid);
		return 0;
	}
	tempdesc.descrip = malloc(tempdesc.pages_key_nums*(sizeof(DESCRIP)+sizeof(POINYXY)));
	if(tempdesc.descrip==NULL)
	{
		printf("error:upBookList:malloc memory spach failed!\n");
		fclose(outfid);
		return 0;
	}
	fread(tempdesc.descrip,tempdesc.pages_key_nums*(sizeof(DESCRIP)+sizeof(POINYXY)),1,outfid);
	fclose(outfid);
	/////////////////////////////////////////////////////////////
	infid = fopen(fileBookList.c_str(), "rb+");
	if (infid == NULL)
	{
		if(tempdesc.descrip!=NULL)
		{
			free(tempdesc.descrip);
			tempdesc.descrip = NULL;
		}
		printf("error: upBookList:%s is not opened!\n",fileBookList);
		return 0;
	}
	//获取文件大小
	long tempSize;
	fseek(infid, 0, SEEK_END);
	long lSize = ftell(infid);
	rewind(infid);
	fread((void *)&Downloadescribe,sizeof(Downloadescribe.versionType)+sizeof(Downloadescribe.filesize)+sizeof(Downloadescribe.nums_desc), 1, infid);
	if(strcmp(Downloadescribe.versionType,VERSION_TYPE)!=0)
	{
		if(tempdesc.descrip!=NULL)
		{
			free(tempdesc.descrip);
			tempdesc.descrip = NULL;
		}
		printf("error:upBookList:Library version mismatch:%s\n",Downloadescribe.versionType);
		fclose(infid);
		return 0;
	}
	tempSize = Downloadescribe.filesize;
	if (lSize != tempSize + sizeof(Downloadescribe.versionType)+sizeof(Downloadescribe.filesize) + sizeof(Downloadescribe.nums_desc) + sizeof(Downloadescribe.srial_number))
	{
		if(tempdesc.descrip!=NULL)
		{
			free(tempdesc.descrip);
			tempdesc.descrip = NULL;
		}
		printf("upBookList:%s file is damage off!\n",fileBookList);
		fclose(infid);
		return 0;
	}

	Downloadescribe.filesize +=sizeof(tempdesc.pages_key_nums) + sizeof(tempdesc.pages_srial_number)+tempdesc.pages_key_nums*(sizeof(DESCRIP)+sizeof(POINYXY));
	Downloadescribe.nums_desc +=1;
	fseek(infid, 0, SEEK_END);
	fwrite(&tempdesc,sizeof(tempdesc.pages_key_nums) + sizeof(tempdesc.pages_srial_number),1,infid);
	fwrite(tempdesc.descrip,tempdesc.pages_key_nums*(sizeof(DESCRIP)+sizeof(POINYXY)),1,infid);
	rewind(infid);
	fwrite(&Downloadescribe, sizeof(Downloadescribe.versionType)+sizeof(Downloadescribe.filesize)+sizeof(Downloadescribe.nums_desc), 1, infid);
	if(tempdesc.descrip!=NULL)
	{
		free(tempdesc.descrip);
		tempdesc.descrip = NULL;
	}
	fseek(infid, 0, SEEK_END);
	lSize = ftell(infid);
	tempSize = Downloadescribe.filesize;
	if (lSize != tempSize + sizeof(Downloadescribe.versionType)+sizeof(Downloadescribe.filesize) + sizeof(Downloadescribe.nums_desc) + sizeof(Downloadescribe.srial_number))
	{
		printf("upBookList:%s file is write fail!\n",fileBookList);
		fclose(infid);
		return 0;
	}
	fclose(infid);
	return Downloadescribe.filesize;

}

int delBookList(string fileBookList, string booksrial_number)
{
	string fileBookList_new=fileBookList;
	fileBookList_new.append("new");
	BOOKDESCRIP Downloadescribe;
	TEMPLATEDES tempdesc;
	int knems;
	FILE *infid;
	FILE *infid_new;
	infid = fopen(fileBookList.c_str(), "rb");
	if (infid == NULL)
	{
		printf("upBookList:%s is not opened!\n",fileBookList);
		return 0;
	}
	//获取文件大小
	long tempSize;
	fseek(infid, 0, SEEK_END);
	long lSize = ftell(infid);
	rewind(infid);
	fread((void *)&Downloadescribe,sizeof(Downloadescribe.versionType)+sizeof(Downloadescribe.filesize)+sizeof(Downloadescribe.nums_desc)+ sizeof(Downloadescribe.srial_number), 1, infid);
	if(strcmp(Downloadescribe.versionType,VERSION_TYPE)!=0)
	{
		printf("error:delBookList:Library version mismatch:%s\n",Downloadescribe.versionType);
		fclose(infid);
		return 0;
	}
	tempSize = Downloadescribe.filesize;
	if (lSize != tempSize + sizeof(Downloadescribe.versionType)+sizeof(Downloadescribe.filesize) + sizeof(Downloadescribe.nums_desc) + sizeof(Downloadescribe.srial_number))
	{
		printf("error:delBookList:%s file is damage off!\n",fileBookList);
		fclose(infid);
		return 0;
	}
	infid_new = fopen(fileBookList_new.c_str(),"wb");
	fwrite(&Downloadescribe, sizeof(Downloadescribe.versionType)+sizeof(Downloadescribe.filesize) + sizeof(Downloadescribe.nums_desc) + sizeof(Downloadescribe.srial_number), 1, infid_new);
	int i=0;
	knems = Downloadescribe.nums_desc;
	Downloadescribe.filesize = 0;
	Downloadescribe.nums_desc = 0;
	while(knems>i)
	{
		fread(&tempdesc,sizeof(tempdesc.pages_key_nums) + sizeof(tempdesc.pages_srial_number),1,infid);
		if(strcmp(tempdesc.pages_srial_number,booksrial_number.c_str())!=0)
		{
			if(tempdesc.pages_key_nums<50 ||tempdesc.pages_key_nums>500)
			{
				printf("error:delBookList:tempdesc.pages_key_nums=%d\n",tempdesc.pages_key_nums);
				fclose(infid);
				fclose(infid_new);
				return 0;
			}
			tempdesc.descrip = malloc(tempdesc.pages_key_nums*(sizeof(DESCRIP)+sizeof(POINYXY)));
			if(tempdesc.descrip==NULL)
			{
				printf("error:delBookList:malloc memory spach failed!\n");
				fclose(infid);
				fclose(infid_new);
				return 0;
			}
			fread(tempdesc.descrip,tempdesc.pages_key_nums*(sizeof(DESCRIP)+sizeof(POINYXY)),1,infid);
			fwrite(&tempdesc,sizeof(tempdesc.pages_key_nums) + sizeof(tempdesc.pages_srial_number),1,infid_new);
			fwrite(tempdesc.descrip,tempdesc.pages_key_nums*(sizeof(DESCRIP)+sizeof(POINYXY)),1,infid_new);
			if(tempdesc.descrip!=NULL)
			{
				free(tempdesc.descrip);
				tempdesc.descrip = NULL;
			}
			Downloadescribe.nums_desc ++;
			Downloadescribe.filesize +=sizeof(tempdesc.pages_key_nums) + sizeof(tempdesc.pages_srial_number)+tempdesc.pages_key_nums*(sizeof(DESCRIP)+sizeof(POINYXY));
		}
		else
		{
			fseek(infid,tempdesc.pages_key_nums*(sizeof(DESCRIP)+sizeof(POINYXY)), SEEK_CUR);
		}
		i++;
	}
	rewind(infid_new);
	fwrite(&Downloadescribe, sizeof(Downloadescribe.versionType)+sizeof(Downloadescribe.filesize) + sizeof(Downloadescribe.nums_desc) + sizeof(Downloadescribe.srial_number), 1, infid_new);
	fseek(infid_new, 0, SEEK_END);
	lSize = ftell(infid_new);
	tempSize = Downloadescribe.filesize;
	if (lSize != tempSize + sizeof(Downloadescribe.versionType)+sizeof(Downloadescribe.filesize) + sizeof(Downloadescribe.nums_desc) + sizeof(Downloadescribe.srial_number))
	{
		printf("delBookList:%s file is write fail!\n",fileBookList_new);
		fclose(infid_new);
		return 0;
	}
	fclose(infid);
	fclose(infid_new);
	if(remove(fileBookList.c_str()))
    {
		printf("delBookList:remove %s is fail!\n",fileBookList.c_str());   
    }
	if(rename(fileBookList_new.c_str(),fileBookList.c_str()))
	{
		printf("delBookList:rename %s is fail!\n",fileBookList.c_str());   
	}
	return Downloadescribe.filesize;
}

int SearchBooksrial_number(string fileBookList, string booksrial_number)
{
	BOOKDESCRIP Downloadescribe;
	TEMPLATEDES tempdesc;
	FILE *infid;
	int findnums=0;
	infid = fopen(fileBookList.c_str(), "rb");
	if (infid == NULL)
	{
		printf("upBookList:%s is not opened!\n",fileBookList);
		return 0;
	}
	//获取文件大小
	long tempSize;
	fseek(infid, 0, SEEK_END);
	long lSize = ftell(infid);
	rewind(infid);
	fread((void *)&Downloadescribe,sizeof(Downloadescribe.versionType)+sizeof(Downloadescribe.filesize)+sizeof(Downloadescribe.nums_desc)+ sizeof(Downloadescribe.srial_number), 1, infid);
	if(strcmp(Downloadescribe.versionType,VERSION_TYPE)!=0)
	{

		printf("error:delBookList:Library version mismatch:%s\n",Downloadescribe.versionType);
		fclose(infid);
		return 0;
	}
	tempSize = Downloadescribe.filesize;
	if (lSize != tempSize + sizeof(Downloadescribe.versionType)+sizeof(Downloadescribe.filesize) + sizeof(Downloadescribe.nums_desc) + sizeof(Downloadescribe.srial_number))
	{
		printf("error:delBookList:%s file is damage off!\n",fileBookList);
		fclose(infid);
		return 0;
	}
	int i=0;
	while(Downloadescribe.nums_desc>i)
	{
		fread(&tempdesc,sizeof(tempdesc.pages_key_nums) + sizeof(tempdesc.pages_srial_number),1,infid);
		if(strcmp(tempdesc.pages_srial_number,booksrial_number.c_str())==0)
		{
			findnums++;
		}
		fseek(infid,tempdesc.pages_key_nums*(sizeof(DESCRIP)+sizeof(POINYXY)), SEEK_CUR);
		i++;
	}

	fclose(infid);
	return findnums;
}



/*
功能：检测当前次图片和上一次图片是否是相同的
参数
1：pImage 指向灰度图片数据
返回值：1：当前次和上一次图片相同
		0：当前次和上一次图片不相同
		-1：函数执行失败
*/

#define DETEC_H 180//120
#define DETEC_KEYPOINT 50
#define RATIO_DETEC 0.6f

static DESCRIP srcDesrcrp[DETEC_KEYPOINT]={0};
static int kynums_last=0;
#if USING_MODES == GK_SYSTEM
 int detetcPIC_three(Imagetype * pImage)
 {
	Imagetype img_1;
	img_1.imageData =NULL;

	int detec_w,fristpoint_h,fristpoint_w;
	int cn = 0;
	int tempkeysize ;
	unsigned char *temppdata;
	unsigned char *pTempImag;


	DESCRIP templateDesrcrp[DETEC_KEYPOINT];
	Matchtype Ptr[DETEC_KEYPOINT];
	KEYPOINT keypoints_dest[DETEC_KEYPOINT];

	if(pImage->width<DETEC_H+50) return -1;

	detec_w = DETEC_H*pImage->width/pImage->height;
	fristpoint_h = (int)((pImage->height-DETEC_H)/2);
	fristpoint_w = (int)((pImage->width-detec_w)/2);

	img_1.height = DETEC_H;
	img_1.width = detec_w;
	img_1.imageData = (unsigned char *)malloc(img_1.height*img_1.width * sizeof(unsigned char));

	if(img_1.imageData == NULL)
	{
		return -1;
	}
	pTempImag = pImage->imageData + fristpoint_h* pImage->width+fristpoint_w;
	temppdata = img_1.imageData;
	for (int i = 0; i < DETEC_H; i++)
	{		
		memcpy((void*)temppdata, (void *)pTempImag, detec_w);
		temppdata += detec_w;
		pTempImag += pImage->width;

	}

#if PICWRITE==1
	ClImage img_2;
	img_2.channels =1;
	img_2.height=img_1.height;
	img_2.width=img_1.width;
	img_2.imageData=img_1.imageData;
	clSaveImage("detetcPIC_three.bmp", &img_2) ;
#endif

	tempkeysize = detectAndCompute(&img_1, keypoints_dest, templateDesrcrp,DETEC_KEYPOINT );
	cn = hanmmingmatch(templateDesrcrp,tempkeysize,srcDesrcrp,kynums_last,Ptr,RATIO_DETEC);
	kynums_last = tempkeysize;
	memset((void *)srcDesrcrp, 0, sizeof(char) * 32 * DETEC_KEYPOINT);
	memcpy((void*)srcDesrcrp, (void *)templateDesrcrp, sizeof(char) * 32 * tempkeysize);
	if(img_1.imageData != NULL)
	{
		free(img_1.imageData);
		img_1.imageData = NULL;
	}
#if PRINTOUT==1
	printf("detetcPIC_three cn=%d\n",cn);
#endif
	if (cn > tempkeysize / 3)
	{
		return 1;
	}
	else
	{
		return 0;
	}
 }
#elif USING_MODES == T2X_SYSTEM
 int detetcPIC_three(Imagetype * pImage)
 {

	 int detec_w,fristpoint_h,fristpoint_w;
	 int cn = 0,i;
	 unsigned char *pdata;
	 unsigned char *temppdata;
	 unsigned char *pTempImag;
	 KEYPOINT_F KeyPoint[DETEC_KEYPOINT];
	 TWDESCRIPTORS pdescriptors[DETEC_KEYPOINT];
	 DESCRIP templateDesrcrp[DETEC_KEYPOINT];
	 Matchtype Ptr[DETEC_KEYPOINT];
	 int kynums=0;

	if(pImage->width<DETEC_H+50) return -1;

	detec_w = DETEC_H*pImage->width/pImage->height;
	fristpoint_h = (int)((pImage->height-DETEC_H)/2);
	fristpoint_w = (int)((pImage->width-detec_w)/2);

	 pdata = (unsigned char *)malloc(detec_w*DETEC_H*sizeof(char));
	 if(pdata == NULL)
	 {
		 return -1;
	 }
	 temppdata = pdata;
	 pTempImag = pImage->imageData + fristpoint_h* pImage->width+fristpoint_w;
	for (int i = 0; i < DETEC_H; i++)
	{		
		memcpy((void*)temppdata, (void *)pTempImag, detec_w);
		temppdata += detec_w;
		pTempImag += pImage->width;

	}

#if PICWRITE==1
	ClImage img_2;
	img_2.channels =1;
	img_2.height=DETEC_H;
	img_2.width=detec_w;
	img_2.imageData=pdata;
	clSaveImage("detetcPIC_three.bmp", &img_2) ;
#endif

	 kynums = twcomputeKeyPoints(pdata,detec_w,DETEC_H,KeyPoint,DETEC_KEYPOINT,20,8, 31);
	if(kynums>10)
	{
		tw_GaussBlur(pdata,detec_w,DETEC_H, 2.0f);
		computeDescriptors(pdata,KeyPoint,pdescriptors,kynums,detec_w);
		for(i=0;i<kynums;i++)
		{
			memcpy(templateDesrcrp[i].descrip,pdescriptors[i].descrip,32);
		}
		cn = hanmmingmatch(templateDesrcrp,kynums,srcDesrcrp,kynums_last,Ptr,RATIO_DETEC);
		kynums_last = kynums;
		memset((void *)srcDesrcrp, 0, sizeof(char) * 32 * DETEC_KEYPOINT);
		memcpy((void*)srcDesrcrp, (void *)templateDesrcrp, sizeof(char) * 32 * kynums);
	}
	if(pdata != NULL)
	{
		free(pdata);
		pdata = NULL;
	}
#if PRINTOUT==1
	printf("detetcPIC_three cn=%d\n",cn);
#endif
	if (cn > kynums / 3)
	{
		return 1;
	}
	else
	{
		return 0;
	}
 }
#endif
