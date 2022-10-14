#include "tools.h"
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>


#ifdef _WIN32
#include<Windows.h>
#include <direct.h>
#include <ShlObj.h>
#include <io.h>
#else
#include <sys/types.h>
#include <dirent.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#define MAX_PATH 260
#endif

//#include <boost/uuid/uuid.hpp>
//#include <boost/uuid/uuid_io.hpp>
//#include <boost/uuid/uuid_generators.hpp>

#define INIT_PORT "5555"

static const char *imageFormats[] = { "BMP", "GIF", "JPG", "PNG", "PBM", "PGM", "PPM", "XBM", "XPM", "SVG" };

const unsigned char base64EncodeMap[64] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};

namespace koal {
	namespace tool {
		int64 getLocalTime() {
			time_t start;
			time(&start);
			static int64 sed = start;
			return start + (++sed);
		}
		std::string getUUID() {
			srand(getLocalTime());
			char sz[65] = "";
			for (int i = 0; i < 32; i++) {
				sz[i] = base64EncodeMap[rand() % 63];
			}
			/*
			boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
			const std::string tmp_uuid = oost::uuids::to_string(a_uuid);
			return tmp_uuid ;
			*/
			return sz;
		}
		///睡眠
		void  sleep(int32 millsec) {
#ifndef _WIN32
			usleep(millsec * 1000);
#else
			Sleep(millsec);
#endif
		}

		int getFileInDir(const char * dir, std::vector<std::string> filenames) {
			std::string dirName = dir;
#ifdef _WIN32
			if (dir[strlen(dir) - 1] != '\\'
				&& dir[strlen(dir) - 1] != '/') {
				dirName.append("\\");
			}
			_finddata_t fd;
			intptr_t handle;
			handle = _findfirst((dirName + "*.*").c_str(), &fd);
			if (handle != -1) {
				do {
					if (fd.attrib & _A_SUBDIR) {
						continue;
					}
					if (strcmp(fd.name, ".."))
						filenames.push_back(dirName + fd.name);
				} while (_findnext(handle, &fd) == 0);
				_findclose(handle);
			}
#else
			if (dir[strlen(dir) - 1] != '/') {
				dirName.append("/");
			}
			struct dirent* ent = NULL;
			DIR * pDir;
			pDir = opendir(dir);
			if (pDir != NULL) {
				while (NULL != (ent = readdir(pDir))) {
					if (ent->d_type == DT_DIR) {
						continue;
					}
					if (strcmp(ent->d_name, ".."))
						filenames.push_back(dirName + ent->d_name);
				}
				closedir(pDir);
			}
#endif
			return filenames.size();
		}

		int64 getStartMsec() {
#ifndef _WIN32
#ifdef  __linux__
			struct sysinfo info;
			sysinfo(&info);
			struct timeval tv;
			gettimeofday(&tv, 0);
			int64 tmp = info.uptime;
			tmp = tmp * 1000;
			return tmp + tv.tv_usec / 1000;
#endif
#ifdef __APPLE__
			struct timeval tv;
			gettimeofday(&tv, 0);
			int64 tmp = tv.tv_sec;
			tmp = tmp * 1000;
			return tmp + tv.tv_usec / 1000;
#endif
#else
			return ::GetTickCount();
#endif
			return 0;
		}

		void getCurTimestring(char * szTime) {
#ifndef _WIN32
			time_t curTm;
			time(&curTm);
			struct timeval tv;
			gettimeofday(&tv, 0);
			struct tm pTm;
			localtime_r(&curTm, &pTm);
			int32 msec = tv.tv_usec / 1000;
			sprintf(szTime, "%02d:%02d:%02d.%03d", pTm.tm_hour, pTm.tm_min, pTm.tm_sec,
				msec);
#else
			SYSTEMTIME sys;
			GetLocalTime(&sys);
			sprintf(szTime, "%02d:%02d:%02d.%03d", sys.wHour, sys.wMinute, sys.wSecond, GetTickCount() % 1000);
#endif
		}

		void getCurDaystring(char * szDay) {
#ifndef _WIN32
			time_t curTm;
			time(&curTm);
			struct tm  pTm = *localtime(&curTm);
			sprintf(szDay, "%04d%02d%02d", pTm.tm_year + 1900, pTm.tm_mon + 1,
				pTm.tm_mday);
#else
			SYSTEMTIME sys;
			GetLocalTime(&sys);
			sprintf(szDay, "%04d%02d%02d", sys.wYear, sys.wMonth,
				sys.wDay);
#endif
		}

		int64 getTID() {
#ifdef _WIN32
			int32  threadid = GetCurrentThreadId();
#else
			pthread_t  threadid = pthread_self();
#endif
			return threadid;
		}

		int64 getPID() {
#ifndef _WIN32
			int64 pid = getpid();
#else
			unsigned long pid = GetCurrentProcessId();
#endif
			return pid;
		}

		int32 getLastError() {
#ifdef _WIN32
			return GetLastError();
#else
			return errno;;
#endif
		}

		static bool _isLittleEndian() { int i = 1;  return (*(char *)&i == 1); }

		bool isLittleEndian() {
			static bool bLittleEndian = _isLittleEndian();
			return bLittleEndian;
		}

		char * convEChar(const std::wstring &  wstr, char * pBuffer, int bufferLen) {
			if (bufferLen < wstr.length()) {
				return pBuffer;
			}
			uint32 pos = 0;
			for (; pos < wstr.length(); pos++) {
				*(pBuffer + pos) = wstr[pos];
			}
			return pBuffer;
		}

