#include "processControl.h"
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QProcess>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include "DataQAction.h"
#include "uniManagerTray.h"
#include "client.h"
#include <utils.h>



ProcessControl::ProcessControl(QObject *parent) : QObject(parent)
{
}


ProcessControl::~ProcessControl()
{
}

void ProcessControl::Resolve(const JSON_Object *jData, UniManagerTray *manager)
{
	_manager = manager;

	std::vector<ActionData> actVector;
	ActionData data;
	if (json_object_has_value(jData, "appName"))
		data.appName = json_object_get_string(jData, "appName");
	if (json_object_has_value(jData, "appId"))
		data.appId = json_object_get_string(jData, "appId");
	if (json_object_has_value(jData, "cmdCode"))
		data.cmdCode = static_cast<int32_t>(json_object_get_number(jData, "cmdCode"));

	JSON_Object *content = nullptr;
	if (json_object_has_value(jData, "content"))
		content = json_object_get_object(jData, "content");

	if (content)
	{
		if (json_object_has_value(content, "priority"))
			data.priority = static_cast<unsigned short>(json_object_get_number(content, "priority"));
		if (json_object_has_value(content, "parentMenus"))
			data.parentMenus = json_object_get_string(content, "parentMenus");
		if (json_object_has_value(content, "actName"))
			data.actionName = json_object_get_string(content, "actName");
		if (json_object_has_value(content, "cmd"))
			data.cmd = json_object_get_string(content, "cmd");

		JSON_Array *argsArray = nullptr;
		if (json_object_has_value(content, "args"))
			argsArray = json_object_get_array(content, "args");

		data.args = "";
		if (argsArray && json_array_get_count(argsArray) > 0)
		{
			for (int j = 0; j < json_array_get_count(argsArray); ++j)
			{
				data.args += json_array_get_string(argsArray, j);
				data.args += " ";
			}
			data.args.erase(data.args.length() - 1);
		}
		actVector.emplace_back(data);
	}

	Dispatch(actVector);
}

void ProcessControl::Dispatch(std::vector<ActionData> &actVector)
{
	if (actVector.size() > 0)
	{
		QMenu *procMenu = _manager->FindProcMenu(actVector[0].appName);
		if (procMenu == nullptr)
		{
			QString name = QString::fromStdString(actVector[0].appName);//productList[actVector[0].proc];
			procMenu = _manager->CreateMenu(name);
			_manager->AddProcMenu(actVector[0].appName, procMenu);
		}

		for (auto &item : actVector)
		{
			if (item.cmdCode == MSG_PROCESS_CONTROL_REGISTER ||
				item.cmdCode == MSG_PROCESS_MONITOR_REGISTER)
				Register(procMenu, item);

			else if (item.cmdCode == MSG_PROCESS_CONTROL_UNREGISTER ||
				item.cmdCode == MSG_PROCESS_MONITOR_UNREGISTER)
			{
				item.cmdCode -= 1;
				UnRegister(procMenu, item);
			}
			else if (item.cmdCode == MSG_LOGOUT_CLEAN_DEAL)
				ClearAppMenus(procMenu, item);
		}
	}
}

void ProcessControl::Register(QMenu *procMenu, const ActionData &data)
{
	if (IsExistAction(data)) return;

	QString strMenus = QString::fromStdString(data.parentMenus);
	QStringList parentMenus = strMenus.split('/');
	QMenu *parentMenu = procMenu;
	QMenu *rootParentMenu = nullptr;
	QMenu *lastChildMenu = nullptr;

	QString currentPath;
	for (int i = 0; i < parentMenus.size(); ++i)
	{
		currentPath += parentMenus[i];
		if (currentPath.isEmpty()) continue;
		ActionData tmpData = {};
		tmpData.appName = data.appName;
		tmpData.appId = data.appId;
		tmpData.parentMenus = currentPath.toLocal8Bit().data();
		currentPath += '/';

		if (isExistMenu(tmpData))
		{
			lastChildMenu = _mapCmdMenu[tmpData];
			parentMenu = lastChildMenu;
		}
		else
		{
			QMenu* tmpMenu = _manager->CreateMenu(parentMenus[i], parentMenu);
			if (lastChildMenu != nullptr)
				lastChildMenu->addMenu(tmpMenu);
			lastChildMenu = tmpMenu;
			parentMenu = lastChildMenu;
			_mapCmdMenu.emplace(tmpData, lastChildMenu);
		}
		if (0 == i)
			rootParentMenu = lastChildMenu;
	}

	if (data.cmdCode == MSG_PROCESS_CONTROL_REGISTER)
	{
		DataQAction *actionProc = _manager->CreateAction(data.actionName.c_str(), parentMenu);
		actionProc->SetActionData(data);
		connect(actionProc, SIGNAL(triggered()), actionProc, SLOT(OnProcessOpened()));

		parentMenu->addAction(actionProc);
		_mapProcAction.emplace(data, actionProc);
	}
	else if (data.cmdCode == MSG_PROCESS_MONITOR_REGISTER)
	{
		QString strActionName = QString::fromUtf8("启动");
		QMenu *cmdMenu = _manager->CreateMenu(data.actionName.c_str(), parentMenu);
		DataQAction *actionOpen = _manager->CreateAction(strActionName, cmdMenu);
		actionOpen->SetActionData(data);
		connect(actionOpen, SIGNAL(triggered()), actionOpen, SLOT(OnProcessOpened()));

		DataQAction *actionClose = _manager->CreateAction(QString::fromUtf8("关闭"));
		actionClose->SetActionData(data);
		connect(actionClose, SIGNAL(triggered()), actionClose, SLOT(OnProcessClosed()));

		cmdMenu->addAction(actionOpen);
		cmdMenu->addAction(actionClose);
		_mapProcAction.emplace(data, actionOpen);
		_mapProcAction.emplace(data, actionClose);
		_mapCmdMenu.emplace(data, cmdMenu);

		parentMenu->addMenu(cmdMenu);
	}

	if (rootParentMenu != nullptr)
		procMenu->addMenu(rootParentMenu);
}

