#pragma once
#include <commonDef.h>
#include <string>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#endif //_WIN32
/**
* @brief 根据产品类型获取appId
* @param [type]     产品类型
* @param [strAppid] 返回appId
*/
bool getAppIdByProType(ProcType type, std::string &strAppId);

//控制程序启动一次
bool checkIsOne();

//判断程序是否运行 
bool isRunning(const std::string &procName);

//杀死进程
bool killProcess(const std::string &procName);

//读取配置文件
bool getNetConfig(NetConfig &config);

//保存channel连接信息
bool storeChannelInfo(const std::string &channel);

//获取channel连接信息
bool getChannelInfo(std::string &channel);


#ifdef _WIN32

//以管理员权限启动程序
bool StartProcessAsHighPrivilege(LPCSTR proPath, LPCSTR args);

//突破SESSION 0隔离创建用户进程  传入程序路径
int CreateProcessAsUserForXp(LPSTR lpCommandLine, bool showWindow = true);
int CreateProcessAsUserForSystem(LPSTR lpCommandLine);
bool CreateProcessAsUserForCustom(LPSTR process_path);

// 将正在使用的端口设置到注册表
long setCurrentChannelIntoRegister(HKEY hRootKey, LPCTSTR lpKeyName, LPCTSTR lpValueName, LPCTSTR lpValue);

// 从注册表获取端口
long getCurrentChannelFromRegister(HKEY hRootKey, LPCTSTR lpKeyName, LPCTSTR lpValueName, std::string &value);

//判断windows 系统版本是否大于 xp
bool isGreaterXp();
#endif // Q_OS_WIN

#ifdef __linux__
// 将正在使用的端口写入配置文件 
long setCurrentChannelIntoConfigFile(const char *fileName, const char *channel);
long getCurrentChannelFromConfigFile(const char *fileName, std::string &channel);
#endif //Q_OS_LINUX


