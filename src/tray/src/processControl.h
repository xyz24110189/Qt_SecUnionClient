#pragma once
#include "commonDef.h"
#include <map>
#include <vector>
#include <QtCore/QObject>

class UniManagerTray;
class QAction;
class QMenu;

class ProcessControl : public QObject
{
	Q_OBJECT
public:
	ProcessControl(QObject *parent = nullptr);
	~ProcessControl();

	void Resolve(const JSON_Object *jData, UniManagerTray *manager);

private:
	void Dispatch(std::vector<ActionData> &actVector);
	void Register(QMenu *procMenu, const ActionData &data);
	void UnRegister(QMenu *procMenu, const ActionData &data);


	void RemoveEmptyMenus(QMenu *procMenu, const ActionData &data);
	void ClearAppMenus(QMenu *procMenu, const ActionData &data);
	bool IsExistAction(const ActionData &data);
	bool isExistMenu(const ActionData &data);

	std::map<ActionData, QMenu *> _mapCmdMenu;
	std::multimap<ActionData, QAction *> _mapProcAction;
	UniManagerTray *_manager;
};

