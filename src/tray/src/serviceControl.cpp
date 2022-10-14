#include "serviceControl.h"
#include <QtGui/QMenu>
#include <QtGui/QAction>

#include "DataQAction.h"
#include "uniManagerTray.h"
#include "client.h"


ServiceControl::ServiceControl(QObject *parent) : QObject(parent)
{
}


ServiceControl::~ServiceControl()
{
}

void ServiceControl::Resolve(const JSON_Object *jData, UniManagerTray *manager)
{
	_manager = manager;

	std::vector<SeviceData> actVector;
	SeviceData data;
	if (json_object_has_value(jData, "appName"))
		data.appName = json_object_get_string(jData, "appName");
	if (json_object_has_value(jData, "appId"))
		data.appId = json_object_get_string(jData, "appId");
	if (json_object_has_value(jData, "cmdCode"))
		data.cmdCode = static_cast<int32_t>(json_object_get_number(jData, "cmdCode"));

	JSON_Array *content = nullptr;
	if (json_object_has_value(jData, "content"))
		content = json_object_get_array(jData, "content");

	if (content)
	{
		for (int i = 0; i < json_array_get_count(content); ++i)
		{
			JSON_Object *item = json_array_get_object(content, i);
			if (!item) continue;

			if (json_object_has_value(item, "priority"))
				data.priority = static_cast<unsigned short>(json_object_get_number(item, "priority"));
			if (json_object_has_value(item, "parentMenus"))
				data.parentMenus = json_object_get_string(item, "parentMenus");
			if (json_object_has_value(item, "actName"))
				data.actionName = json_object_get_string(item, "actName");
			if (json_object_has_value(item, "cmd"))
				data.serverName = json_object_get_string(item, "cmd");

			JSON_Array *argsArray = nullptr;
			if (json_object_has_value(item, "args"))
				argsArray = json_object_get_array(item, "args");

			data.args = "";
			if (argsArray && json_array_get_count(argsArray) > 0)
			{
				for (int j = 0; j < json_array_get_count(argsArray); ++j)
				{
					data.args += json_array_get_string(argsArray, j);
					data.args += '&';
				}
				data.args.erase(data.args.length() - 1);
			}
			actVector.emplace_back(data);
		}
	}

	Dispatch(actVector);
}


void ServiceControl::Dispatch(std::vector<SeviceData> &actVector)
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
			if (item.cmdCode == MSG_SERVICE_CONTROL_REGISTER)
				Register(procMenu, item);

			else if (item.cmdCode == MSG_SERVICE_CONTROL_UNREGISTER)
			{
				item.cmdCode -= 1;
				UnRegister(procMenu, item);
			}
		}
	}
}

void ServiceControl::Register(QMenu *procMenu, const SeviceData &data)
{
	if (IsExistAction(data)) return;

	QMenu *serviceMenu = _manager->CreateMenu(data.actionName.c_str());
	DataQAction *actionStart = _manager->CreateAction(QString::fromUtf8("启动"));
	actionStart->SetServiceData(data);
	connect(actionStart, SIGNAL(triggered()), actionStart, SLOT(OnServiceStart()));

	DataQAction *actionStop = _manager->CreateAction(QString::fromUtf8("停止"));
	actionStop->SetServiceData(data);
	connect(actionStop, SIGNAL(triggered()), actionStop, SLOT(OnServiceStop()));

	serviceMenu->addAction(actionStart);
	serviceMenu->addAction(actionStop);
	_mapProcAction.emplace(data, actionStart);
	_mapProcAction.emplace(data, actionStop);

	procMenu->addMenu(serviceMenu);
	_mapServiceMenu.emplace(data, serviceMenu);
}

void ServiceControl::UnRegister(QMenu *procMenu, const SeviceData &data)
{
	auto iter = _mapServiceMenu.find(data);
	if (iter != _mapServiceMenu.end())
	{
		QMenu *serviceMenu = iter->second;
		int count = _mapProcAction.count(data);
		for (int i = 0; i < count; i++)
		{
			auto iter = _mapProcAction.find(data);
			serviceMenu->removeAction(iter->second);
			_mapProcAction.erase(iter);
		}
		procMenu->removeAction(serviceMenu->menuAction());
	}

	if (procMenu->isEmpty())
		_manager->RemoveProcMenu(data.appName, procMenu);
}

bool ServiceControl::IsExistAction(const SeviceData &data)
{
	return _mapProcAction.count(data);
}