		std::string getEnv(const char * pVar) {
			return "";
		}

		bool  isAbsPath(const char * path) {
			///该方法是做一个简单判断，基于输入是经过人为输入的
			if (strlen(path) < 4) { ///全路径不能少于4个字符
				return false;
			}
#ifdef _WIN32
			if (*(path + 1) != ':') { ///第二个字符不是:
				return false;
			}
#else
			if (*path != '/') {
				return false;
			}
#endif 
			return true;
		}

		bool  pathExist(const char *pathname) {
#ifndef _WIN32
			if (-1 != access(pathname, 0))
#else
			if (-1 != _access(pathname, 0))
#endif
				return true;
			return false;
		}

		bool  createDir(const char * pPath) {
			if (pathExist(pPath))
				return true;

			char tmpPath[MAX_PATH] = "";
			const char* pCur = pPath;
			memset(tmpPath, 0, sizeof(tmpPath));
			int pos = 0;
			while (*pCur++ != '\0') {
				tmpPath[pos++] = *(pCur - 1);
				if (tmpPath[0] == '\0') {
					return false;
				}
				if (*pCur == '/' || *pCur == '\0') {
					if (!pathExist(tmpPath)) {
#ifndef _WIN32
						if (mkdir(tmpPath, 0755) == -1)
							return false;
#else
						if (_mkdir(tmpPath) == -1)
							return false;
#endif
					}
				}
			}
			return true;
		}

		bool  getIPCUrl(const char * pUrl,
			std::string & pProto,
			std::string & port)
		{
			std::string  strUrl = pUrl;
			printf("23432432432 \n");
			std::string::size_type pos = strUrl.find("://");
			if (pos == std::string::npos) {
				printf("-fdafdasfdsafsdfdas\n");
				return false;
			}
			pProto = strUrl.substr(0, pos);
			printf("pProto = %s\n", pProto.c_str());
			strUrl = strUrl.substr(pos + 3, strUrl.length() - pos - 3);
			printf("strUrl = %s\n", strUrl.c_str());
			if (pProto == "tcp" || pProto == "ws") {
				pos = strUrl.find(":");
				if (pos == std::string::npos) {
					return false;
				}
				port = strUrl.substr(pos + 1, strUrl.length() - pos - 1);
			}

			return true;
		}

		bool isSupportedExt(const std::string &ext)
		{
			const char **p = imageFormats;
			while (*p != NULL)
			{
				std::string tmpExt = *p++;
				if (!tmpExt.compare(ext))
					return true;
			}
			return false;
		}

		std::string getFileExt(const std::string& strFile)
		{
			std::string::size_type pos;
			pos = strFile.rfind('.');
			if (pos != std::string::npos)
				return strFile.substr(++pos);
			return "";
		}

		std::string getAppDataPath()
		{
#ifdef _WIN32
			TCHAR szPath[MAX_PATH];

			if (SUCCEEDED(SHGetFolderPath(NULL,
				CSIDL_COMMON_APPDATA/*CSIDL_APPDATA*/ | CSIDL_FLAG_CREATE,
				NULL,
				0,
				szPath)))
			{
				std::string strDir = std::string(szPath) + "/koal/mw/secUnion";
				createDir(strDir.data());
				return strDir;
			}
#elif __linux__
			std::string strDir = "::getenv("HOME");
			strDir += "/.local/share/koal/MW/secUnion";
			createDir(strDir.data());
			return strDir;
#endif// _WIN32	
		}

		std::string getExecPath() {
#ifdef _WIN32
			TCHAR filePath[255];
			GetModuleFileName(NULL, filePath, 255);
			(strrchr(filePath, '\\'))[1] = 0;
			return filePath;
#else
			int rval;
			char link_target[4096];
			char* last_slash;
			size_t result_Length;
			char* result;

			std::string	strExeDir;

			rval = readlink("/proc/self/exe", link_target, 4096);
			if (rval < 0 || rval >= 1024)
			{
				return "";
			}
			link_target[rval] = '\0';
			last_slash = strrchr(link_target, '/');
			if (last_slash == 0 || last_slash == link_target)
				return "";

			result_Length = last_slash - link_target;
			result = (char*)malloc(result_Length + 1);
			strncpy(result, link_target, result_Length);
			result[result_Length] = '\0';

			strExeDir.append(result);
			strExeDir.append("/");
			free(result);
			return strExeDir;
#endif
		}

		std::string getExecName()
		{
#ifdef _WIN32
			TCHAR filePath[MAX_PATH];
			TCHAR *exeName = NULL;
			if (GetModuleFileNameA(NULL, filePath, MAX_PATH))
			{
				exeName = std::strrchr(filePath, '\\') + 1;
				std::strchr(exeName, '.')[0] = 0;
			}
			return exeName;
#else
			int rval;
			char link_target[4096];
			char* last_slash;

			rval = readlink("/proc/self/exe", link_target, 4096);
			if (rval < 0 || rval >= 1024)
			{
				return "";
			}
			link_target[rval] = '\0';;
			last_slash = strrchr(link_target, '/') + 1;
			if (last_slash == 0 || last_slash == link_target)
				return "";
			return last_slash;
#endif
		}

#ifdef _WIN32
		bool isGreaterXp()
		{
			OSVERSIONINFO osvi;
			BOOL bIsWindowsXPLater;

			ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
			osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

			GetVersionEx(&osvi);

			bIsWindowsXPLater = (osvi.dwMajorVersion > 5);

			return bIsWindowsXPLater;
		}
#endif //_WIN32
	}
}
