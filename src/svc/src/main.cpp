#include "commonDef.h"
#include "InteractiveService.h"
#include <tools.h>
#include <QtCore/QDir>

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	// 获取日志和数据库目录
	std::string appPath = koal::tool::getAppDataPath();
	std::string logPath = appPath + "/logs";
	std::string dbPath = appPath + "/database";

	// 创建目录
	QDir dir;
	dir.mkpath(logPath.c_str());
	dir.mkpath(dbPath.c_str());

	// 初始化日志库
	logPath += "/myeasylogSvc.log";
	logPath = QDir::toNativeSeparators(logPath.c_str()).toLocal8Bit().data();
	plog::init(plog::debug, logPath.c_str(), 10000000, 10);


	InteractiveService a(argc, argv);
	return a.exec();
}
