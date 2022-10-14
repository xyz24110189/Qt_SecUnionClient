#include "InteractiveService.h"
#include <QThreadPool>
#include <QThread>
#include <QTimer>
#include <QStringList>
#include <commonDef.h>
#include <utils.h>
#include <tools.h>
#include "server.h"
#include "DataParseEngine.h"
#include "KeepAliveRunable.h"
#include "ProgramOperatorRunable.h"

IClientItem *g_client = nullptr;

InteractiveService::InteractiveService(int argc, char **argv) :
	QtService<QCoreApplication>(argc, argv, "KCliBaseService")
{
	setServiceDescription("格尔客户端基础业务中间件");
	setServiceFlags(QtService::CanBeSuspended);
}

InteractiveService::~InteractiveService()
{
}

void InteractiveService::start()
{
	LOG_INFO << "InteractiveService::start()";
	_db.InitDb();

	ServerPipe *pipeInst = ServerPipe::instance();
	pipeInst->msgCallBack(std::bind(&InteractiveService::ParseClientData, \
		this, std::placeholders::_1, std::placeholders::_2));
	pipeInst->logInCallBack(std::bind(&InteractiveService::OnLogin, \
		this, std::placeholders::_1));
	pipeInst->logOutCallBack(std::bind(&InteractiveService::OnLoginOut, \
		this, std::placeholders::_1));

	_keepRunable = std::make_shared<KeepAliveRunable>();
	_programRunable = std::make_shared<ProgramOperatorRunable>();
	QThreadPool::globalInstance()->start(_keepRunable.get());
	QThreadPool::globalInstance()->start(_programRunable.get());

	/*std::vector<std::string> strVec;
	_db.QueryEvents(strVec);
	for (const auto &item : strVec)
		ParseData(item);*/
		
	LOG_INFO << "InteractiveService::end()";
}

void InteractiveService::stop()
{
	LOG_INFO << "InteractiveService::stop()";
	_keepRunable->shutDown();
	_programRunable->ShutDown();
	//ServerPipe::instance()->unInit();
}

void InteractiveService::pause()
{
	LOG_INFO << "InteractiveService::pause()";
}

void InteractiveService::resume()
{
	LOG_INFO << "InteractiveService::resume()";
}

void InteractiveService::processCommand(int code)
{
	LOG_INFO << "InteractiveService::processCommand(int code)";
}

void InteractiveService::ParseClientData(unsigned short msg, const std::string &jData)
{
	if (msg < MSG_INNER_OUTER_SPLITER)
		ParseInnerData(jData);
	else
	{
		ParseData(jData);
		ServerPipe *pipeInst = ServerPipe::instance();
		pipeInst->sendMsg(APPID_TRAY, MSG_SEND_TO_TRAY, (unsigned char *)jData.data(), jData.length());
	}
}

void InteractiveService::OnLogin(void *pClient)
{
	IClientItem *client = (IClientItem *)pClient;

	if ((g_client && g_client->getID() != client->getID()) || !g_client)
	{
		//LOG_INFO << "InteractiveService::OnLogin  pClientAppName = " << client->getAppName() << " pClientAppID = " << client->getAppID();
		if (!std::string("unionTray").compare(client->getAppName()) &&
			!std::string(APPID_TRAY).compare(client->getAppID()))
		{
			LOG_INFO << "unionTray  has logined! select data from local db.";
			g_client = client;
			std::vector<std::string> strVec;
			_db.QueryEvents(strVec);
			if (strVec.empty()) return;

			LOG_INFO << "unionTray  has logined! send message to tray. size = " << strVec.size();
			ServerPipe *pipeInst = ServerPipe::instance();
			std::string strMsg;
			for (const auto &item : strVec)
			{
				strMsg.append(item);
				strMsg.append("&&");
			}
			if (!strMsg.empty())
				strMsg.erase(strMsg.length() - 2);

			pipeInst->sendMsg(APPID_TRAY, MSG_SEND_TO_TRAY, (unsigned char *)strMsg.c_str(), strMsg.length());
		}
	}	
}

