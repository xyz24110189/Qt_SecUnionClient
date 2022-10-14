#include "DataQAction.h"
#include <utils.h>
#include "client.h"
#include <QtCore/QProcess>
#include <QtCore/QDir>
#include <QtCore/QTextCodec>

DataQAction::DataQAction(QObject *parent) : QAction(parent)
{

}

DataQAction::DataQAction(const QString &text, QObject *parent) : QAction(text, parent)
{

}

DataQAction::~DataQAction()
{

}

void DataQAction::SetActionData(const ActionData &data)
{
	m_actionData = data;
}

void DataQAction::SetServiceData(const SeviceData &data)
{
	m_serviceData = data;
}

void DataQAction::OnProcessOpened()
{
	QProcess process;
	QString cmd = m_actionData.cmd.c_str();
#ifdef _WIN32
	cmd.insert(0, "\"");
	cmd.append("\"");
#endif //_WIN32
	QStringList args;
	args << QDir::toNativeSeparators(m_actionData.args.c_str());
	bool flag = process.startDetached(QDir::toNativeSeparators(cmd), args);
	if (!flag)
	{
		std::string procPath = (const char *)QDir::toNativeSeparators(cmd).toLocal8Bit();
		std::string arg = args[0].toLocal8Bit().data();
#ifdef _WIN32
		procPath = utf8ToGb2312(procPath.data());
		arg = utf8ToGb2312(arg.data());
		flag = StartProcessAsHighPrivilege(procPath.data(), arg.data());
#endif //_WIN32
		LOG_ERROR << "qprocess start failed for = " << cmd.toLocal8Bit().constData();
	}

	///触发非保活菜单，只打开程序
	if (m_actionData.cmdCode != MSG_PROCESS_MONITOR_REGISTER) return;

	char *serialized_string = nullptr;
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	json_object_set_string(root_object, "appName", m_actionData.appName.c_str());
	json_object_set_string(root_object, "appId", m_actionData.appId.c_str());
	json_object_set_number(root_object, "cmdCode", m_actionData.cmdCode);
	json_object_set_number(root_object, "priority", m_actionData.priority);
	json_object_set_string(root_object, "parentMenus", m_actionData.parentMenus.c_str());
	json_object_set_string(root_object, "actName", m_actionData.actionName.c_str());
	json_object_set_string(root_object, "cmd", m_actionData.cmd.c_str());
	json_object_set_string(root_object, "args", m_actionData.args.c_str());
	serialized_string = json_serialize_to_string_pretty(root_value);
	std::string strMsg = serialized_string;
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);

	ClientPipe *inst = ClientPipe::instance();
	inst->sendMsg(MSG_PROCESS_MONITOR_OPEN, (unsigned char *)strMsg.c_str(), strMsg.length());
}

void DataQAction::OnProcessClosed()
{
	LOG_INFO << "DataQAction::OnProcessClosed()  begin...";
	QFileInfo info(m_actionData.cmd.c_str());
	bool flag = killProcess(info.fileName().toLocal8Bit().constData());
	LOG_INFO << "close process for = " << info.fileName().toLocal8Bit().constData() << " flag = " << flag;
	if (!flag)
		LOG_ERROR << "close process failed for = " << m_actionData.cmd;

	char *serialized_string = nullptr;
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	json_object_set_string(root_object, "appName", m_actionData.appName.c_str());
	json_object_set_string(root_object, "appId", m_actionData.appId.c_str());
	json_object_set_number(root_object, "cmdCode", m_actionData.cmdCode);
	json_object_set_number(root_object, "priority", m_actionData.priority);
	json_object_set_string(root_object, "parentMenus", m_actionData.parentMenus.c_str());
	json_object_set_string(root_object, "actName", m_actionData.actionName.c_str());
	json_object_set_string(root_object, "cmd", m_actionData.cmd.c_str());
	json_object_set_string(root_object, "args", m_actionData.args.c_str());
	serialized_string = json_serialize_to_string_pretty(root_value);
	std::string strMsg = serialized_string;
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);

	ClientPipe *inst = ClientPipe::instance();
	inst->sendMsg(MSG_PROCESS_MONITOR_CLOSE, (unsigned char *)strMsg.c_str(), strMsg.length());
}

void DataQAction::OnServiceStart()
{
	//QtServiceController controller(data.serverName.c_str());
	//controller.start(QString(data.args.c_str()).split('&', QString::SkipEmptyParts));
	char *serialized_string = nullptr;
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	json_object_set_string(root_object, "serverName", m_serviceData.serverName.c_str());
	json_object_set_string(root_object, "args", m_serviceData.args.c_str());
	serialized_string = json_serialize_to_string_pretty(root_value);
	std::string strMsg = serialized_string;
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);

	ClientPipe *inst = ClientPipe::instance();
	inst->sendMsg(MSG_SERVICE_CONTROL_START, (unsigned char *)strMsg.c_str(), strMsg.length());
}

void DataQAction::OnServiceStop()
{
	//QtServiceController controller(data.serverName.c_str());
	//controller.stop();
	char *serialized_string = nullptr;
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	json_object_set_string(root_object, "serverName", m_serviceData.serverName.c_str());
	json_object_set_string(root_object, "args", "");
	serialized_string = json_serialize_to_string_pretty(root_value);
	std::string strMsg = serialized_string;
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);

	ClientPipe *inst = ClientPipe::instance();
	inst->sendMsg(MSG_SERVICE_CONTROL_STOP, (unsigned char *)strMsg.c_str(), strMsg.length());
}

std::string DataQAction::utf8ToGb2312(const char *strUtf8)
{
	QTextCodec* utf8Codec = QTextCodec::codecForName("utf-8");
	QTextCodec* gb2312Codec = QTextCodec::codecForName("gb2312");

	QString strUnicode = utf8Codec->toUnicode(strUtf8);
	QByteArray ByteGb2312 = gb2312Codec->fromUnicode(strUnicode);

	return ByteGb2312.data();
}
