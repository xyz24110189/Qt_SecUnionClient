#pragma once
#include <qtservice/qtservice.h>
#include <memory>
#include <QtCore/QObject>
#include "uniqueMrDatabase.h"

class KeepAliveRunable;
class ProgramOperatorRunable;
class InteractiveService : public QObject, public QtService<QCoreApplication>
{
	Q_OBJECT
public:
	InteractiveService(int argc, char **argv);
	~InteractiveService();

	void ParseClientData(unsigned short msg, const std::string &jData);
	void OnLogin(void *pClient);
	void OnLoginOut(void *pClient);

protected:
	void start();
	void stop();
	void pause();
	void resume();
	void processCommand(int code);

private:
	void ParseData(const std::string &jData);
	void ParseInnerData(const std::string &jData);

	UniqueMrDatabase _db;
	std::shared_ptr<KeepAliveRunable> _keepRunable;
	std::shared_ptr<ProgramOperatorRunable> _programRunable;
};

