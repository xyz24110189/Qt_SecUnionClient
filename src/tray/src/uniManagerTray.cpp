#include "uniManagerTray.h"
#include <QtCore/QCoreApplication>
#include <QtGui/QSystemTrayIcon>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QIcon>
#include <QtCore/QProcess>
#include <QtCore/QDir>
#include <QtCore/QMetaType>

#ifdef TEST_MONITOR_MARCO
#include "client.h"
#include <QFileInfo>
#include <QtCore/QStringList>
#endif

#include "DataQAction.h"
#include "ActionEngine.h"
#include "processControl.h"
#include "trayInfoControl.h"
#include "serviceControl.h"
#include "selectFileDialog.h"

UniManagerTray::UniManagerTray(QWidget *parent)
	: QWidget(parent)
{
	InitAction();
	CreateTrayIcon();
	InitConnection();
}

void UniManagerTray::Show()
{
	_trayIcon->show();
}

void UniManagerTray::InitConnection()
{
	qRegisterMetaType<std::string>("const std::string &");

	connect(this, SIGNAL(MsgReveive(const std::string &)), 
		this, SLOT(OnThreadSecRecv(const std::string &)), Qt::QueuedConnection);
	connect(this, SIGNAL(Relogin(bool)), 
		this, SLOT(OnThreadSecRelogin(bool)), Qt::QueuedConnection);
}

void UniManagerTray::InitAction()
{
	_quiteAction = new QAction(tr("空空如也"), this);
	_lastAction = _quiteAction;
	connect(_quiteAction, SIGNAL(triggered()), qApp, SLOT(quit()));
	
#ifdef TEST_MONITOR_MARCO
	_monitorAction = new QAction(tr("添加保活程序"), this);
	_lastAction = _monitorAction;
	connect(_monitorAction, &QAction::triggered, this, [=]() {
		SelectFileDialog dialog;
		dialog.exec();
		QString strPath = dialog.GetProgramPath();
		if (strPath.isEmpty()) return;

		QStringList programList = strPath.split("&&");
		QFileInfo info(programList[0]);

		std::string actName = info.fileName().toLocal8Bit() + std::string(" 程序");
		nlohmann::json cmd_build = {
			{
				{ "actName", actName },
				{ "cmd",     QDir::toNativeSeparators(programList[0]).toLocal8Bit() },   //运行程序绝对路径
				{ "args",
				{
					programList[1].toLocal8Bit()               //日志等级
				}
				}
			},
		};

		std::string strMsg = cmd_build.dump();
		ClientPipe *inst = ClientPipe::instance();
		inst->sendMsg(MSG_PROCESS_MONITOR_REGISTER, (unsigned char *)strMsg.c_str(), strMsg.length());
	});
#endif
}

void UniManagerTray::CreateTrayIcon()
{
	_trayIconMenu = new QMenu(this);
#ifdef TEST_MONITOR_MARCO
	_trayIconMenu->addAction(_monitorAction);
#endif
	//_trayIconMenu->addAction(_quiteAction);

	_trayIcon = new QSystemTrayIcon(this);
	_trayIcon->setIcon(QIcon(":/uniManagerTray/image/tray_nomal.png"));
	_trayIcon->setContextMenu(_trayIconMenu);
	_trayIcon->setToolTip(trUtf8("格尔统一安全中间件"));
	_trayIcon->setVisible(true);
}

