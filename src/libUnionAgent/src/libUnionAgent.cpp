#include "libUnionAgent.h"
#include "commonMsgDef.h"
#include "client.h"
#include "buildDataUtils.h"
#include "tools.h"
#include <plog/Log.h>
#include <algorithm>
#include <cctype>
#include <memory>
#include <map>

Encoding g_enc = UTF8;
int g_error_code = ERR_SUCCESS;
std::map<std::string, int> g_msgMap;
static std::shared_ptr<ClientPipe> g_pipe(nullptr);

void ImpOnReLogin(bool loginStatus)
{
	LOG_INFO << "ImpOnReLogin begin...  loginStatus = " << loginStatus;
	if (!loginStatus) return ;
	std::map<std::string, int>::iterator iter = g_msgMap.begin();
	for (; iter != g_msgMap.end(); ++iter)
	{
		std::string strMsg = iter->first;
		g_pipe->sendMsg(iter->second, (unsigned char *)strMsg.c_str(), strMsg.length());
	}
	LOG_INFO << "ImpOnReLogin end...";
}

void ImpStoreMsg(const std::string &strMsg, int msgCode)
{
	g_msgMap[strMsg] = msgCode;
}

void ImpInitErrCode()
{
	g_error_code = ERR_SUCCESS;
}

bool UniSecInitClient(LOGINDATA *pLogin, serverMsgCB cb, void *param, Encoding enc)
{
	g_enc = enc;
	std::string strAppId = pLogin->strAppId;
	std::string strAppName = pLogin->strAppName;
	ImpInitErrCode();
	if (strAppId.empty() || strAppName.empty())
	{
		LOG_ERROR << "login parameter is empty";
		g_error_code = ERR_PARAMTER_EMPTY;
		return false;
	}
	g_pipe.reset();
	g_pipe = std::make_shared<ClientPipe>();
	g_pipe->msgCallBack(cb, param);
	g_pipe->reLoginCallBack(ImpOnReLogin);

	encodingConv(strAppId);
	encodingConv(strAppName);
	memset(pLogin->strAppId, 0, sizeof(pLogin->strAppId));
	memset(pLogin->strAppName, 0, sizeof(pLogin->strAppName));
	memcpy(pLogin->strAppId, strAppId.c_str(), strAppId.size());
	memcpy(pLogin->strAppName, strAppName.c_str(), strAppName.size());
	bool bRet = g_pipe->init(*pLogin);
	return bRet;
}

bool UniSecUnInitClient()
{

	g_pipe.reset();
	return true;
}

int UniSecGetErrno()
{
	return g_error_code;
}

bool UniSecSendMsgToApp(const char *pAppName, const char *buf, int bufLen)
{
	LOG_INFO << "UniSecSendMsgToApp begin...";
	std::string jsonMsg;
	std::string strAppName = pAppName;
	std::string strMsg = buf;
	ImpInitErrCode();
	if (strAppName.empty())
	{
		LOG_ERROR << "strAppName parameter is empty";
		g_error_code = ERR_PARAMTER_EMPTY;
		return false;
	}
	if (strMsg.empty() || bufLen <= 0)
	{
		LOG_ERROR << "strMsg parameter is empty";
		g_error_code = ERR_EMPTY_MESSAGE;
		return false;
	}

	bool bRet = buildTransmitDataJson(pAppName, buf, bufLen, jsonMsg);
	bRet = g_pipe->sendMsg(MSG_SEND_TO_APP, (unsigned char *)jsonMsg.c_str(), jsonMsg.length());
	LOG_INFO << "UniSecSendMsgToApp end...";
	return bRet;
}

bool UniSecOpTrayMenu(MENUDATA *menu, bool isReg)
{
	LOG_INFO << "UniSecOpTrayMenu begin... strExePath = " << menu->exePath;
	std::string jsonMsg;
	std::string strExePath = menu->exePath;
	encodingConv(strExePath);

	std::string strActionName = menu->actionName;
	ImpInitErrCode();
	if (menu->priority < 0)
	{
		LOG_ERROR << "priority parameter is negative";
		g_error_code = ERR_MENU_PRIORYTY_ERROR;
		return false;
	}
	if (strExePath.empty() || strActionName.empty())
	{
		LOG_ERROR << "strExePath parameter is empty";
		g_error_code = ERR_PARAMTER_EMPTY;
		return false;
	}
	/*if (strExePath.find('\\') != std::string::npos)
	{
		g_error_code = ERR_PATH_SLASH_ERROR;
		return false;
	}
	if (!koal::tool::pathExist(strExePath.c_str()))
	{
		SLOG_ERROR("strExePath parameter is not exist");
		g_error_code = ERR_PATH_NOT_EXIST;
		return false;
	}*/

	bool bRet = buildProcessDataJson(menu, jsonMsg);

	MsgCode code = isReg ? MSG_PROCESS_CONTROL_REGISTER : MSG_PROCESS_CONTROL_UNREGISTER;
	bRet = g_pipe->sendMsg(code, (unsigned char *)jsonMsg.c_str(), jsonMsg.length());
	ImpStoreMsg(jsonMsg, code);
	LOG_INFO << "UniSecOpTrayMenu end...";
	return bRet;
}

