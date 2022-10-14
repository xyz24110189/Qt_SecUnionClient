#ifndef THIS_IS_TOOLS_HEADER_1231231231
#define THIS_IS_TOOLS_HEADER_1231231231
#include "config.h"
#include <string>
#include <vector>
#ifdef __linux__
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS   64
#endif
#ifndef __USE_FILE_OFFSET64
#define __USE_FILE_OFFSET64 1
#endif
#endif

namespace koal {
    namespace tool {
        ///获取启动毫秒数
        int64 getStartMsec();
        ///获取系统时间，单位秒
        int64 getLocalTime();
        ///获取UUID
        std::string getUUID();
        ///睡眠
        void  sleep(int32 millsec);
        ///获取目录下所有文件
        int getFileInDir(const char * dir,std::vector<std::string> filenames);
        ///获取当前时间的格式化字符串
        void getCurTimestring(char * szTime);
        ///获取当前日期的格式化字符串
        void getCurDaystring(char * szDay);
        ///获取进程ID
        int64 getPID() ;
        ///获取线程ID
        int64 getTID() ;
        ///获取错误号
        int32 getLastError();
        ///切分字符串
        template<typename T1, typename T2>
        void  splitString( T1 & strSrc,T2 breakChar, std::vector<T1> & _vt) {
            int length = strSrc.length();
            int lastPos = 0 ;
            for(int pos = 0 ; pos < length ; pos++) {
                if(strSrc[pos] == breakChar) {
                    _vt.push_back(strSrc.substr(lastPos,pos - lastPos));
                    lastPos = pos + 1;
                }
            }
            if(lastPos != length) {
                _vt.push_back(strSrc.substr(lastPos,length - lastPos));
            } 
        }  
        ///判断大小端
        bool  isLittleEndian();
        ///英文宽字符转化为字符
        char * convEChar(const std::wstring &  wstr,char * pBuffer,int bufferLen);

		///判断图片后缀是否支持
		bool isSupportedExt(const std::string &ext);
		///获取文件后缀名
		std::string getFileExt(const std::string& strFile);
		///获取appData路径
		std::string getAppDataPath();
        ///获取程序执行路径
        std::string getExecPath();
		///获取程序名
		std::string getExecName();
        ///获取系统变量
        std::string getEnv(const char * pVar);
        ///判断是否绝对路径
        bool  isAbsPath(const char * path);
        ///创建路径
        bool  createDir(const char * pPath); 
        ///判断文件或路径是否存在
        bool  pathExist(const char *pathname);
        ///
        bool  getIPCUrl(const char * pUrl, 
                        std::string & pProto, 
						std::string & port);
#ifdef _WIN32
		bool isGreaterXp();
#endif // _WIN32
    }
}

#endif