bool UniManagerTray::ParseJsonData(const JSON_Object *obj)
{
	LOG_INFO << "UniManagerTray::ParseJsonToType begin:";
	if (!obj)
	{
		LOG_ERROR << "json parse error!";
		return false;
	}

	BaseData data;
	if (json_object_has_value(obj, "cmdCode"))
		data.cmdCode = static_cast<int32_t>(json_object_get_number(obj, "cmdCode"));

	if (data.cmdCode == MSG_LOGOUT_CLEAN_DEAL)
	{
		ActionEngine<ProcessControl> *ProcEngine = nullptr;
		auto iterPro = _mapActProcess.find(MSG_PROCESS_CONTROL_REGISTER);
		if (iterPro != _mapActProcess.end())
		{
			ProcEngine = (ActionEngine<ProcessControl> *)iterPro->second;
			ProcEngine->Resolve(obj);
		}

		ActionEngine<ServiceControl> *ServiEngine = nullptr;
		auto iterSer = _mapActProcess.find(MSG_SERVICE_CONTROL_REGISTER);
		if (iterSer != _mapActProcess.end())
		{
			ServiEngine = (ActionEngine<ServiceControl> *)iterSer->second;
			ServiEngine->Resolve(obj);
		}

		ActionEngine<TrayInfoControl> *TrayEngine = nullptr;
		auto iter = _mapActProcess.find(MSG_CHANGE_TRAYICONS_INFO);
		if (iter != _mapActProcess.end())
		{
			TrayEngine = (ActionEngine<TrayInfoControl> *)iter->second;
			TrayEngine->Resolve(obj);
		}
	}
	else if (data.cmdCode == MSG_CHANGE_TRAYICONS_INFO || 
		data.cmdCode == MSG_SHOW_TRAY_TIPS)
	{
		ActionEngine<TrayInfoControl> *engine = nullptr;
		auto iter = _mapActProcess.find(MSG_CHANGE_TRAYICONS_INFO);
		if (iter != _mapActProcess.end())
			engine = (ActionEngine<TrayInfoControl> *)iter->second;
		else
		{
			engine = new ActionEngine<TrayInfoControl>(this);
			_mapActProcess.emplace(MSG_CHANGE_TRAYICONS_INFO, engine);
		}
		engine->Resolve(obj);
	}
	else if (data.cmdCode >= MSG_PROCESS_CONTROL_REGISTER && 
		data.cmdCode < MSG_PROCESS_CONTROL_REGISTER + MSG_RANGE)
	{
		ActionEngine<ProcessControl> *engine = nullptr;
		auto iter = _mapActProcess.find(MSG_PROCESS_CONTROL_REGISTER);
		if (iter != _mapActProcess.end())
			engine = (ActionEngine<ProcessControl> *)iter->second;
		else
		{
			engine = new ActionEngine<ProcessControl>(this);
			_mapActProcess.emplace(MSG_PROCESS_CONTROL_REGISTER, engine);
		}
		engine->Resolve(obj);
	}
	else if (data.cmdCode >= MSG_PROCESS_MONITOR_REGISTER && 
		data.cmdCode < MSG_PROCESS_MONITOR_REGISTER + MSG_RANGE)
	{
		ActionEngine<ProcessControl> *engine = nullptr;
		auto iter = _mapActProcess.find(MSG_PROCESS_MONITOR_REGISTER);
		if (iter != _mapActProcess.end())
			engine = (ActionEngine<ProcessControl> *)iter->second;
		else
		{
			engine = new ActionEngine<ProcessControl>(this);
			_mapActProcess.emplace(MSG_PROCESS_MONITOR_REGISTER, engine);
		}
		engine->Resolve(obj);
	}
	else if (data.cmdCode >= MSG_SERVICE_CONTROL_REGISTER && 
		data.cmdCode < MSG_SERVICE_CONTROL_REGISTER + MSG_RANGE)
	{
		ActionEngine<ServiceControl> *engine = nullptr;
		auto iter = _mapActProcess.find(MSG_SERVICE_CONTROL_REGISTER);
		if (iter != _mapActProcess.end())
			engine = (ActionEngine<ServiceControl> *)iter->second;
		else
		{
			engine = new ActionEngine<ServiceControl>(this);
			_mapActProcess.emplace(MSG_SERVICE_CONTROL_REGISTER, engine);
		}
		engine->Resolve(obj);
	}

	return true;
	LOG_INFO << "UniManagerTray::ParseJsonToType end:";
}

void UniManagerTray::HandleActionDynamic(const std::string &strJson)
{
	LOG_INFO << "UniManagerTray::AddActionDynamic begin:";
	QStringList strJsonList = QString::fromStdString(strJson).split("&&");
	for (int i = 0; i < strJsonList.size(); ++i)
	{
		JSON_Value *root_value = json_parse_string(strJsonList[i].toLocal8Bit());
		if (!root_value) return;
		JSON_Object *root_object = json_value_get_object(root_value);
		ParseJsonData(root_object);
		json_value_free(root_value);
	}
	LOG_INFO << "UniManagerTray::AddActionDynamic end:";
}

