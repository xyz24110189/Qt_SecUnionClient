#include "KeepAliveRunable.h"
#include <QCoreApplication>
#include "ProgramOperatorRunable.h"
#include <QThread>
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QSysInfo>
#include <utils.h>
#include <tools.h>

ProgramOperatorRunable::ProgramOperatorRunable() : _shutDown(false)
{
	setAutoDelete(false);
	_semephor_ptr = std::make_shared<QSemaphore>();
}

ProgramOperatorRunable::~ProgramOperatorRunable()
{

}

void ProgramOperatorRunable::run()
{
	LOG_INFO << "ProgramOperatorRunable::run begine...";
	while (!_shutDown)
	{
		_semephor_ptr->acquire();
		if (_queueData.empty()) continue;

		ProgramData data = _queueData.dequeue();
		if (koal::tool::pathExist(data.exePath.c_str()))
		{
			bool flag = true;
			if (data.oper == ProgramOper::START)
			{
				QProcess process;
				QString qcmd = QDir::toNativeSeparators(data.exePath.c_str());
#ifdef Q_OS_WIN
				qcmd.insert(0, "\"");
				qcmd.append("\"");
				std::string cmd = qcmd.toAscii() + " " + data.exeParam.c_str();
				int pid = -1;
				if (!isGreaterXp())
					pid = CreateProcessAsUserForXp((char *)cmd.c_str());
				else
					pid = CreateProcessAsUserForCustom((char *)cmd.c_str());
				flag = (pid != -1);
#else
				QStringList args;
				args << data.exeParam.c_str();
				flag = process.startDetached(QDir::toNativeSeparators(qcmd), args);
#endif //Q_OS_WIN
				if (!flag)
				{
					LOG_ERROR << "qprocess start failed for = " << qcmd.toLocal8Bit().constData() << " arg= " << data.exeParam.c_str() << " flag = " << flag;
					continue;
				}
				LOG_ERROR << "qprocess start success for = " << qcmd.toLocal8Bit().constData();
			}
			else if (data.oper == ProgramOper::KILL)
			{
				QFileInfo info(data.exePath.c_str());
				flag = killProcess(info.fileName().toLocal8Bit().constData());
				LOG_INFO << "close process for = " << data.exePath.c_str() << " flag = " << flag;
				if (!flag)
					LOG_ERROR << "close process failed for = " << data.exePath.c_str();
			}
		}
	}
	LOG_INFO << "ProgramOperatorRunable::run end...";
}

void ProgramOperatorRunable::AddProgramTask(const ProgramData &proData)
{
	LOG_INFO << "ProgramOperatorRunable::addMonitorObject begine...";
	if (!_queueData.contains(proData))
	{
		_queueData.enqueue(proData);
		_semephor_ptr->release();
	}
	LOG_INFO << "ProgramOperatorRunable::addMonitorObject end...";
}


