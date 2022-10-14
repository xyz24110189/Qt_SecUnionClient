#include "commonDef.h"
#include "uniManagerTray.h"
#include "client.h"
#include <utils.h>
#include <tools.h>
#include <QtGui/QApplication>
#include <QtCore/QTextCodec>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <singleapplication.h>

//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

void setAppStyleSheet()
{
	QFile qssFile(":/uniManagerTray/KUniSecService.qss");
	if (!qssFile.open(QIODevice::ReadOnly)) return;
	qApp->setStyleSheet(qssFile.readAll());
}

int main(int argc, char *argv[])
{
	SingleApplication a(argc, argv);

	if (!checkIsOne()) return -1;
	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	QTextCodec::setCodecForLocale(codec);
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForTr(codec);
	//设置样式
	setAppStyleSheet();

	//日志初始化
	std::string logPath = koal::tool::getAppDataPath();
	logPath += "/logs";
	QDir dir;
	dir.mkpath(logPath.c_str());
	logPath += "/myeasylogCli.log";
	logPath = QDir::toNativeSeparators(logPath.c_str()).toLocal8Bit().data();
	plog::init(plog::debug, logPath.c_str(), 10000000, 10);

	UniManagerTray w;
	w.Show();

	std::function<void(const std::string &)> msgCallFun = 
		std::bind(&UniManagerTray::OnReceiveData, &w, std::placeholders::_1);

	std::function<void(bool)> reLoginCallFun =
		std::bind(&UniManagerTray::OnRelogin, &w, std::placeholders::_1);

	ClientPipe *inst = ClientPipe::instance();
	inst->msgCallBack(msgCallFun);
	inst->reLoginCallBack(reLoginCallFun);
	inst->init();

	return a.exec();
}
