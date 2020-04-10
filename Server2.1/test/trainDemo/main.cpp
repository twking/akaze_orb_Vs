#include "tran.h"
#ifdef _WIN32
#define BOOKLIST "F:\\VS2012_project\\database\\booklist\\"
#define BOOKS "F:\\VS2012_project\\database\\books\\"
#define TESTPICYURE "F:\\VS2012_project\\database\\testPicture\\"
#else
#define BOOKLIST "./database/booklist/"
#define BOOKS "./database/books/"
#define TESTPICYURE "./database/testPicture/"
#endif

int main()
{
	INDENTIFOptions options;
	string bookstr = BOOKS;
#ifdef _WIN32
    bookstr.append("da_wei_re_ma_fang\\Pictures");//wo_ba_ba hai_xiu_de_ai_mi_li da_wei_re_ma_fang
#else
    bookstr.append("13140/Pictures");//wo_ba_ba hai_xiu_de_ai_mi_li da_wei_re_ma_fang
#endif
	string fileType = ".jpg";
	string contentType = ".bmp";
	string contentstr = BOOKS;
#ifdef _WIN32
	contentstr.append("booklist\\Pictures");
#else
    contentstr.append("/Pictures");
#endif
	options.dthreshold = 0.001f;
	options.nsublevels = 2;
	inittranBookDescrib(&options);//初始化一些训练参数，按照上面设计即可
	int size = tranBookDescrib(bookstr, fileType);//获取书籍的训练数据接口
    if (size != -1)
    {
        printf("tranBookDescrib succeed\n");
    }
    else
    {
        printf("tranBookDescrib failed\n");
    }
	size = tranContentDescrib(contentstr, contentType);//获取目录册的训练数据接口
    if(size != -1)
    {
        printf("tranContentDescrib succeed\n");
    }
    else
    {
        printf("tranContentDescrib failed\n");        
    }    
    return 0;
}
