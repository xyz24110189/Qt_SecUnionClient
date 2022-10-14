#include "server.h"
#include <utils.h>
#include <tools.h>
#include <parson.h>
#include <commonDef.h>
#include <algorithm>

ServerPipe::ServerPipe() 
	: m_interval(50)
{
	LOG_INFO << "ServerPipe::ServerPipe begin...";
	initChannel();
	LOG_INFO << "ServerPipe::ServerPipe end...";
}

ServerPipe::~ServerPipe() {
	LOG_INFO << "ServerPipe::~ServerPipe() begin...";
	unInit();
	LOG_INFO << "ServerPipe::~ServerPipe() end...";
}

void ServerPipe::initChannel()
{
	LOG_INFO << "ServerPipe::initChannel() begin...";
	NetConfig config;
	getNetConfig(config);

	std::string strAddr = config.channelAddr;
	if (strAddr.empty())
	{
		strAddr = "tcp://*:8877";
#ifdef _WIN32
		if (!isGreaterXp())
			strAddr = "ipc://union_channel";
#endif // _WIN32
	}

#ifdef _WIN32
	if (!isGreaterXp())
		m_interval = 200;
#endif // _WIN32

	m_pSink = createMSGServer(this, strAddr.c_str());
	if (m_pSink == NULL) {
		LOG_INFO << "Server m_pSink == NULL ";
	}
	LOG_INFO << "ServerPipe::initChannel end...";
}


ServerPipe *ServerPipe::instance()
{
	static ServerPipe inst;
	return &inst;
}

void ServerPipe::unInit()
{
	if (m_pSink) {
		freeMSGServer(m_pSink);
	}
}

bool ServerPipe::onLogin(IClientItem * pCli, char * pTips) {
	LOG_INFO << "ServerPipe::onLogin begin...";

	LOG_INFO << "appName = "<< pCli->getAppName() << " appId = " << pCli->getAppID();
	auto iter = m_vecClients.begin();
	while (iter != m_vecClients.end())
	{
		if (!std::string((*iter)->getAppID()).compare(pCli->getAppID()))
			iter = m_vecClients.erase(iter);
		else ++iter;
	}
	
	m_vecClients.emplace_back(pCli);
	LOG_INFO << "ServerPipe::onLogin end...";
	return true;
}

bool ServerPipe::onHb(IClientItem * pCli)
{
	m_logInCallback(pCli);
	return true;
}

bool ServerPipe::onClientMsg(IClientItem * pfromCli, unsigned short msg, unsigned char * buf, int bufLen) {
	LOG_INFO << "ServerPipe::onClientMsg begin...";

	std::string cmdJson;
	std::string strMsg((const char *)buf, bufLen);
	LOG_INFO << "appName = " << pfromCli->getAppName() << " appId = " \
		<< pfromCli->getAppID() << " msgId = " << msg << " msg = " << strMsg.c_str();

	buildCmdJson(pfromCli, msg, strMsg, cmdJson);
	m_msgCallback(msg, cmdJson);

	LOG_INFO << "ServerPipe::onClientMsg end...";
	return true;
}

void ServerPipe::onLogout(IClientItem * pCli, bool isClean) {
	LOG_INFO << "ServerPipe::onLogout begin...";

	LOG_INFO << "appName = " << pCli->getAppName() << " appId = " << pCli->getAppID();
	//logoutMsgToTray(pCli);
	if (isClean) m_logOutCallback(pCli);
	auto iter = m_vecClients.begin();
	while (iter != m_vecClients.end())
	{
		if (*iter == pCli)
		{
			m_vecClients.erase(iter);
			break;
		}
		++iter;
	}

	LOG_INFO << "ServerPipe::onLogout end...";
}

bool ServerPipe::sendMsg(const std::string &appId,
	unsigned short msg,
	unsigned char *buf,
	int bufLen)
{
	LOG_INFO << "ServerPipe::sendMsg msgCode = " << msg << " msg = " << buf;
	static int64 lasterTime = koal::tool::getStartMsec();
	for (const auto &item : m_vecClients)
	{
		if (!appId.compare(item->getAppID()))
		{
			int64 currentTime = koal::tool::getStartMsec();
			int64 interval = m_interval - (currentTime - lasterTime);
			lasterTime = currentTime;
			if (interval > 0)
				koal::tool::sleep(interval);

			bool bRet = m_pSink->sendMsg(item, msg, buf, bufLen);
			if (!bRet) LOG_ERROR << "sendMsg failed!!"; 
			return bRet;
		}
	}
	return false;
	LOG_INFO << "ServerPipe::sendMsg end...";
}

bool ServerPipe::getAppIdByAppName(const std::string &appName/*in*/, std::string &appId/*out*/)
{
	LOG_INFO << "ServerPipe::getAppIdByAppName begin...";
	auto iter = m_vecClients.begin();
	while (iter != m_vecClients.end())
	{
		if (!std::string((*iter)->getAppName()).compare(appName))
		{
			appId = (*iter)->getAppID();
			return true;
		}
		++iter;
	}
	LOG_INFO << "ServerPipe::getAppIdByAppName end...";
	return false;
}

bool ServerPipe::logoutMsgToTray(IClientItem * pfromCli)
{
	std::string cmdJson;
	std::string strMsg = "{}";
	buildCmdJson(pfromCli, MSG_LOGOUT_CLEAN_DEAL, strMsg, cmdJson);
	return sendMsg(APPID_TRAY, MSG_SEND_TO_TRAY, \
		(unsigned char *)cmdJson.c_str(), cmdJson.length());
}

void ServerPipe::buildCmdJson(IClientItem * pfromCli,
	unsigned short msg,
	const std::string &strJson,
	std::string &cmdJson/*out*/)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	
	json_object_set_string(root_object, "appName", pfromCli->getAppName());
	json_object_set_string(root_object, "appId", pfromCli->getAppID());
	json_object_set_number(root_object, "cmdCode", msg);
	JSON_Value *conten_value = json_parse_string(strJson.c_str());
	json_object_set_value(root_object, "content", conten_value);

	char *serialized_string = nullptr;
	serialized_string = json_serialize_to_string_pretty(root_value);
	cmdJson = serialized_string;
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
}