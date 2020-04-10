
#include "c_bmp.h"
#include "opencv2/features2d.hpp"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/imgproc/types_c.h"
#include <string.h>

using namespace std;
using namespace cv;

#pragma warning(disable:4996)

void mainI()
{

	char srcfiles[128];
	char desfiles[128];
	ClImage bmpgimg_temp;
	for(int i=0;i<22;i++)
	{
		sprintf(srcfiles,"F:\\VS2012_project\\matlab_huiben\\s%d.bmp",i);
		sprintf(desfiles,"F:\\VS2012_project\\matlab_huiben\\1_%d.bmp",i);

		cv::Mat gimg_temp,img_temp=imread(srcfiles);
		cvtColor(img_temp, gimg_temp, cv::COLOR_RGB2GRAY);
		bmpgimg_temp.channels=1;
		bmpgimg_temp.height = gimg_temp.rows;
		bmpgimg_temp.width = gimg_temp.cols;
		bmpgimg_temp.imageData = gimg_temp.data;
		gimg_temp = clSaveImage(desfiles,&bmpgimg_temp);  
	}


}