void InteractiveService::OnLoginOut(void *pClient)
{
	IClientItem *client = (IClientItem *)pClient;
	if (std::string("unionTray").compare(client->getAppName()) &&
		std::string(APPID_TRAY).compare(client->getAppID()))
	{
		ServerPipe *pipeInst = ServerPipe::instance();
		BaseData data;
		data.appId = client->getAppID();
		data.appName = client->getAppName();
		_db.DeleteAppData(data);
		_keepRunable->deleteAppData(data);
		pipeInst->logoutMsgToTray(client);
	}
}

void InteractiveService::ParseData(const std::string &jData)
{
	BaseData data;
	JSON_Value *root_value = json_parse_string(jData.c_str());
	if (!root_value) return;
	JSON_Object *root_object = json_value_get_object(root_value);
	if (!root_object) return;

	if (json_object_has_value(root_object, "cmdCode"))
		data.cmdCode = static_cast<int32_t>(json_object_get_number(root_object, "cmdCode"));

	DataParseEngine parseEngine;
	if (data.cmdCode >= MSG_PROCESS_CONTROL_REGISTER &&
		data.cmdCode < MSG_PROCESS_MONITOR_REGISTER + MSG_RANGE)
	{
		ActionData actionData;
		parseEngine.ParsonActionObj(root_object, actionData);
		_db.HandleActionData(actionData, jData);

		if (data.cmdCode >= MSG_PROCESS_MONITOR_REGISTER &&
			data.cmdCode < MSG_PROCESS_MONITOR_REGISTER + MSG_RANGE)
			_keepRunable->handleMonitorObject(actionData);
	}
	else if (data.cmdCode >= MSG_SERVICE_CONTROL_REGISTER &&
		data.cmdCode < MSG_SERVICE_CONTROL_REGISTER + MSG_RANGE)
	{
		std::vector<SeviceData> vData;
		parseEngine.ParsonServiceObj(root_object, vData);
		for (const auto &item : vData)
			_db.HandleSeviceData(item, jData);
	}
	else if (data.cmdCode == MSG_EXTERNAL_PROGRAM_OPER)
	{
		ProgramData progData;
		DataParseEngine::ParsonProgramOperObj(root_object, progData);
		_programRunable->AddProgramTask(progData);
	}

	json_value_free(root_value);
}

void InteractiveService::ParseInnerData(const std::string &jData)
{
	JSON_Value *root_value = json_parse_string(jData.c_str());
	if (!root_value) return ;

	BaseData data;
	DataParseEngine parseEngine;
	JSON_Object *root_object = json_value_get_object(root_value);
	if (!root_object) goto end_label;
	data.cmdCode = static_cast<int32_t>(json_object_get_number(root_object, "cmdCode"));

	if (data.cmdCode == MSG_SEND_TO_APP)
	{
		std::string appName;
		std::string strMsg;
		parseEngine.ParsonTransmitObj(root_value, appName, strMsg);

		ServerPipe *pipeInst = ServerPipe::instance();
		std::string strAppId;
		bool bRet = pipeInst->getAppIdByAppName(appName, strAppId);
		if (!bRet) goto end_label;
		pipeInst->sendMsg(strAppId, MSG_SEND_TO_APP, (unsigned char *)strMsg.data(), strMsg.length());
	}
	else if (data.cmdCode == MSG_PROCESS_MONITOR_OPEN || data.cmdCode == MSG_PROCESS_MONITOR_CLOSE)
	{
		ActionData vData;
		parseEngine.ParsonActionInnerObj(root_object, vData);
		vData.cmdCode = (data.cmdCode == MSG_PROCESS_MONITOR_OPEN) ?
			MSG_PROCESS_MONITOR_REGISTER : MSG_PROCESS_MONITOR_UNREGISTER;
		_keepRunable->handleMonitorObject(vData);
	}
	else if (data.cmdCode == MSG_SERVICE_CONTROL_START || data.cmdCode == MSG_SERVICE_CONTROL_STOP)
	{
		ServiceOp serviceOp;
		parseEngine.ParsonServiceOper(root_object, serviceOp);
		if (serviceOp.cmdCode == MSG_SERVICE_CONTROL_START)
		{
			QtServiceController controller(serviceOp.serviceName.c_str());
			controller.start(QString(serviceOp.args.c_str()).split('&', QString::SkipEmptyParts));
		}
		else if (serviceOp.cmdCode == MSG_SERVICE_CONTROL_STOP)
		{
			QtServiceController controller(serviceOp.serviceName.c_str());
			controller.stop();
		}
	}

end_label:
	json_value_free(root_value);
}