#ifndef _TS_HELPER_H
#define _TS_HELPER_H


#include <string>
#include <vector>
#include <cstring>

using namespace std;
#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#include <stdio.h>
#include <io.h>
#pragma warning(disable:4996)
#define snprintf _snprintf
#endif

#ifndef _WIN32
#include <dirent.h>
#endif
void getAllFiles(string path, vector<string>& files, string fileType);
void get_filelist_with(const char *path, const char *suffixs, std::vector<string> &file_list);
void get_hash128(const char* strs,char* hash128,int hash128size);
#ifdef __cplusplus
}
#endif

#endif //_TS_HELPER_H