void UniManagerTray::RestoreTrayIconInfo()
{
	_trayIcon->setIcon(QIcon(":/uniManagerTray/image/tray_nomal.png"));
	_trayIcon->setToolTip(trUtf8("统一安全中间件"));
}

void UniManagerTray::ChangeTrayIconsInfo(const std::string &iconPath,
	const std::string &tips)
{
	LOG_INFO << "UniManagerTray::ChangeTrayIconsInfo begin:";
	QString nativePath = QDir::toNativeSeparators(iconPath.c_str());
	_trayIcon->setIcon(QIcon(nativePath));

	_trayIcon->setToolTip(trUtf8(tips.c_str()));
	LOG_INFO << "UniManagerTray::ChangeTrayIconsInfo end:";
}

void UniManagerTray::ShowTips(const std::string &tips, int holdTime)
{
	_trayIcon->showMessage("", trUtf8(tips.c_str()), QSystemTrayIcon::NoIcon, holdTime * 1000);
}

void UniManagerTray::SetTrayIco(bool status)
{
	if (status)
		_trayIcon->setIcon(QIcon(":/uniManagerTray/image/tray_nomal.png"));
	else
		_trayIcon->setIcon(QIcon(":/uniManagerTray/image/tray_unusual.png"));
}

void UniManagerTray::AddProcMenu(const std::string &appName, QMenu *menu)
{
	LOG_INFO << "UniManagerTray::AddProcMenu begin:";
	LOG_INFO << "MenuName = " << menu->title().toLocal8Bit().constData();
	_trayIconMenu->insertMenu(_lastAction, menu);	
	_mapProcMenu.emplace(appName, menu);
	LOG_INFO << "UniManagerTray::AddProcMenu end:";
}

void UniManagerTray::RemoveProcMenu(const std::string &appName, QMenu *menu)
{
	LOG_INFO << "UniManagerTray::RemoveProcMenu begin";
	_trayIconMenu->removeAction(menu->menuAction());
	_mapProcMenu.erase(appName);
	menu->deleteLater();
	LOG_INFO << "UniManagerTray::RemoveProcMenu end";
}

QMenu*  UniManagerTray::FindProcMenu(const std::string &appName)
{
	auto iter = _mapProcMenu.find(appName);
	return (iter != _mapProcMenu.end()) ? iter->second : nullptr;
}

QMenu* UniManagerTray::CreateMenu(const QString &name, QWidget *parent)
{
	LOG_INFO << "UniManagerTray::CreateMenu begin:";
	return new QMenu(name, parent == nullptr ? this : parent);
	LOG_INFO << "UniManagerTray::CreateMenu end:";
}

DataQAction* UniManagerTray::CreateAction(const QString &name, QWidget *parent)
{
	LOG_INFO << "UniManagerTray::CreateAction begin:";
	LOG_INFO << "actionName = " << name.toLocal8Bit().constData();
	return new DataQAction(name, parent == nullptr ? this : parent);
	LOG_INFO << "UniManagerTray::CreateAction end:";
}

void UniManagerTray::OnReceiveData(const std::string &data)
{
	LOG_INFO << "UniManagerTray::OnReceiveData begin:";
	emit MsgReveive(data);
	LOG_INFO << "UniManagerTray::OnReceiveData end:";
}

void UniManagerTray::OnThreadSecRecv(const std::string &data)
{
	LOG_INFO << "UniManagerTray::OnThreadSecRecv begin:";
	HandleActionDynamic(data);
	LOG_INFO << "UniManagerTray::OnThreadSecRecv end:";
}

void UniManagerTray::OnThreadSecRelogin(bool loginStatus)
{
	LOG_INFO << "UniManagerTray::OnThreadSecRelogin begin:";
	SetTrayIco(loginStatus);
	LOG_INFO << "UniManagerTray::OnThreadSecRelogin end:";
}

void UniManagerTray::OnRelogin(bool loginStatus)
{
	LOG_INFO << "UniManagerTray::OnRelogin begin:";
	emit Relogin(loginStatus);
	LOG_INFO << "UniManagerTray::OnRelogin end:";
}
