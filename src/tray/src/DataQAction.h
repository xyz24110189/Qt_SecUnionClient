#pragma once
#include <QString>
#include <QtGui/QAction>
#include <commonDef.h>

class DataQAction : public QAction
{
	Q_OBJECT
public:
	DataQAction(QObject *parent = NULL);
	DataQAction(const QString &text, QObject *parent = NULL);
	~DataQAction();

	void SetActionData(const ActionData &data);
	void SetServiceData(const SeviceData &data);

public slots:
	void OnProcessOpened();
	void OnProcessClosed();
	void OnServiceStart();
	void OnServiceStop();

private:
	std::string utf8ToGb2312(const char *strUtf8);

private:
	ActionData m_actionData;
	SeviceData m_serviceData;
};

