
#include "../pc_dsplib/pc_dsplib.h"
#include "tran.h"
#include "xxhash.h"
#include "helper.h"
#include <ctime>
#include <string.h>
#include <fstream>
#include <io.h>
#include <stdio.h>

#pragma warning(disable:4996)

using namespace std;
using namespace cv;


static INDENTIFOptions tran_options;
static unsigned long long calcul_hash(const void *buffer, size_t length)
{
	unsigned long long const seed = 0; /* or any other value */
	unsigned long long const hash = XXH64(buffer, length, seed);
	return hash;
}

int inittranBookDescrib(INDENTIFOptions *options)
{
	if (options->dthreshold > 0.0005f&&options->dthreshold < 0.1f)
	{
		tran_options.dthreshold = options->dthreshold;
	}
	else
	{
		return -1;
	}
	if (options->omax >= 1 && options->omax <= 4)
	{
		tran_options.omax = options->omax;
	}
	else
	{
		return -1;
	}
	if (options->nsublevels >= 1 && options->nsublevels <= 4)
	{
		tran_options.nsublevels = options->nsublevels;
	}
	else
	{
		return -1;
	}
	if (options->teamplatekeyPointNums >= 200 && options->teamplatekeyPointNums <= 500)
	{
		tran_options.teamplatekeyPointNums = options->teamplatekeyPointNums;
	}
	else
	{
		return -1;
	}
	if (options->keymode == INDENTIFOptions::DATABASE_KEY_MODE_MAX || options->keymode == INDENTIFOptions::DATABASE_KEY_MODE_MEAN)
	{
		tran_options.keymode = options->keymode;
	}
	else
	{
		return -1;
	}
	if (options->keysize >= 5 && options->keysize <= 15)
	{
		tran_options.keysize = options->keysize;
	}
	else
	{
		return -1;
	}

	if (options->img_max_width >= 300 && options->img_max_width <= 1500)
	{
		tran_options.img_max_width = options->img_max_width;
	}
	else
	{
		return -1;
	}
	if (options->img_max_height >= 300 && options->img_max_height <= 1500)
	{
		tran_options.img_max_height = options->img_max_height;
	}
	else
	{
		return -1;
	}
	return initdetectAndDescriptors(&tran_options);

}
int tranBookDescrib(string str, string fileType)
{
	vector<string> files;
	vector<string> res;
	getAllFiles(str, files, fileType);
	if (files.size() < 1)
	{
		return -1;
	}
#ifdef _WIN32
	const char * split = "\\";
#else
    const char * split = "/";
#endif
	char * p;
	//先将要切割的字符串从string类型转换为char*类型  
	char * strs = new char[files[0].length() + 1]; //不要忘了  
	if (strs == NULL)
	{
        printf("[ERROR]%d", __LINE__);
		return -1;
	}
	strcpy(strs, files[0].c_str());
	p = strtok(strs, split);
	while (p != NULL)
	{
		string s = p; //分割得到的字符串转换为string类型  
		res.push_back(s); //存入结果数组  
		p = strtok(NULL, split);
	}
	delete[] strs;
	if (res.size() <3)
	{
        printf("[ERROR]%d\n", __LINE__);
        for (int i = 0; i < res.size(); i++)
        {
            printf("res[%d] = %s", i, res[i].c_str());
        }
		return -1;
	}
	strs = new char[res[res.size() - 3].length() + 1];
	if (strs == NULL)
	{
        printf("[ERROR]%d\n", __LINE__);
		return -1;
	}
	strcpy(strs, res[res.size() - 3].c_str());
	BOOKDESCRIP booksDesr;
	booksDesr.filesize = 0;
	booksDesr.nums_desc = files.size();

	char has128[32] = {0};
	{
		unsigned long long hash_value = calcul_hash((void *)strs, strlen(strs));
		snprintf(has128, 32, "%016llx", hash_value);
		printf("BOOKNAME = %s,hash_value=%lld,has128=%s\n", strs, hash_value, has128);
	}
	memcpy((void *)booksDesr.srial_number, has128, 32);

	delete[] strs;
	string tempstr,istr;
	Mat img_1;
	Mat gimg_1;
	TEMPLATEDES keyAndDes;
	keyAndDes.descrip = new DESCRIP[tran_options.teamplatekeyPointNums];
	keyAndDes.ponit = new POINYXY[tran_options.teamplatekeyPointNums];
	if (keyAndDes.descrip == NULL|| keyAndDes.ponit ==NULL) 
	{	
		if (keyAndDes.descrip != NULL) delete[] keyAndDes.descrip;
		if (keyAndDes.ponit != NULL) delete[] keyAndDes.ponit;
		printf("[ERROR] keyAndDes.descrip keyAndDes.ponit memory error\n");
		return -1;
	}
	booksDesr.pDescrip = &keyAndDes;

	string filestr = str + split +has128;
	FILE *fid;
	fid = fopen(filestr.c_str(), "wb");
	if (fid == NULL)
	{
		printf("write file failed!\n");
		return -1;
	}
	printf("Writing %s file\n", filestr.c_str());
	fwrite((char *)&booksDesr, sizeof(booksDesr.versionType)+sizeof(booksDesr.filesize)+sizeof(booksDesr.nums_desc)+sizeof(booksDesr.srial_number), 1, fid);

	unsigned int lSize = ftell(fid);
	for (unsigned int i = 1; i <= files.size(); i++)
	{
		stringstream num_str;
		num_str << i;
		istr = num_str.str();
#ifdef _WIN32
		tempstr = str +"\\"+istr + fileType;
#else
	    tempstr = str +"/"+istr + fileType;;
#endif
		printf("running%s\n",tempstr.c_str());
		img_1 = cv::imread(tempstr);
		cvtColor(img_1, gimg_1, cv::COLOR_RGB2GRAY);
		if (i == 1)
		{
			filestr += ".bmp";
			cv::imwrite(filestr, img_1);
		}
		
		//gimg_1 = img_1;
		int keynums = detectAndDescriptors(gimg_1, &keyAndDes);
		int  index = tempstr.find_last_of('.');
#ifdef _WIN32
		int start = tempstr.find_last_of('\\');
#else
	    int start = tempstr.find_last_of('/');
#endif
		string substring = tempstr.substr(start+1, index - start-1);
		memset(keyAndDes.pages_srial_number,0,32);
		memcpy(keyAndDes.pages_srial_number, substring.c_str(), substring.length());
		keyAndDes.pages_key_nums = keynums;
		fwrite((char *)&keyAndDes, sizeof(keyAndDes.pages_key_nums)+sizeof(keyAndDes.pages_srial_number), 1, fid);
		fwrite(keyAndDes.descrip, sizeof(DESCRIP), keynums, fid);
		fwrite(keyAndDes.ponit, sizeof(POINYXY), keynums, fid);
	}
	unsigned int lastlSize = ftell(fid);
	booksDesr.filesize = lastlSize - lSize;
	rewind(fid);
	fwrite((char *)&booksDesr,sizeof(booksDesr.versionType)+sizeof(booksDesr.filesize), 1, fid);
	fclose(fid);
	if (keyAndDes.descrip != NULL) delete[] keyAndDes.descrip;
	if (keyAndDes.ponit != NULL) delete[] keyAndDes.ponit;
	printf("Write file finshed\n");
	return files.size();


	
}
int tranContentDescrib(string str, string fileType)
{
	vector<string> files;
	vector<string> res;
	getAllFiles(str, files, fileType);
	if (files.size() < 1)
	{
        printf("[ERROR]%d %s %s\n", __LINE__, str.c_str(), fileType.c_str());
		return -1;
	}
	
	BOOKDESCRIP booksDesr;
	booksDesr.filesize = 0;
	booksDesr.nums_desc = files.size();
	const char *bookuuid = "booklist";
	char has128[32] = {0};
	{
		unsigned long long hash_value = calcul_hash((void *)bookuuid, strlen(bookuuid));
		snprintf(has128, 32, "%016llx", hash_value);
		printf("booklist = %s,hash_value=%lld,has128=%s\n", bookuuid, hash_value, has128);
	}
	memcpy((void *)booksDesr.srial_number, has128, 32);
	Mat img_1;
	Mat gimg_1;
	TEMPLATEDES keyAndDes;
	keyAndDes.descrip = new DESCRIP[tran_options.teamplatekeyPointNums];
	keyAndDes.ponit = new POINYXY[tran_options.teamplatekeyPointNums];
	if (keyAndDes.descrip == NULL|| keyAndDes.ponit ==NULL) 
	{	
		if (keyAndDes.descrip != NULL) delete[] keyAndDes.descrip;
		if (keyAndDes.ponit != NULL) delete[] keyAndDes.ponit;
		printf("[ERROR] keyAndDes.descrip keyAndDes.ponit memory error\n");
		return -1;
	}
	booksDesr.pDescrip = &keyAndDes;

	string filestr = str + "/" + has128;
	FILE *fid;
	fid = fopen(filestr.c_str(), "wb");
	if (fid == NULL)
	{
		printf("write file failed!\n");
		return -1;
	}
	printf("Writing %s file\n", filestr.c_str());
	fwrite((char *)&booksDesr, sizeof(booksDesr.versionType)+sizeof(booksDesr.filesize)+sizeof(booksDesr.nums_desc)+sizeof(booksDesr.srial_number), 1, fid);
	unsigned int lSize = ftell(fid);

	for (unsigned int i = 0; i < files.size(); i++)
	{
	
		printf("running%s\n",files[i].c_str());
		img_1 = cv::imread(files[i]);
		cvtColor(img_1, gimg_1, cv::COLOR_RGB2GRAY);
		//gimg_1 = img_1;
		int keynums = detectAndDescriptors(gimg_1, &keyAndDes);
		int  index = files[i].find_last_of('.');
#ifdef _WIN32
		int start = files[i].find_last_of('\\');
#else
	    int start = files[i].find_last_of('/');
#endif
		string substring = files[i].substr(start + 1, index - start - 1);
		memset(keyAndDes.pages_srial_number, 0, 32);
		memcpy(keyAndDes.pages_srial_number, substring.c_str(), substring.length());
		keyAndDes.pages_key_nums = keynums;
		fwrite((char *)&keyAndDes, sizeof(keyAndDes.pages_key_nums) + sizeof(keyAndDes.pages_srial_number), 1, fid);
		fwrite(keyAndDes.descrip, sizeof(DESCRIP), keynums, fid);
		fwrite(keyAndDes.ponit, sizeof(POINYXY), keynums, fid);
	}
	unsigned int lastlSize = ftell(fid);
	booksDesr.filesize = lastlSize - lSize;
	rewind(fid);
	fwrite((char *)&booksDesr,sizeof(booksDesr.versionType)+sizeof(booksDesr.filesize), 1, fid);
	fclose(fid);
	if (keyAndDes.descrip != NULL) delete[] keyAndDes.descrip;
	if (keyAndDes.ponit != NULL) delete[] keyAndDes.ponit;
	printf("Write file finshed\n");
	return files.size();
}