bool UniSecRegMonitor(MENUDATA *menu, bool isReg)
{
	LOG_INFO << "UniSecRegMonitor begin... strExePath = " << menu->exePath;
	std::string jsonMsg;
	std::string strExePath = menu->exePath;
	encodingConv(strExePath);

	std::string strActionName = menu->actionName;
	ImpInitErrCode();
	if (menu->priority < 0)
	{
		LOG_ERROR << "priority parameter is negative";
		g_error_code = ERR_MENU_PRIORYTY_ERROR;
		return false;
	}
	if (strExePath.empty() || strActionName.empty())
	{
		LOG_ERROR << "strExePath parameter is empty";
		g_error_code = ERR_PARAMTER_EMPTY;
		return false;
	}
	/*if (strExePath.find('\\') != std::string::npos)
	{
	g_error_code = ERR_PATH_SLASH_ERROR;
	return false;
	}
	if (!koal::tool::pathExist(strExePath.c_str()))
	{
	SLOG_ERROR("strExePath parameter is not exist");
	g_error_code = ERR_PATH_NOT_EXIST;
	return false;
	}*/

	bool bRet = buildProcessDataJson(menu, jsonMsg);

	MsgCode code = isReg ? MSG_PROCESS_MONITOR_REGISTER : MSG_PROCESS_MONITOR_UNREGISTER;
	bRet = g_pipe->sendMsg(code, (unsigned char *)jsonMsg.c_str(), jsonMsg.length());
	ImpStoreMsg(jsonMsg, code);
	LOG_INFO << "UniSecOpTrayMenu end...";
	return bRet;
}

bool UniSecShowTrayTips (TRAYTIPS *tips)
{
	LOG_INFO << "UniSecShowTrayTips begin...";
	std::string jsonMsg;
	std::string strTips = tips->tips;
	ImpInitErrCode();
	if (strTips.empty())
	{
		LOG_ERROR << "strTips parameter is empty";
		g_error_code = ERR_PARAMTER_EMPTY;
		return false;
	}
	if (tips->holdTime < 0)
	{
		LOG_ERROR << "holdTime parameter is negative";
		g_error_code = ERR_TIP_TIME_ERROR;
		return false;
	}

	bool bRet = buildTrayTipsDataJson(tips, jsonMsg);
	bRet = g_pipe->sendMsg(MSG_SHOW_TRAY_TIPS, (unsigned char *)jsonMsg.c_str(), jsonMsg.length());
	LOG_INFO << "UniSecShowTrayTips end...";
	return bRet;
}

bool UniSecSetTrayIcon(TRAYICONS *icons)
{
	LOG_INFO << "UniSecSetTrayIcon begin...";
	std::string jsonMsg;
	std::string strIconPath = icons->iconPath;
	std::string strTips = icons->tips;
	std::string ext;
	ImpInitErrCode();
	if (strTips.empty() || strIconPath.empty())
	{
		LOG_ERROR << "strTips or strIconPath parameter is empty";
		g_error_code = ERR_PARAMTER_EMPTY;
		return false;
	}
	/*if (strIconPath.find('\\') != std::string::npos)
	{
		SLOG_ERROR("strIconPath parameter is empty");
		g_error_code = ERR_PATH_SLASH_ERROR;
		return false;
	}
	if (!koal::tool::pathExist(icons->iconPath))
	{
		SLOG_ERROR("iconPath parameter is not exist");
		g_error_code = ERR_PATH_NOT_EXIST;
		return false;
	}*/

	ext = koal::tool::getFileExt(strIconPath);
	std::transform(ext.begin(), ext.end(), ext.begin(),
		[](unsigned char c) -> unsigned char { return std::toupper(c); });
	if (!koal::tool::isSupportedExt(ext))
	{
		LOG_ERROR << "the icon format is not supported";
		g_error_code = ERR_NOT_SUPPORT_FORMATS;
		return false;
	}

	bool bRet = buildTrayIconsDataJson(icons, jsonMsg);
	bRet = g_pipe->sendMsg(MSG_CHANGE_TRAYICONS_INFO, (unsigned char *)jsonMsg.c_str(), jsonMsg.length());
	LOG_INFO << "UniSecSetTrayIcon end...";
	return bRet;
}

bool UniSecOperProgram(PROGRAMDATA *program)
{
	LOG_INFO << "UniSecOperProgram begin...";
	std::string jsonMsg;
	std::string exePath = program->exePath;
	ProgramOper op = program->oper;
	ImpInitErrCode();

	if (exePath.empty())
	{
		LOG_ERROR << "strExePath parameter is empty";
		g_error_code = ERR_PARAMTER_EMPTY;
		return false;
	}
	if (op < ProgramOper::START || op > ProgramOper::KILL)
	{
		LOG_ERROR << "oper type is not support";
		g_error_code = ERR_OP_NOT_EXIST;
		return false;
	}

	bool bRet = buildProgramOperDataJson(program, jsonMsg);
	bRet = g_pipe->sendMsg(MSG_EXTERNAL_PROGRAM_OPER, (unsigned char *)jsonMsg.c_str(), jsonMsg.length());

	LOG_INFO << "UniSecOperProgram end...";
	return bRet;
}


/*
int keepAlive(int nSize, const PROCESSDATA *dataV, bool isReg)
{
	std::string jsonMsg;
	bool bRet = buildProcessDataJson(nSize, dataV, jsonMsg);
	MsgCode code = isReg ? MSG_PROCESS_MONITOR_REGISTER : MSG_PROCESS_MONITOR_UNREGISTER;
	bRet = g_pipe->sendMsg(code, (unsigned char *)jsonMsg.c_str(), jsonMsg.length());
	return bRet ? SUCCESS : FAILURE;;
}

int serviceCtrl(int nSize, const SERVICEDATA *dataV, bool isReg)
{
	std::string jsonMsg;
	bool bRet = buildServiceDataJson(nSize, dataV, jsonMsg);
	MsgCode code = isReg ? MSG_SERVICE_CONTROL_REGISTER : MSG_SERVICE_CONTROL_UNREGISTER;
	bRet = g_pipe->sendMsg(code, (unsigned char *)jsonMsg.c_str(), jsonMsg.length());
	return bRet ? SUCCESS : FAILURE;;
}
*/

