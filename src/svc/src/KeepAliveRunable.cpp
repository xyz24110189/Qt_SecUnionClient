#include "KeepAliveRunable.h"
#include <QCoreApplication>
#include <QThread>
#include <QMutex> 
#include <QMutexLocker>
#include <QFileInfo> 
#include <QProcess>
#include <QDir>
#include <QSysInfo>
#include <utils.h>
#include <tools.h>

QMutex g_mutext;

KeepAliveRunable::KeepAliveRunable() : _shutDown(false)
{
	setAutoDelete(false);
	startServiceMonitor();
}

KeepAliveRunable::~KeepAliveRunable()
{
	endServiceMonitor();
}

void KeepAliveRunable::run()
{
	LOG_INFO << "KeepAliveRunable::run begine...";
	while (!_shutDown)
	{
		//LOG_INFO << "_queueData size = " << _queueData.size();
		for (int i = 0; i < _queueData.size(); ++i)
		{
			ActionData data = _queueData.at(i);
			QFileInfo info(data.cmd.c_str());
			std::string fileName = info.fileName().toLocal8Bit().data();
			if (koal::tool::pathExist(data.cmd.c_str()) && !isRunning(fileName))
			{
				QProcess process;
				QString qcmd = QDir::toNativeSeparators(data.cmd.c_str());

				QStringList args;
				args << data.args.c_str();

				bool flag = true;
#ifdef Q_OS_WIN
				qcmd.insert(0, "\"");
				qcmd.append("\"");
				std::string cmd = qcmd.toAscii() + " " + data.args.c_str();
				int pid = -1;
				if (!isGreaterXp())
					pid = CreateProcessAsUserForXp((char *)cmd.c_str());
				else
					pid = CreateProcessAsUserForCustom((char *)cmd.c_str());
				flag = (pid != -1);
#else
				flag = process.startDetached(QDir::toNativeSeparators(qcmd), args);
#endif //Q_OS_WIN

				if (!flag)
				{
					LOG_ERROR << "qprocess start failed for = " << qcmd.toLocal8Bit().constData() << " arg= " << data.args.c_str() <<  " flag = " << flag;
					continue;
				}
				LOG_ERROR << "qprocess start success for = " << qcmd.toLocal8Bit().constData();
			}

		}
		koal::tool::sleep(5000);
	}
	LOG_INFO << "KeepAliveRunable::run end...";
}

void KeepAliveRunable::deleteAppData(const BaseData &data)
{
	QMutexLocker lock(&g_mutext);
	QQueue<ActionData>::iterator iter = _queueData.begin();
	while (iter != _queueData.end())
	{
		if (!iter->appName.compare(data.appName) && 
			!iter->appId.compare(data.appId))
		{
			iter = _queueData.erase(iter);
			continue;
		}
		++iter;
	}
}

void KeepAliveRunable::handleMonitorObject(const ActionData &actionData)
{
	LOG_INFO << "KeepAliveRunable::handleMonitorObjectn begin...";
	switch (actionData.cmdCode)
	{
	case MSG_PROCESS_MONITOR_REGISTER:
		addMonitorObject(actionData);
		break;
	case MSG_PROCESS_MONITOR_UNREGISTER:
		removeMonitorObject(actionData);
		break;
	default:
		break;
	}
	LOG_INFO << "KeepAliveRunable::handleMonitorObjectn end...";
}

void KeepAliveRunable::addMonitorObject(const ActionData &actionData)
{
	LOG_INFO << "KeepAliveRunable::addMonitorObject begine...";
	QMutexLocker lock(&g_mutext);
	if (!_queueData.contains(actionData))
		_queueData.enqueue(actionData);
	LOG_INFO << "KeepAliveRunable::addMonitorObject end...";
}

void KeepAliveRunable::removeMonitorObject(const ActionData &actionData)
{
	LOG_INFO << "KeepAliveRunable::removeMonitorObject begine...";
	QMutexLocker lock(&g_mutext);
	ActionData unregAction = actionData;
	unregAction.cmdCode -= 1;
	_queueData.removeAll(unregAction);
	LOG_INFO << "KeepAliveRunable::removeMonitorObject end...";
}

void KeepAliveRunable::startServiceMonitor()
{
	LOG_INFO << "KeepAliveRunable::addServiceMonitor begine...";
	//保活托盘
	QString strTrayPath = QCoreApplication::applicationDirPath();
	strTrayPath += '/';
	strTrayPath += BINARY_TRAY;
	ActionData trayData;
	trayData.cmd = strTrayPath.toLocal8Bit().data();
	addMonitorObject(trayData);

	//windows 下隐藏进程
#ifdef Q_OS_WIN
	std::string appName = QCoreApplication::applicationName().toLocal8Bit();
	QString proHiderPath = QCoreApplication::applicationDirPath();
	QString proHiderFile = proHiderPath + "/x64Hider.exe";
	proHiderPath += "/ProcessHider.exe";
	proHiderPath = QDir::toNativeSeparators(proHiderPath);
	proHiderFile = QDir::toNativeSeparators(proHiderFile);

	ActionData data;
	data.actionName = "ProcessHider";
	data.cmd = proHiderPath.toLocal8Bit();
	std::string arg = " -n ";
	arg += appName;
	data.args = arg;

	QStringList args;
	args << QDir::toNativeSeparators(data.args.c_str());

	killProcess("x64Hider.exe");
	QDir dir;
	dir.remove(proHiderFile);

	if (QSysInfo::WindowsVersion <= QSysInfo::WV_WINDOWS7)
	{
		std::string cmd = data.cmd + data.args.c_str();
		int pid = CreateProcessAsUserForXp((char *)cmd.c_str(), false);
	}
	else
	{
		int result = (int)::ShellExecuteA(0, "open", data.cmd.c_str(), data.args.c_str(), 0, SW_HIDE);
		if (result < 32)
			LOG_INFO << "ShellExecuteA process failed  proName = " << appName;
	}	
#endif
	LOG_INFO << "KeepAliveRunable::addServiceMonitor end...";
}

void KeepAliveRunable::endServiceMonitor()
{
	LOG_INFO << "KeepAliveRunable::endServiceMonitor begine...";
	killProcess("ProcessHider.exe");
	killProcess("x64Hider.exe");
	LOG_INFO << "KeepAliveRunable::endServiceMonitor end...";
}

