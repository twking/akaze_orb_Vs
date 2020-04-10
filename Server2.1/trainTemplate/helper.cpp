#include "helper.h"
#include "xxhash.h"
#include <algorithm>    // std::sort

#ifdef _WIN32
void getAllFiles(string path, vector<string> &files, string fileType)
{
	// 文件句柄
	long hFile = 0;
	// 文件信息
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*" + fileType).c_str(), &fileinfo)) != -1)
	{
		do
		{
			// 保存文件的全路径
			files.push_back(p.assign(path).append("\\").append(fileinfo.name));

		} while (_findnext(hFile, &fileinfo) == 0); //寻找下一个，成功返回0，否则-1

		_findclose(hFile);
	}
}
#else
/**
 * Lists all files and sub-directories recursively 
 * considering path as base path.
 */
static void listFilesRecursively(const char *basePath, const char *suffix, vector<string> &files)
{
	char path[1000];
	struct dirent *dp;
	DIR *dir = opendir(basePath);

	// Unable to open directory stream
	if (!dir)
	{
		// printf("[ERROR] Unable to open directory stream, %s\n", basePath);
		return;
	}

	while ((dp = readdir(dir)) != NULL)
	{
		if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
		{
			char *dot = strrchr(dp->d_name, '.');
			if (dot && !strcmp(dot, suffix))
			{
				char buf[1024] = {0};
				snprintf(buf, 1023, "%s/%s", basePath, dp->d_name);
				files.push_back(buf);
			}
			// printf("%s/%s\n", basePath, dp->d_name);

			// Construct new path from our base path
			strcpy(path, basePath);
			strcat(path, "/");
			strcat(path, dp->d_name);

			listFilesRecursively(path, suffix, files);
		}
	}
	closedir(dir);
}

void getAllFiles(string path, vector<string> &files, string fileType)
{
	listFilesRecursively(path.c_str(), fileType.c_str(), files);
	printf("[INFO ] %s size = %d\n", fileType.c_str(), files.size());
}
const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

bool compare_filenames(string a, string b)
{
    char *pA, *pB;
    long A = strtol(a.c_str(), &pA, 10),
         B = strtol(b.c_str(), &pB, 10);
    if (A < B)
        return true;
    if (A == B)
        return strcmp(pA, pB);
    return false;
}

void get_filelist_with(const char *path, const char *suffixs, std::vector<string> &file_list)
{	
	DIR *dirFile = opendir(path);
	if (dirFile)
	{
		struct dirent *hFile;
		errno = 0;
		char buf[100] = {0};
		while ((hFile = readdir(dirFile)) != NULL)
		{
			if (!strcmp(hFile->d_name, "."))
				continue;
			if (!strcmp(hFile->d_name, ".."))
				continue;
			const char* ext = get_filename_ext(hFile->d_name);
			if(strcmp(ext, "") && strstr(suffixs, ext))
			{
				snprintf(buf,100, "%s", hFile->d_name);
				file_list.push_back(buf);
			}
		}
		closedir(dirFile);
	}
	sort(file_list.begin(), file_list.end(), compare_filenames);
	for(int i =0; i < file_list.size(); i++)
	{
		file_list[i].insert(0, "/");
		file_list[i].insert(0, path);
	}
}
#endif

void get_hash128(const char* strs,char* hash128,int hash128size)
{
	unsigned long long const seed = 0; /* or any other value */
	unsigned long long const hash_value = XXH64(strs, strlen(strs), seed);	
	snprintf(hash128, hash128size, "%016llx", hash_value);
	printf("[INFO] BOOKNAME = %s,hash_value=%lld,hash128=%s\n", strs, hash_value, hash128);
}