void ProcessControl::UnRegister(QMenu *procMenu, const ActionData &data)
{
	auto iter = _mapCmdMenu.find(data);
	if (iter != _mapCmdMenu.end())
	{
		QMenu *serviceMenu = iter->second;
		int count = _mapProcAction.count(data);
		for (int i = 0; i < count; i++)
		{
			auto iter = _mapProcAction.find(data);
			serviceMenu->removeAction(iter->second);
			_mapProcAction.erase(iter);
		}
		QMenu *parentMenu = dynamic_cast<QMenu *>(serviceMenu->parent());
		parentMenu->removeAction(serviceMenu->menuAction());
		_mapCmdMenu.erase(data);
		serviceMenu->deleteLater();
	}

	if (procMenu->isEmpty())
		_manager->RemoveProcMenu(data.appName, procMenu);
	RemoveEmptyMenus(procMenu, data);
}

void ProcessControl::RemoveEmptyMenus(QMenu *procMenu, const ActionData &data)
{
	QString strMenus = QString::fromStdString(data.parentMenus);
	QStringList parentMenus = strMenus.split('/');

	for (int i = parentMenus.size() - 1; i >= 0; --i)
	{
		if (strMenus.isEmpty()) return;

		ActionData tmpData = {};
		tmpData.appName = data.appName;
		tmpData.appId = data.appId;
		tmpData.parentMenus = strMenus.toLocal8Bit().data();

		auto iter = _mapCmdMenu.find(tmpData);
		if (iter != _mapCmdMenu.end())
		{
			QMenu *currentMenu = iter->second;
			if (currentMenu->isEmpty())
			{
				QMenu *parentMenu = dynamic_cast<QMenu *>(currentMenu->parent());
				parentMenu->removeAction(currentMenu->menuAction());
				_mapCmdMenu.erase(tmpData);
				if (!parentMenu->isEmpty()) break;
			}
			
		}
		int nLen = parentMenus[i].length();
		strMenus = strMenus.left(strMenus.length() - nLen - 1);
	}

	if (procMenu->isEmpty())
		_manager->RemoveProcMenu(data.appName, procMenu);
}

void ProcessControl::ClearAppMenus(QMenu *procMenu, const ActionData &data)
{
	auto iterPro = _mapProcAction.begin();
	while (iterPro != _mapProcAction.end())
	{
		ActionData tmpData = iterPro->first;
		if (!tmpData.appName.compare(data.appName) &&
			!tmpData.appId.compare(data.appId))
		{
			procMenu->removeAction(iterPro->second);
			iterPro = _mapProcAction.erase(iterPro);
		}
		else ++iterPro;
	}

	auto iterSer = _mapCmdMenu.begin();
	while (iterSer != _mapCmdMenu.end())
	{
		ActionData tmpData = iterSer->first;
		if (!tmpData.appName.compare(data.appName) &&
			!tmpData.appId.compare(data.appId))
		{
			procMenu->removeAction(iterSer->second->menuAction());
			iterSer->second->clear();
			iterSer = _mapCmdMenu.erase(iterSer);
		}
			
		else ++iterSer;
	}

	if (procMenu->isEmpty())
	{
		procMenu->clear();
		_manager->RemoveProcMenu(data.appName, procMenu);
	}
}

bool ProcessControl::IsExistAction(const ActionData &data)
{
	return _mapProcAction.find(data) != _mapProcAction.end();
}

bool ProcessControl::isExistMenu(const ActionData &data)
{
	return _mapCmdMenu.find(data) != _mapCmdMenu.end();
}
