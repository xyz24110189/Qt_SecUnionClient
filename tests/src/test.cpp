#include <stdio.h>
#include <string.h>
#include <string>
#include <libUnionAgent.h>

void __stdcall onServerMsg(const char *fromAppName, const char *buf, int bufLen, void *param)
{
	LOGINDATA *data = static_cast<LOGINDATA*>(param);
	fprintf(stdout, "onServerMsg operation fromAppName = %s, msg = %s, buflen = %d\n", fromAppName, buf, bufLen);
}

int main(int argc, const char *argv[])
{
	LOGINDATA data = {"NDS客户端", "9853246CE7E042999E379B2B66682F4B", ""};

	bool bRet = UniSecInitClient(&data, onServerMsg, (void *)&data, Encoding::UTF8); //vs2015 以下对utf8编码支持不好，建议用gb2312 编码格式
	int retCode = ErrCode::ERR_SUCCESS;
	if (!bRet)
	{
		retCode = UniSecGetErrno();
		fprintf(stdout, "UniSecInitClient operation error = %d\n", retCode);
		return -1;
	}

	MENUDATA proData1 = { 0, "", "打开 setting", "D:/Program Files/feiq/Recv Files/setting.exe", \
		"/home/songyanqing/work/sec-union-client/tests/src/test.cpp" };
	MENUDATA proData2 = { 0, "", "打开 Depends2", "C:/Program Files (x86)/Feitian/测试目录/Depends.exe", \
		"C:/Users/GERJ/Desktop/pkiAgent4c.dll" };
	MENUDATA proData3 = { 0, "一级菜单/二级菜单/三级菜单/四级菜单", "打开 Depends3", "C:/Users/GERJ/Desktop/Depends.exe", \
		"C:/Users/GERJ/Desktop/pkiAgent4c.dll" };
	MENUDATA proData4 = { 0, "一级菜单/二级菜单/三级菜单/四级菜单", "打开 Depends4", "C:/Users/GERJ/Desktop/Depends.exe", \
		"C:/Users/GERJ/Desktop/pkiAgent4c.dll" };
	MENUDATA proData5 = { 0, "一级菜单/二级菜单/三级菜单/四级菜单", "打开 Depends5", "C:/Users/GERJ/Desktop/Depends.exe", \
		"C:/Users/GERJ/Desktop/pkiAgent4c.dll" };	

	bRet = UniSecOpTrayMenu(&proData1, true);
	bRet = UniSecOpTrayMenu(&proData2, true);
	bRet = UniSecOpTrayMenu(&proData3, true);
	bRet = UniSecOpTrayMenu(&proData4, true);
	bRet = UniSecOpTrayMenu(&proData5, true);
	if (!bRet)
	{
		retCode = UniSecGetErrno();
		fprintf(stdout, "UniSecOpTrayMenu operation error = %d\n", retCode);
		return -2;
	}

	bRet = UniSecRegMonitor(&proData1, true);
	if (!bRet)
	{
		retCode = UniSecGetErrno();
		fprintf(stdout, "UniSecRegMonitor operation error = %d\n", retCode);
		return -3;
	}
	

	PROGRAMDATA programData = {
		"D:/Program Files/Notepad++/notepad++.exe",
		"D:/Qt/Qt5.6.3/5.6.3/Src/qtdoc/doc/src/snippets/gs/notepad2.cpp",
		ProgramOper::START
	};
	bRet = UniSecOperProgram(&programData);
	if (!bRet)
	{
		retCode = UniSecGetErrno();
		fprintf(stdout, "UniSecOperProgram operation error = %d\n", retCode);
		return -4;
	}

	char *msg = "NACHEHEHEHEH!!!!!!";
	bRet = UniSecSendMsgToApp("NAC 客户端", msg, strlen(msg));
	if (!bRet)
	{
		retCode = UniSecGetErrno();
		fprintf(stdout, "UniSecSendMsgToApp operation error = %d\n", retCode);
		return -5;
	}

	TRAYICONS icons = {
		"C:/Users/GERJ/Downloads/lion.jpg",
		"格尔身份鉴别系统客户端！"
	};
	bRet = UniSecSetTrayIcon(&icons);
	if (!bRet)
	{
		retCode = UniSecGetErrno();
		fprintf(stdout, "UniSecSetTrayIcon operation error = %d\n", retCode);
		return -6;
	}

	TRAYTIPS trayTips = {
		"生活不易，且行且珍惜",
		10
	};
	bRet = UniSecShowTrayTips(&trayTips);
	if (!bRet)
	{
		retCode = UniSecGetErrno();
		fprintf(stdout, "UniSecShowTrayTips operation error = %d\n", retCode);
		return -7;
	}

	getchar();
	bRet = UniSecUnInitClient();
	if (!bRet)
	{
		retCode = UniSecGetErrno();
		fprintf(stdout, "UniSecUnInitClient operation error = %d\n", retCode);
		return -8;
	}
	return 0;
}
