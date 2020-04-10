#include "c_bmp.h"
#include <string.h>
#include <stdio.h>
using namespace std;


#pragma warning(disable:4996)

typedef struct _Imagetype
{  
    int width;  
    int height;  
    unsigned char* imageData;  
}Imagetype;  

void imageHalf(Imagetype *Inputimage,Imagetype *Outputimage)
{
	int i,j;
	unsigned char* indata,*outdata;  
	outdata = Outputimage->imageData;
	for(i=0;i<(Inputimage->height>>1);i++)
	{
		indata =Inputimage->imageData+2*i*Inputimage->width;
		for(j=0;j<(Inputimage->width>>1);j++)
		{
			*outdata++=*(indata+j*2);
		}
	}
	Outputimage->height=Inputimage->height>>1;
	Outputimage->width=Inputimage->width>>1;
}

void mainhalf()
{

	char srcfiles[128];
	char desfiles[128];
	int i;

	ClImage *bmpgimg_temp,half_imag;

	for(i=1;i<14;i++)
	{
		sprintf(desfiles,"F:\\VS2012_project\\trantmpalte\\tran\\tran\\domepic\\%d.bmp",i);
		bmpgimg_temp=clLoadImage(desfiles);  

		Imagetype Inputimage,Outputimage;

		Inputimage.height=bmpgimg_temp->height;
		Inputimage.width=bmpgimg_temp->width;
		Inputimage.imageData=bmpgimg_temp->imageData;

		Outputimage.height=(Inputimage.height>>1);
		Outputimage.width=(Inputimage.width>>1);
		Outputimage.imageData=new unsigned char[Outputimage.height*Outputimage.width];

		imageHalf(&Inputimage,&Outputimage);

		half_imag.channels=1;
		half_imag.height = Outputimage.height;
		half_imag.width = Outputimage.width;
		half_imag.imageData = Outputimage.imageData;

		sprintf(desfiles,"F:\\VS2012_project\\trantmpalte\\tran\\tran\\domepic\\half_%d.bmp",i);
		clSaveImage(desfiles,&half_imag);  


		delete [] Outputimage.imageData;
		delete [] bmpgimg_temp->imageData;
	}


}