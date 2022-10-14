#pragma once
#include "commonDef.h"
#include <vector>
#include <map>
#include <QtCore/QObject>

class UniManagerTray;
class QMenu;
class QAction;

class ServiceControl : public QObject
{
	Q_OBJECT
public:
	ServiceControl(QObject *parent = nullptr);
	~ServiceControl();

	void Resolve(const JSON_Object *jData, UniManagerTray *manager);

private:
	void Dispatch(std::vector<SeviceData> &actVector);
	void Register(QMenu *procMenu, const SeviceData &data);
	void UnRegister(QMenu *procMenu, const SeviceData &data);

	bool IsExistAction(const SeviceData &data);

	std::map<SeviceData, QMenu *> _mapServiceMenu;
	std::multimap<SeviceData, QAction *> _mapProcAction;
	UniManagerTray *_manager;
};

