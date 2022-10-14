#include "client.h"
#include "buildDataUtils.h"
#include <plog/Log.h>
#include <commonMsgDef.h>
#include <tools.h>
#include <stdio.h>

extern int g_error_code;
ClientPipe::ClientPipe() {
	initLog();
	initChannel();
}

ClientPipe::~ClientPipe() {
	m_pSink->logout();
	if (m_pSink) {
		freeMSGClient(m_pSink);
	}
}

bool ClientPipe::initChannel()
{
	LOG_INFO << "ClientPipe::initChannel begin...";
	std::string strAddr = "tcp://127.0.0.1:8877";
#ifdef _WIN32
	if (!koal::tool::isGreaterXp())
		strAddr = "ipc://union_channel";
#endif // _WIN32
	m_pSink = createMSGClient(this, strAddr.c_str());

	if (m_pSink == nullptr) 
	{
		g_error_code = ERR_CREATE_CONNECT_FAILED;
		LOG_ERROR << "createMSGClient m_pSink  failed!!";
	}
	LOG_INFO << "ClientPipe::initChannel end...";
	return nullptr != m_pSink;
}

bool ClientPipe::initLog()
{
	static bool runOnce = false;
	if (runOnce) return true;

	std::string logPath = koal::tool::getAppDataPath();
	std::string exeName = koal::tool::getExecName();
	char slash;
#ifdef _WIN32
	slash = '\\';
#else
	slash = '/';
#endif //_WIN32
	logPath += slash;
	logPath += "logs";
	koal::tool::createDir(logPath.data());

	logPath += slash;
	logPath += exeName;
	logPath += "_client.log";

	fprintf(stdout, "logPath.c_str() = %s\n", logPath.c_str());
	plog::init(plog::debug, logPath.c_str(), 10000000, 10);

	runOnce = true;
	return runOnce;
}

bool ClientPipe::init(const LOGINDATA &logData) {
	LOG_INFO << "ClientPipe::init begin...";
	bool ret = m_pSink->login(logData.strAppName, logData.strAppId, logData.strAppToken, true);

	if (ret == false) {
		LOG_ERROR << "login error!";
		g_error_code = ERR_LOGIN_FAILED;
		return false;
	}
	LOG_INFO << "ClientPipe::init end...";
	return true;
}

void ClientPipe::onReLogin(bool loginStatus)
{
	LOG_INFO << "ClientPipe::onReLogin! loginStatus = %d", loginStatus;
	if (m_reLoginFun)
		m_reLoginFun(loginStatus);
}

void ClientPipe::onLogout()
{
	LOG_INFO << "ClientPipe::onLogout login abort!";
}

bool ClientPipe::onServerMsg(unsigned short msg, unsigned char * buf, int bufLen)
{
	LOG_INFO << "ClientPipe::onServerMsg begin...";
	LOG_INFO << "ClientPipe::onServerMsg msgCode = " << msg << " buf = " << (const char *)buf << " bufLen = " << bufLen;
	std::string strMsg((const char *)buf, bufLen);
	if (msg == MSG_SEND_TO_APP)
	{
		std::string appName;
		std::string msg;
		parseTransmitDataJson(strMsg, appName, msg);

		if (m_msgFun) m_msgFun(appName.c_str(), msg.c_str(), msg.length(), m_param);
	}
	else
	{
		if (m_msgFun) m_msgFun("", (const char *)buf, bufLen, m_param);
	}
	LOG_INFO << "ClientPipe::onServerMsg end...";
	return true;
}

void ClientPipe::onError(int err, const char * tip)
{
	LOG_ERROR << "ClientPipe::onError err = " << err << " tipMsg = " << tip;
}

bool ClientPipe::sendMsg(unsigned short msg, unsigned char *buf, int bufLen)
{
	LOG_INFO << "ClientPipe::sendMsg : msg = " << msg << " msg = " << (const char *)buf;
	bool bRet = m_pSink->sendMsg(msg, buf, bufLen, nullptr, 0);
	if (!bRet)
	{
		g_error_code = 2;//ERR_SEND_MESSAGE_FAILED
		LOG_INFO << "sendMsg failed!";
	}
	return bRet;
}

