#include "client.h"
#include <commonDef.h>
#include <fstream>
#include <utils.h>

ClientPipe::ClientPipe() {
	initChannel();
}

ClientPipe::~ClientPipe() {
	if (m_pSink) {
		freeMSGClient(m_pSink);
	}
}

bool ClientPipe::initChannel()
{
	std::string strAddr;
	getChannelInfo(strAddr);
	if (strAddr.empty())
	{
		strAddr = "tcp://127.0.0.1:8877";
#ifdef _WIN32
		if (!isGreaterXp())
			strAddr = "ipc://union_channel";
#endif // _WIN32
	}

	m_pSink = createMSGClient(this, strAddr.c_str());

	return nullptr != m_pSink;
}

ClientPipe *ClientPipe::instance()
{
	static ClientPipe inst;
	return &inst;
}

void ClientPipe::init() {
	bool ret = m_pSink->login("unionTray", "856A3F47900E42C090AB906C1893D25A", "appToken");
	if (ret == false) {
		return;
	}
}

void ClientPipe::onReLogin(bool loginStatus)
{
	LOG_INFO << "ClientPipe::onReLogin!";
	m_reLoginCallBack(loginStatus);
}

void ClientPipe::onLogout()
{
	LOG_INFO << "ClientPipe::onLogout login abort! ";
}

bool ClientPipe::onServerMsg(unsigned short msg, unsigned char * buf, int bufLen)
{
	LOG_INFO << "ClientPipe::onServerMsg msgCode = " << msg << " msg = " << (const char *)buf << " bufLen = " << bufLen;
	if (msg == MSG_SEND_TO_TRAY)
	{
		std::string strMsg((const char *)buf, bufLen);
		if (m_msgCallBack) m_msgCallBack(strMsg);
	}

	return true;
}

void ClientPipe::onError(int err, const char * tip)
{
	LOG_INFO << "ClientPipe::onError err = " << err << " tipMsg = " << tip;
}

bool ClientPipe::sendMsg(unsigned short msg, unsigned char *buf, int bufLen)
{
	LOG_INFO << "ClientPipe::sendMsg : msg = " << msg << " msg = " << (const char *)buf << " bufLen = " << bufLen;
	return m_pSink->sendMsg(msg, buf, bufLen, nullptr, 0);
}

