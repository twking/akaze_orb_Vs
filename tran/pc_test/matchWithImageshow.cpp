#include "c_bmp.h"
#include "opencv2/features2d.hpp"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/imgproc/types_c.h"
#include <string.h>

#include "../Server/pc_dsplib/pc_dsplib.h"
#include "../Server/matchDestination/pc_huiben.h"

#pragma warning(disable:4996)

using namespace std;
using namespace cv;

#define BOOKLIST "F:/VS2012_project/database/booklist/"
#define BOOKS "F:/VS2012_project/database/books/"
#define TESTPICYURE "F:/VS2012_project/database/testPicture/"

static bool SortByM2(const DMatch &v1, const DMatch &v2)//注意：本函数的参数的类型一定要与vector中元素的类型一致  
{
	return v1.distance < v2.distance;//升序排列  
}

void viewTest(cv::Mat img_temp,cv::Mat img_src)
{
	INDENTIFOptions akaze_options;

	akaze_options.dthreshold = 0.001f;
	akaze_options.nsublevels = 2;
	akaze_options.omax = 1;
	akaze_options.keysize = 5;

	initdetectAndDescriptors(&akaze_options);
	int keynums;

	cv::Mat img_1;
	if (img_temp.rows <= 0 || img_temp.cols <= 0 )
		return ;
	if (img_temp.cols > akaze_options.img_max_width || img_temp.rows > akaze_options.img_max_height)
	{
		Size a;
		float k1, k2;
		k1 = 1.0f*img_temp.cols / akaze_options.img_max_width;
		k2 = 1.0f*img_temp.rows / akaze_options.img_max_height;
		if (k2 < k1)
			k2 = k1;
		a.height = (int)(img_temp.rows / k2);
		a.width = (int)(img_temp.cols / k2);
		resize(img_temp, img_1, a);
	}
	else
	{
		img_1 = img_temp;
	}

	std::vector<KeyPoint> keypoints_temp,keypoints_src;
	cv::Mat desc_temp,desc_src;

#if USING_MODES==T2X_SYSTEM
	Ptr<AKAZE> akz = AKAZE::create(AKAZE::DESCRIPTOR_MLDB, 256, 3, akaze_options.dthreshold, akaze_options.omax, akaze_options.nsublevels, KAZE::DIFF_PM_G2);
#elif USING_MODES==GK_SYSTEM
	Ptr<ORB> akz = ORB::create(3*akaze_options.teamplatekeyPointNums, 1.2f,8, 31, 0, 2, ORB::HARRIS_SCORE, 31,33);
#else 
	#error "USING_MODES must is AKAZE_DEFINE or ORB_DEFINE"
#endif
	akz->detect(img_1, keypoints_temp);
	keynums = (int)keypoints_temp.size();
	if(keynums>= akaze_options.teamplatekeyPointNums)
	{ 
		if (akaze_options.keymode == INDENTIFOptions::DATABASE_KEY_MODE_MAX)
		{
			std::sort(keypoints_temp.begin(), keypoints_temp.end(), SortByM3);
			keypoints_temp.erase(keypoints_temp.begin()+ akaze_options.teamplatekeyPointNums, keypoints_temp.end());
		}
		else
		{
			getKeypoint(img_1, keypoints_temp, akaze_options.teamplatekeyPointNums);
		}
		akz->compute(img_1, keypoints_temp, desc_temp);
		
		//cv::Mat img_k;
		//drawKeypoints(img_1, keypoints_temp, img_k);
		//imshow("keypomt", img_k);
		//waitKey(0);
		
	}
	//akaze_options.keysize = 20;

	akaze_options.dthreshold = 0.0007f;
#if USING_MODES==T2X_SYSTEM
	Ptr<AKAZE> akz1 = AKAZE::create(AKAZE::DESCRIPTOR_MLDB, 256, 3, 0.0007f, 1, 1, KAZE::DIFF_PM_G2);
#elif USING_MODES==GK_SYSTEM
	Ptr<ORB> akz1 = ORB::create(KEYPOINTNUMS, 1.2f,1, 31, 0, 2, ORB::HARRIS_SCORE, 31,33);
#else 
	#error "USING_MODES must is AKAZE_DEFINE or ORB_DEFINE";
#endif
	
	akz1->detect(img_src, keypoints_src);
	if(keypoints_src.size()>= KEYPOINTNUMS)
	{ 
		std::sort(keypoints_src.begin(), keypoints_src.end(), SortByM3);
		keypoints_src.erase(keypoints_src.begin()+ 50, keypoints_src.end());
		//getKeypoint(img_src, keypoints_src,150);
		akz1->compute(img_src, keypoints_src, desc_src);
			
		//cv::Mat img_k;
		//drawKeypoints(img_src, keypoints_src, img_k);
		//imshow("keypomt", img_k);
		//waitKey(0);
		

	}
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
	vector< vector<DMatch> > matches;
	vector<DMatch> goodmatches;
    int matchCount = 0;
    matcher->knnMatch(desc_src,desc_temp, matches, 2);
		
    for (unsigned i = 0; i < matches.size(); i++)
    {
        if (matches[i][0].distance < 0.8 * matches[i][1].distance)
        {
            matchCount++;
			goodmatches.push_back(matches[i][0]);
        }
    }
	printf("matchCount=%d\n",matchCount);
	 if (8 < matchCount)
        {
           matchCount=8;
		   std::sort(goodmatches.begin(),goodmatches.end(),SortByM2);  
		   goodmatches.erase(goodmatches.begin()+matchCount,goodmatches.end());
		   POINYXY pScr[150];// = new POINYXY[pBOOKDescribe->pages_key_nums];
		   POINYXY pDest[150];// = new POINYXY[pBOOKDescribe->pages_key_nums];
			for (int j = 0; j<8; j++)
			{
				pDest[j].x = keypoints_src[goodmatches[j].queryIdx].pt.x;
				pDest[j].y = keypoints_src[goodmatches[j].queryIdx].pt.y;
				pScr[j].x = keypoints_temp[goodmatches[j].trainIdx].pt.x;
				pScr[j].y = keypoints_temp[goodmatches[j].trainIdx].pt.y;

			}
		cmpKeypoint(pDest, pScr,matchCount);


        }

	Mat img_mathes; 
    cv::drawMatches(img_src, keypoints_src,img_1, keypoints_temp,  goodmatches, img_mathes); 
	cv::imshow("img_mathes",img_mathes);
	waitKey();
}


void main()
{

	string strtemp=TESTPICYURE;
	strtemp.append("1.bmp");
	string strsrc=TESTPICYURE;
	strsrc.append("R_2048.bmp");
	cv::Mat gimg_temp,img_temp=imread(strtemp);
	cvtColor(img_temp, gimg_temp, cv::COLOR_RGB2GRAY);
	cv::Mat img_src=imread(strsrc);
	viewTest(gimg_temp,img_src);

}