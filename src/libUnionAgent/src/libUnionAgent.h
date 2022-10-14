#ifndef LIB_UNION_AGENT_H
#define LIB_UNION_AGENT_H

#ifdef _WIN32

#ifdef KOAL_UNIONLIB_EXPORT
#define  KOAL_EXPORT_UNIONLIB extern "C" __declspec(dllexport) 
#else
#define  KOAL_EXPORT_UNIONLIB extern "C" __declspec(dllimport)
#endif

#define __stdcall __stdcall

#else
#define  KOAL_EXPORT_UNIONLIB extern "C" __attribute__((visibility("default")))
#define __stdcall

#endif

#define MAX_PATH 260

/**
* @brief   接受消息
* @param   [fromApp]     应用名称
* @param   [buf]         消息内容
* @param   [bufLen]      消息大小
* @param   [param]       自定义数据(携带数据或对象)
*/
typedef void (__stdcall *serverMsgCB)(const char *fromAppName, const char *buf, int bufLen, void *param);

enum ErrCode{
	//成功
	ERR_SUCCESS = 0x00000000,
	/******** 系统错误号 ********/
	/* 创建连接失败 */
	ERR_CREATE_CONNECT_FAILED,
	/* 发送消息失败 */
	ERR_SEND_MESSAGE_FAILED,
	/* 消息内容为空 */
	ERR_EMPTY_MESSAGE,
	/* 登录失败 服务异常 */
	ERR_LOGIN_FAILED,
	/* 应用已经登录 */
	ERR_HAS_LOGED,
	/* 客户端注销失败 */
	ERR_LOGOUT_FIALED,

	/* 系统错误号最大 10000 */
	ERR_MAX_SYSTEM_ERROR = 0x00002710,
	
	/******** 业务错误号 ********/
	/* 参数错误 */
	ERR_PARAMTER_ERROR,
	/* 参数不能为空 */
	ERR_PARAMTER_EMPTY,
	/* 菜单路径分割错误 */
	ERR_MENU_PATH_ERROR,
	/* 程序路径使用了右斜杠 '\' */
	ERR_PATH_SLASH_ERROR,
	/* 程序或文件路径不存在 */
	ERR_PATH_NOT_EXIST,
	/* 程序启动参数分割错误 */
	ERR_PROGRAM_PARAM_ERROR,
	/* 菜单已存在 */
	ERR_MENU_EXIST,
	/* 删除菜单失败 */
	ERR_DELETE_MENU_ERROR,
	/* 菜单优先级错误，不能为负值 */
	ERR_MENU_PRIORYTY_ERROR,
	/* 托盘显示时间不能为负数 */
	ERR_TIP_TIME_ERROR,
	/* 图标路径不存在 */
	ERR_ICON_NOT_EXIST,
	/* 不是图片文件 */
	ERR_NOT_ICON_FILE,
	/* 不支持的图片格式 */
	ERR_NOT_SUPPORT_FORMATS,
	/* 注册菜单太多(大于50个) */
	ERR_TOO_MANY_MENUS,
	/* 不存在的操作类型 */
	ERR_OP_NOT_EXIST
};

/**
* @ brief 编码格式
*/
enum Encoding {
	UTF8,
	GB2312
};

/**
* @ brief 程序操作类型
*/
enum ProgramOper {
	START,
	KILL   
};

/*
* @ brief 登录数据
* @ member [strAppName]  应用名  最长36个字节
* @ member [strAppId]    应用ID  最长36个字节
* @ member [strAppToken] 应用Token(暂时为空) 最长36个字节
*/
typedef struct _LOGINDATA
{
	char strAppName[37];   
	char strAppId[37];     
	char strAppToken[37];  
} LOGINDATA;

/**
* @ brief 菜单信息
* @ member  priority        大于等于0, 值越大优先级越高(0 用于默认优先级)
* @ member  parentMenuPath  父级菜单路径(多级路径用 '/' 字符分割)
* @ member  actionName      菜单名
* @ member  exePath         程序全路径(路径中使用左斜杠 '/')
* @ member  exeParam        启动参数(多参数用分号 ';' 字符分割)
*/
typedef struct _MENUDATA
{
	unsigned short priority;        
	char parentMenuPath[MAX_PATH];
	char actionName[64];  
	char exePath[MAX_PATH];	 
	char exeParam[MAX_PATH];   
} MENUDATA;

