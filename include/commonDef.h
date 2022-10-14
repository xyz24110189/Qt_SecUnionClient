#pragma once

#include <plog/Log.h>
#include <parson.h>
#include <string>
#include <cstdint>
#include <vector>
#include <commonMsgDef.h>

#ifdef _WIN32
#define BINARY_TRAY "KCliBaseTray.exe"
#elif  __linux__
#define BINARY_TRAY "KCliBaseTray"
#endif

#define APPID_TRAY "856A3F47900E42C090AB906C1893D25A"
#define APPID_NDS  "3144FD603B444A76AF88503989A8BF01"
#define APPID_NAC  "9853246CE7E042999E379B2B66682F4A"
#define APPID_PKI_SERVICE "DE437B9014604F3EB129E13978EDFAE8"

/* 产品分类 */
enum ProcType: uint8_t
{
	INVALID,
	UNION_TRAY,           /* 安全聚合托盘客户端 */
	NDS,                  /* 身份鉴别系统客户端 */
	NAC,                  /* 网络接入控制系统客户端 */
	PKI_SERVICE           /* pki中间件 */
};

/* ipc 类型 */
enum ConnProtocal
{
	PIPE,                /* 管道 */
	SOCK,                /* socket */
	INPRO                /* 进程内 */
};

/* 错误号 */
enum ErrCode
{
	ERR_SUCCESS  = 0x00000000,           /* 错误 */
	ERR_ERROR,                           /* 成功 */
	ERR_JSON_PARSE_ERROR                 /* json解析失败 */
};

/* 控制程序类型 */
enum ProgramOper {
	START,
	KILL
};

struct BaseData {
	uint16_t    cmdCode;                         /* 命令code */
	std::string appName;                         /* 应用名称 */
	std::string appId;                           /* 应用ID */
};

struct Position{
	unsigned short priority;                     /* 优先级(值越小优先级越高, 0 为默认优先级) */
	std::string    parentMenus;                  /* 父级菜单链 */
};

struct ActionData : public BaseData, public Position
{
	std::string actionName;					  /* action 名字 */
	std::string cmd;				          /* 具体操作命令 */
	std::string args;                         /* 附加参数 */

	friend bool operator==(const ActionData &_Left, const ActionData &_Right)
	{
		return (_Left.appId       == _Right.appId &&
				_Left.appName     == _Right.appName &&
				_Left.cmdCode     == _Right.cmdCode &&
				_Left.parentMenus == _Right.parentMenus && 
				_Left.actionName  == _Right.actionName &&
				_Left.cmd         == _Right.cmd);
	}

	friend bool operator<(const ActionData &_Left, const ActionData &_Right)
	{
		return  (_Left.appId < _Right.appId) ||
				(_Left.appId == _Right.appId && _Left.appName < _Right.appName) ||
				(_Left.appId == _Right.appId && _Left.appName == _Right.appName && _Left.cmdCode < _Right.cmdCode) ||
				(_Left.appId == _Right.appId && _Left.appName == _Right.appName && _Left.cmdCode == _Right.cmdCode && _Left.parentMenus < _Right.parentMenus) ||
				(_Left.appId == _Right.appId && _Left.appName == _Right.appName && _Left.cmdCode == _Right.cmdCode && _Left.parentMenus == _Right.parentMenus && _Left.actionName < _Right.actionName) ||
				(_Left.appId == _Right.appId && _Left.appName == _Right.appName && _Left.cmdCode == _Right.cmdCode && _Left.parentMenus == _Right.parentMenus && _Left.actionName == _Right.actionName && _Left.cmd < _Right.cmd);
	}
};

struct SeviceData : public BaseData, public Position
{
	std::string actionName;					  /* action 名字 */
	std::string serverName;					  /* 服务 名字 */
	std::string args;                         /* 附加参数 */

	friend bool operator==(const SeviceData &_Left, const SeviceData &_Right)
	{
		return (_Left.appId       == _Right.appId &&
				_Left.appName     == _Right.appName &&
				_Left.cmdCode     == _Right.cmdCode &&
				_Left.parentMenus == _Right.parentMenus &&
				_Left.actionName  == _Right.actionName &&
				_Left.serverName  == _Right.serverName);
	}

	friend bool operator<(const SeviceData &_Left, const SeviceData &_Right)
	{
		return  (_Left.appId < _Right.appId) ||
				(_Left.appId == _Right.appId && _Left.appName < _Right.appName) ||
				(_Left.appId == _Right.appId && _Left.appName == _Right.appName && _Left.cmdCode < _Right.cmdCode) ||
				(_Left.appId == _Right.appId && _Left.appName == _Right.appName && _Left.cmdCode == _Right.cmdCode && _Left.parentMenus < _Right.parentMenus) ||
				(_Left.appId == _Right.appId && _Left.appName == _Right.appName && _Left.cmdCode == _Right.cmdCode && _Left.parentMenus == _Right.parentMenus && _Left.actionName < _Right.actionName) ||
				(_Left.appId == _Right.appId && _Left.appName == _Right.appName && _Left.cmdCode == _Right.cmdCode && _Left.parentMenus == _Right.parentMenus && _Left.actionName == _Right.actionName && _Left.serverName < _Right.serverName);
	}
};

/* 外部程序操作数据 */
struct ProgramData : public BaseData
{
	std::string exePath;
	std::string exeParam;
	uint16_t oper;

	friend bool operator==(const ProgramData &_Left, const ProgramData &_Right)
	{
		return (_Left.appId == _Right.appId &&
			_Left.appName == _Right.appName &&
			_Left.cmdCode == _Right.cmdCode &&
			_Left.exePath == _Right.exePath &&
			_Left.exeParam == _Right.exeParam &&
			_Left.oper == _Right.oper);
	}
};

/* service 程序的控制数据结构 */
struct ServiceOp : public BaseData
{
	std::string serviceName;
	std::string args;
};

/* 托盘信息数据结构 */
struct TrayIcons : public BaseData
{
	std::string iconPath;
	std::string holdTips;
};

/* 托盘冒泡信息数据结构 */
struct TrayTips : public BaseData
{
	int holdTime;
	std::string tips;
};

/* 网络配置数据 */
struct NetConfig
{
	std::string portOffset;
	std::string channelAddr;
};