/*
*	功能说明:
*       根据Page路径,截取页面数量
*	输入:
*		page_path	路径./database/books/13140/Pictures/1.png
*		page_snsize page_sn大小
*	输出:
*		page_sn		1
*/
inline int get_page_snnumber(const char *page_path, char *page_sn, int page_snsize)
{
	const char *p1 = strrchr(page_path, '/');
	const char *p2 = strrchr(page_path, '.');
	if (p2 - p1 > 0)
	{
		int len = p2 - p1 - 1;
		memcpy(page_sn, p1+1, len > page_snsize ? page_snsize : len);
		return 1;
	}
	return 0;
}
/*
* 功能说明:
*         训练一本书籍(注:目录作为一本书籍)
* 参数说明:
*		  book_path 书本Pictures文件路径(./database/books/13140/Pictures)
*         book_type 书本Pictures文件后缀(.jpg|.png)
*         page_list 页面路径列表(注意:顺序排列,可提高识别效率)
*		  out_path  书本特征数据存储路径(./database/booklist/)
* 返回:
*		  -1 代表输入数据有误,否者返回特征文件大小
*/

int tranBook(const char* book_name, std::vector<string> &page_list, std::string out_path)
{
	INDENTIFOptions options;
	options.dthreshold = 0.001f;
	options.nsublevels = 2;
	inittranBookDescrib(&options); //初始化一些训练参数，按照上面设计即可
	BOOKDESCRIP booksDesr;
	booksDesr.filesize = 0;
	booksDesr.nums_desc = page_list.size();
	char hash128[32] = {0}; get_hash128(book_name,  hash128, 32);	
	printf("[DEBUG] BOOKNAME = %s,hash128=%s\n", book_name, hash128);

	memcpy((void *)booksDesr.srial_number, hash128, 32);	
	TEMPLATEDES keyAndDes;
	{
		keyAndDes.descrip = new DESCRIP[tran_options.teamplatekeyPointNums];
		keyAndDes.ponit = new POINYXY[tran_options.teamplatekeyPointNums];
	}
	if (keyAndDes.descrip == NULL|| keyAndDes.ponit ==NULL) 
	{	
		if (keyAndDes.descrip != NULL) delete[] keyAndDes.descrip;
		if (keyAndDes.ponit != NULL) delete[] keyAndDes.ponit;
		printf("[ERROR] keyAndDes.descrip keyAndDes.ponit memory error\n");
		return -1;
	}

	else
	{
		booksDesr.pDescrip = &keyAndDes;
	}	
	out_path += hash128;
	FILE *fid = fopen(out_path.c_str() , "wb");
	if (fid == NULL)
	{		
		printf("[ERROR] open out_path failed!\n");
		return -1;
	}
	
	fwrite((char *)&booksDesr, sizeof(booksDesr.versionType)+sizeof(booksDesr.filesize)+sizeof(booksDesr.nums_desc)+sizeof(booksDesr.srial_number), 1, fid);
	unsigned int lSize = ftell(fid);			
	Mat img_1, gimg_1;
	for (unsigned int i = 0; i < page_list.size(); i++)
	{		
		img_1 = cv::imread(page_list[i].c_str());
		cvtColor(img_1, gimg_1, cv::COLOR_RGB2GRAY);		
		int keynums = detectAndDescriptors(gimg_1, &keyAndDes);
		char page_sn[32] = {0};
		if (get_page_snnumber(page_list[i].c_str(), page_sn, 31))
		{
			printf("[INFO] pagename=%s page_sn=%s\n", page_list[i].c_str(), page_sn);
			keyAndDes.pages_key_nums = keynums;
			memset(keyAndDes.pages_srial_number, 0, sizeof(keyAndDes.pages_srial_number));
			memcpy(keyAndDes.pages_srial_number, page_sn, strlen(page_sn));
		}
		else
		{
			printf("[ERROR] get_page_snnumber failed, %s\n", page_list[i].c_str());
			return -1;
		}		
		fwrite((char *)&keyAndDes, sizeof(keyAndDes.pages_key_nums)+sizeof(keyAndDes.pages_srial_number), 1, fid);
		fwrite(keyAndDes.descrip, sizeof(DESCRIP), keynums, fid);
		fwrite(keyAndDes.ponit, sizeof(POINYXY), keynums, fid);
	}
	unsigned int lastlSize = ftell(fid);
	booksDesr.filesize = lastlSize - lSize;
	rewind(fid);
	fwrite((char *)&booksDesr,sizeof(booksDesr.versionType)+sizeof(booksDesr.filesize), 1, fid);
	fclose(fid);
	if (keyAndDes.descrip != NULL) 
	{
		delete[] keyAndDes.descrip;
	}
	if (keyAndDes.ponit != NULL) 
	{
		delete[] keyAndDes.ponit;
	}
	printf("[DEBUG] write file finshed pagenum=%d\n", page_list.size());
	return page_list.size();
}