/**
* @ brief 托盘tooltip提示信息
* @ member tips     内容信息
* @ member holdTime 显示持续时间, 单位秒
*/
typedef struct _TRAYTIPS
{
	char  tips[MAX_PATH];
	int   holdTime ;
} TRAYTIPS;

/**
* @ brief 托盘图标信息
* @ member iconPath  图标路径
* @ member tips      鼠标放到图标上显示的内容
*/
typedef struct _TRAYICONS
{
	char  iconPath[MAX_PATH];
	char  tips[256];
} TRAYICONS;

/**
* @ brief  操作程序信息
* @ member exePath  程序全路径(路径中使用左斜杠 '/')
* @ member oper     操作类型
*/
typedef struct _PROGRAMDATA
{
	char exePath[MAX_PATH];
	char exeParam[MAX_PATH];
	ProgramOper oper;
} PROGRAMDATA;

/**
* @brief   初始化客户端
* @param   [pLogin] 初始化客户端信息
* @param   [cb]     消息回调函数
* @param   [enc]    字符编码格式
* @param   [param]  自定义数据,回调时会被携带
* @return  true: 成功,  false: 失败; 错误号通过UniSecGetErrno获取
*/
KOAL_EXPORT_UNIONLIB bool UniSecInitClient(LOGINDATA *pLogin, serverMsgCB cb, void *param, Encoding enc = UTF8);

/**
* @brief   注销客户端
* @return  true: 成功,  false: 失败; 错误号通过UniSecGetErrno获取
*/
KOAL_EXPORT_UNIONLIB bool UniSecUnInitClient();

/**
* @ brief 获取错误号
* @ return 返回错误号，可查看ErrCode定义
*/
KOAL_EXPORT_UNIONLIB int UniSecGetErrno();

/**
* @brief   发送消息给其他应用
* @param   [appName]   应用名称
* @param   [buf]       消息内容
* @param   [bufLen]    消息大小
* @param   return      true: 成功, false:失败; 错误号通过UniSecGetErrno获取
*/
KOAL_EXPORT_UNIONLIB bool UniSecSendMsgToApp(const char *pAppName, const char *buf, int bufLen);

/**
* @ brief  注册托盘菜单
* @ param  [menu]  菜单信息
* @ param  [isReg] true: 注册菜单, false:移除菜单 
* @ return true: 成功, false:失败; 错误号通过UniSecGetErrno获取
*/
KOAL_EXPORT_UNIONLIB bool UniSecOpTrayMenu (MENUDATA *menu, bool isReg = true);

/**
* @ brief 注册保活程序
* @ param [monitor] 保活菜单信息
* @ param  [isReg] true: 注册菜单, false:移除菜单
* @ return true: 成功, false:失败; 错误号通过UniSecGetErrno获取
*/
KOAL_EXPORT_UNIONLIB bool UniSecRegMonitor (MENUDATA *menu, bool isReg = true);

/**
* @ brief  修改托盘提示信息
* @ param  [tips] 托盘提示数据
* @ return true: 成功, false:失败; 错误号通过UniSecGetErrno获取
*
*/
KOAL_EXPORT_UNIONLIB bool UniSecShowTrayTips (TRAYTIPS *tips);

/**
* @ brief  设置托盘信息
* @ param  [icons] 托盘信息
* @ return true: 成功, false:失败; 错误号通过UniSecGetErrno获取
*/
KOAL_EXPORT_UNIONLIB bool UniSecSetTrayIcon(TRAYICONS *icons);

/**
* @brief   操作外部程序
* @ param  [program]  操作程序信息
* @ return true: 成功, false:失败; 错误号通过UniSecGetErrno获取
*/
KOAL_EXPORT_UNIONLIB bool UniSecOperProgram(PROGRAMDATA *program);

#endif //LIB_UNION_AGENT_H