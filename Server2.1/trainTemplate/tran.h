#ifndef _TW_TRAN__
#define _TW_TRAN__

#include "../pc_dsplib/tw_tpyedef.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

int inittranBookDescrib(INDENTIFOptions *options);
int tranBookDescrib(string str, string fileType);
int tranContentDescrib(string str, string fileType);
// 新增通用函数
int tranBook(const char* book_name, std::vector<string> &page_list, std::string out_path);
#ifdef __cplusplus
}
#endif

#endif