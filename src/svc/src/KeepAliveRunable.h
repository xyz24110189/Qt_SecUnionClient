#pragma once
#include <QRunnable> 
#include <QQueue>
#include "commonDef.h"

class KeepAliveRunable : public QRunnable
{
public:
	KeepAliveRunable();
	~KeepAliveRunable();
	
	void run() override;

	void shutDown() { _shutDown = true; };
	void deleteAppData(const BaseData &data);
	void handleMonitorObject(const ActionData &actionData);
	void addMonitorObject(const ActionData &actionData);
	void removeMonitorObject(const ActionData &actionData);

protected:
	void startServiceMonitor();
	void endServiceMonitor();

private:
	bool _shutDown;
	QQueue<ActionData> _queueData;
};

