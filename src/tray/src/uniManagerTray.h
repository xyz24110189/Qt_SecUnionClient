#pragma once
#include "commonDef.h"
#include <QtGui/QWidget>
#include <map>
#include <vector>
#include <string>

class QSystemTrayIcon;
class QAction;
class DataQAction;
class QMenu;
struct ActionData;

class UniManagerTray : public QWidget
{
	Q_OBJECT
public:
	UniManagerTray(QWidget *parent = NULL);

	void Show();
	void HandleActionDynamic(const std::string &strJson);
	void RestoreTrayIconInfo();
	void ChangeTrayIconsInfo(const std::string &iconPath, 
		const std::string &tips);
	void ShowTips(const std::string &tips, int holdTime);
	void SetTrayIco(bool status);

	void AddProcMenu(const std::string &appName, QMenu *menu);
	void RemoveProcMenu(const std::string &appName, QMenu *menu);
	QMenu* FindProcMenu(const std::string &appName);
	QMenu* CreateMenu(const QString &name, QWidget *parent = nullptr);
	DataQAction* CreateAction(const QString &name, QWidget *parent = nullptr);



	void OnReceiveData(const std::string &data);
	void OnRelogin(bool loginStatus);

signals:
	void MsgReveive(const std::string &data);
	void Relogin(bool loginStatus);

public slots:
	//void OnReceiveData(const QString &data);
	void OnThreadSecRecv(const std::string &data);
	void OnThreadSecRelogin(bool loginStatus);

private:
	void InitConnection();
	void InitAction();
	void CreateTrayIcon();
	bool ParseJsonData(const JSON_Object *obj);

	QMenu *_trayIconMenu;
	QAction *_quiteAction;
	QAction *_lastAction;

#ifdef TEST_MONITOR_MARCO
	QAction *_monitorAction;
#endif
	
	QSystemTrayIcon *_trayIcon;
	
	std::map<std::string, QMenu *> _mapProcMenu;
	std::map<MsgCode, void *> _mapActProcess;
};
