#pragma once
#include "commonDef.h"
#include <QtCore/QObject>

class UniManagerTray;
class TrayInfoControl : public QObject
{
	Q_OBJECT
public:
	TrayInfoControl(QObject *parent = nullptr);
	~TrayInfoControl();

	void Resolve(const JSON_Object *jData, UniManagerTray *manager);

private:
	void ChangeTrayIconsInfo(const JSON_Object *jData);
	void ShowTrayTips(const JSON_Object *jData);
	void ClearTrayIcon(const JSON_Object *jData);
	//void changeSetToolTipInfo(const )

	UniManagerTray *_manager;
	TrayIcons _trayIcons;
};

