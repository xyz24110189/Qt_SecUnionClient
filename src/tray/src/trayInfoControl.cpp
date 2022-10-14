#include "trayInfoControl.h"
#include <QtCore/QDir>
#include <QtCore/QString>
#include "uniManagerTray.h"
#include "client.h"
#include <utils.h>

TrayInfoControl::TrayInfoControl(QObject *parent) : QObject(parent)
{
}

TrayInfoControl::~TrayInfoControl()
{
}

void TrayInfoControl::Resolve(const JSON_Object *jData, UniManagerTray *manager)
{
	_manager = manager;
	BaseData data;
	if (json_object_has_value(jData, "cmdCode"))
		data.cmdCode = static_cast<int32_t>(json_object_get_number(jData, "cmdCode"));

	if (data.cmdCode == MSG_CHANGE_TRAYICONS_INFO)
		ChangeTrayIconsInfo(jData);
	else if (data.cmdCode == MSG_SHOW_TRAY_TIPS)
		ShowTrayTips(jData);
	else if (data.cmdCode == MSG_LOGOUT_CLEAN_DEAL)
		ClearTrayIcon(jData);
}

void TrayInfoControl::ChangeTrayIconsInfo(const JSON_Object *jData)
{
	TrayIcons data;
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
		if (json_object_has_value(content, "iconPath"))
			data.iconPath = json_object_get_string(content, "iconPath");
		if (json_object_has_value(content, "holdTips"))
			data.holdTips = json_object_get_string(content, "holdTips");
	}
	_trayIcons = data;
	_manager->ChangeTrayIconsInfo(data.iconPath, data.holdTips);
}

void TrayInfoControl::ShowTrayTips(const JSON_Object *jData)
{
	TrayTips data; 
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
		if (json_object_has_value(content, "tips"))
			data.tips = json_object_get_string(content, "tips");
		if (json_object_has_value(content, "holdTime"))
			data.holdTime = static_cast<int>(json_object_get_number(content, "holdTime"));
	}
	_manager->ShowTips(data.tips, data.holdTime);
}

void TrayInfoControl::ClearTrayIcon(const JSON_Object *jData)
{
	TrayIcons data;
	if (json_object_has_value(jData, "appName"))
		data.appName = json_object_get_string(jData, "appName");
	if (json_object_has_value(jData, "appId"))
		data.appId = json_object_get_string(jData, "appId");
	if (json_object_has_value(jData, "cmdCode"))
		data.cmdCode = static_cast<int32_t>(json_object_get_number(jData, "cmdCode"));

	if (0 == data.appId.compare(_trayIcons.appId))
		_manager->RestoreTrayIconInfo();
}

