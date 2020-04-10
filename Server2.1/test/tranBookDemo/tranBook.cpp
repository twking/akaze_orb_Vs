#include "tran.h"
#include "helper.h"

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
    // 获取排序的书本列表
    std::vector<string> page_list;
   	string bookstr = BOOKS;
#ifdef _WIN32
    bookstr.append("da_wei_re_ma_fang\\Pictures");//wo_ba_ba hai_xiu_de_ai_mi_li da_wei_re_ma_fang
#else
    bookstr.append("13140/Pictures");//wo_ba_ba hai_xiu_de_ai_mi_li da_wei_re_ma_fang
#endif

#ifdef _WIN32
	getAllFiles(bookstr,page_list, ".jpg");
#else
	 get_filelist_with(bookstr, ".png|.jpg", page_list);
#endif
    for(int i=0;i < page_list.size(); i++)
    {
        printf("%s\n", page_list[i].c_str());
    }
    // 训练书本
    if(page_list.size() > 0)
    {        
        //./database/booklist/ 尾部一定要带斜杠
        tranBook("13140", page_list, "./database/booklist/");
    }
    else
    {
        printf("[ERROR] book %s has no page\n", "./database/books/13140/Pictures");
    }
    return 0;
}
