#include "utils.h"
#include "tools.h"
#include "parson.h"
#include <commonDef.h>
#include <fstream>

#if defined _WIN32
#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#elif defined __linux__
#include <stdlib.h>  
#include <stdio.h>
#endif

#define REG_PORT_PATH  "SOFTWARE\\Koal\\Middleware\\KCliBaseService"
#define LINUX_CFG_PATH "/etc/koal/mw/KCliBaseService/KCliBaseService.cfg"

bool getAppIdByProType(ProcType type, std::string &strAppId)
{
	static const char *appIds[] = {
		"",
		"856A3F47900E42C090AB906C1893D25A",
		"3144FD603B444A76AF88503989A8BF01",
		"9853246CE7E042999E379B2B66682F4A",
		"DE437B9014604F3EB129E13978EDFAE8"
	};
	strAppId = appIds[type];
	return !strAppId.empty();
}


bool isRunning(const std::string &procName)
{
	//LOG_INFO << "isRunning check begine...";
#if defined _WIN32
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processesSnapshot == INVALID_HANDLE_VALUE)
		return false;

	Process32First(processesSnapshot, &processInfo);
	if (!procName.compare(processInfo.szExeFile))
	{
		CloseHandle(processesSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32Next(processesSnapshot, &processInfo))
	{
		if (!procName.compare(processInfo.szExeFile))
		{
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}
	}

	LOG_INFO << "isRunning check... process = " << procName << "is not running!";
	CloseHandle(processesSnapshot);
	return false;
#elif defined __linux__
	char command[64] = { 0 };
	sprintf(command, "pgrep %s > /dev/null", procName.c_str());
	return 0 == system(command);
#endif//_WIN32
	LOG_INFO << "KeepAliveRunable::isRunning end...";
}

bool checkIsOne()
{
#ifdef _WIN32
	HANDLE m_hMutex = CreateMutex(NULL, FALSE, BINARY_TRAY);
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
		return false;
	}
#endif //_WIN32
	return true;
}

bool killProcess(const std::string &procName)
{
#if defined _WIN32
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);
	BOOL hRes = Process32First(hSnapShot, &pEntry);
	while (hRes)
	{
		if (strcmp(pEntry.szExeFile, procName.c_str()) == 0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
				(DWORD)pEntry.th32ProcessID);
			if (hProcess != NULL)
			{
				TerminateProcess(hProcess, 9);
				CloseHandle(hProcess);
			}
		}
		hRes = Process32Next(hSnapShot, &pEntry);
	}
	CloseHandle(hSnapShot);
#elif defined __linux__
	char command[64] = { 0 };
	sprintf(command, "kill -9 `pidof %s`", procName.c_str());
	return 0 == system(command);
#endif//_WIN32
	return true;
}

#ifdef _WIN32
#include <UserEnv.h>
#include <WtsApi32.h>
#pragma comment(lib, "UserEnv.lib")
#pragma comment(lib, "WtsApi32.lib")

bool StartProcessAsHighPrivilege(LPCSTR proPath, LPCSTR args)
{
	// Launch process as administrator. 
	SHELLEXECUTEINFO sei = { sizeof(sei) };
	sei.lpVerb = "runas";
	sei.lpFile = proPath;
	sei.lpParameters = args;
	sei.hwnd = NULL;
	sei.nShow = SW_NORMAL;

	if (!ShellExecuteEx(&sei))
	{
		DWORD dwError = GetLastError();
		LOG_INFO << "StartProcessAsHighPrivilege  ERROR = " << dwError;
		return false;
	}
	else
		LOG_INFO << "StartProcessAsHighPrivilege  Success";
	return true;
}

BOOL GetTokenByName(HANDLE &hToken, LPSTR lpName)
{
	LOG_INFO << "GetTokenByName begin...";
	if (!lpName)
	{
		return FALSE;
	}
	HANDLE hProcessSnap = NULL;
	BOOL bRet = FALSE;
	PROCESSENTRY32 pe32 = { 0 };

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
		return (FALSE);

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hProcessSnap, &pe32))
	{
		do
		{
			if (!strcmp(_strupr(pe32.szExeFile), _strupr(lpName)))
			{
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,
					FALSE, pe32.th32ProcessID);
				bRet = OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken);
				CloseHandle(hProcessSnap);
				LOG_INFO << "GetTokenByName success!";
				return (bRet);
			}
		} while (Process32Next(hProcessSnap, &pe32));
		bRet = TRUE;
	}
	else
		bRet = FALSE;

	CloseHandle(hProcessSnap);
	LOG_INFO << "GetTokenByName end...";
	return (bRet);
}

int CreateProcessAsUserForXp(LPSTR lpCommandLine, bool showWindow)
{
	DWORD pId = 0;// result

	LUID luid; // local uniq id for process

	TOKEN_PRIVILEGES NewState = { 0 };
	TOKEN_PRIVILEGES PreviousState = { 0 };

	HANDLE phToken = NULL;
	HANDLE phNewToken = NULL;

	STARTUPINFO si = { 0 };
	si.cb = sizeof(STARTUPINFO);
	si.lpDesktop = TEXT("WinSta0\\Default");// current user desktop

	LPVOID lpEnvironment = NULL;
	PROCESS_INFORMATION pi = { 0 };

	DWORD ReturnLength;
	if (!GetTokenByName(phToken, "EXPLORER.EXE"))
	{
		DWORD dwErr = GetLastError();
		LOG_INFO << "GetTokenByName  ERROR = " << dwErr;
		return -1;
	}

	if (!DuplicateTokenEx(phToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &phNewToken))
	{
		DWORD dwErr = GetLastError();
		LOG_INFO << "DuplicateTokenEx ERROR = " << dwErr;
		return -1;
	}

	if (!LookupPrivilegeValue(NULL, TEXT("SeTcbPrivilege"), &luid))
	{
		DWORD dwErr = GetLastError();
		LOG_INFO << "LookupPrivilegeValue  SeTcbPrivilege ERROR = " << dwErr;
		return -1;
	}

	NewState.PrivilegeCount = 1;
	NewState.Privileges[0].Luid = luid;
	NewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(phNewToken, FALSE, &NewState, sizeof(TOKEN_PRIVILEGES), &PreviousState, &ReturnLength))// change proc privileges to user
	{
		DWORD dwErr = GetLastError();
		LOG_INFO << "AdjustTokenPrivileges ERROR = " << dwErr;
		return -1;
	}

	if (!CreateEnvironmentBlock(&lpEnvironment, phNewToken, TRUE))
	{
		DWORD dwErr = GetLastError();
		LOG_INFO << "CreateEnvironmentBlock ERROR = " << dwErr;
		return -1;
	}

	DWORD createFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT;
	if (!showWindow)
		createFlags = NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT;
	
	if (!CreateProcessAsUser(phNewToken, NULL, lpCommandLine, NULL, NULL, FALSE, createFlags, lpEnvironment, NULL, &si, &pi))// for hollowing CREATE_SUSPENDED | CREATE_NO_WINDOW 
	{
		DWORD dwErr = GetLastError();
		LOG_INFO << "CreateProcessAsUser error: " << dwErr;
		return -1;
	}

	pId = pi.dwProcessId;

	AdjustTokenPrivileges(phNewToken, FALSE, &PreviousState, sizeof(TOKEN_PRIVILEGES), NULL, NULL);// return proc privileges to system
	DestroyEnvironmentBlock(lpEnvironment);

	// clear memory
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	CloseHandle(phToken);
	CloseHandle(phNewToken);

	return pId;
}

int CreateProcessAsUserForSystem(LPSTR lpCommandLine)
{
	LPCTSTR cmdDir;
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(STARTUPINFO);
	si.lpDesktop = TEXT("WinSta0\\Default");// current user desktop
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	HANDLE token;
	LUID luid; // local uniq id for process

	DWORD sessionId = WTSGetActiveConsoleSessionId();
	if (sessionId == 0xffffffff)  // Noone is logged-in
		return false;
	// This only works if the current user is the system-account (we are probably a Windows-Service)
	HANDLE dummy;
	TOKEN_PRIVILEGES tp;
	HANDLE hPToken = NULL;
	HANDLE hProcess = NULL;
	DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
	if (WTSQueryUserToken(sessionId, &dummy)) {
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY
			| TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_SESSIONID
			| TOKEN_READ | TOKEN_WRITE, &hPToken))
		{
			return false;
		}
		if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
		{
			return false;
		}
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		if (!DuplicateTokenEx(hPToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &token)) {
			return false;
		}

		if (!SetTokenInformation(token, TokenSessionId, (void*)&sessionId, sizeof(DWORD)))
		{
			return false;
		}
		if (!AdjustTokenPrivileges(token, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, NULL))
		{
			return false;
		}
		LPVOID pEnv = NULL;
		if (CreateEnvironmentBlock(&pEnv, token, TRUE))
		{
			dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
		}
		else pEnv = NULL;
		// Create process for user with desktop
		if (!CreateProcessAsUser(token, NULL, lpCommandLine, NULL, NULL, FALSE, dwCreationFlags, pEnv, NULL, &si, &pi)) {  // The "new console" is necessary. Otherwise the process can hang our main process
			CloseHandle(token);
			return false;
		}
		CloseHandle(dummy);
		CloseHandle(token);
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return true;
}

DWORD GetCurrentSessionId()
{
	WTS_SESSION_INFO *pSessionInfo;
	DWORD n_sessions = 0;
	BOOL ok = WTSEnumerateSessions(WTS_CURRENT_SERVER, 0, 1, &pSessionInfo, &n_sessions);
	if (!ok)
		return 0;

	DWORD SessionId = 0;

	for (DWORD i = 0; i < n_sessions; ++i)
	{
		if (pSessionInfo[i].State == WTSActive)
		{
			SessionId = pSessionInfo[i].SessionId;
			break;
		}
	}

	WTSFreeMemory(pSessionInfo);
	return SessionId;
}

bool CreateProcessAsUserForCustom(LPSTR process_path)
{
	DWORD SessionId = GetCurrentSessionId();
	if (SessionId == 0)    // no-one logged in
		return false;

	HANDLE hToken;
	BOOL ok = WTSQueryUserToken(SessionId, &hToken);
	if (!ok)
		return false;

	void *environment = NULL;
	ok = CreateEnvironmentBlock(&environment, hToken, TRUE);

	if (!ok)
	{
		CloseHandle(hToken);
		return false;
	}

	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi = {};
	si.lpDesktop = "winsta0\\default";

	// Do NOT want to inherit handles here
	DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT;
	ok = CreateProcessAsUser(hToken, NULL, (LPSTR)process_path, NULL, NULL, FALSE,
		dwCreationFlags, environment, NULL, &si, &pi);

	DestroyEnvironmentBlock(environment);
	CloseHandle(hToken);

	if (!ok)
		return false;

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return true;
}

bool isGreaterXp()
{
	OSVERSIONINFO osvi;
	BOOL bIsWindowsXPLater;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&osvi);

	bIsWindowsXPLater = (osvi.dwMajorVersion > 5);

	return bIsWindowsXPLater;
}

// 将正在使用的端口设置到注册表
long setCurrentChannelIntoRegister(HKEY hRootKey, LPCTSTR lpKeyName, LPCTSTR lpValueName, LPCTSTR lpValue)
{
	HKEY hKey = nullptr;

	long result = RegOpenKeyEx(hRootKey, lpKeyName, 0, KEY_ALL_ACCESS, &hKey);
	if (ERROR_SUCCESS != result && ERROR_FILE_NOT_FOUND != result) {
		printf("call RegOpenKeyEx failure, result= %d\n", result);
	}

	if (ERROR_FILE_NOT_FOUND == result) {
		result = RegCreateKeyEx(hRootKey, lpKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
		if (ERROR_SUCCESS != result) {
			RegCloseKey(hKey);
			printf("RegCreateKeyEx failure, result= %d\n", result);
			return result;
		}
	}

	result = RegSetValueEx(hKey, lpValueName, 0, REG_SZ, (const unsigned char *)lpValue, strlen(lpValue));
	if (ERROR_SUCCESS != result) {
		RegCloseKey(hKey);
		printf("RegSetValueEx failure, result= %d", result);
		return result;
	}

	if (0 != hKey) RegCloseKey(hKey);
	return result;
}

long getCurrentChannelFromRegister(HKEY hRootKey, LPCTSTR lpKeyName, LPCTSTR lpValueName, std::string &value)
{
	HKEY hKey = nullptr;

	long result = RegOpenKeyEx(hRootKey, lpKeyName, 0, KEY_READ | KEY_QUERY_VALUE, &hKey);
	if (ERROR_SUCCESS != result && ERROR_FILE_NOT_FOUND != result) {
		printf("call RegOpenKeyEx failure, result= %d\n", result);
		return result;
	}

	char buffer[256] = {0};
	DWORD len;
	result = RegQueryValueEx(hKey, lpValueName, NULL, NULL, (LPBYTE)buffer, &len);
	if (result != ERROR_SUCCESS) {
		printf("RegQueryValueEx failure, result= %d\n", result);
		RegCloseKey(hKey);
		return result;
	}
	value = buffer;

	if (0 != hKey) RegCloseKey(hKey);
	return result;
}

#endif //Q_OS_WIN

#ifdef __linux__
long setCurrentChannelIntoConfigFile(const char *fileName, const char *channel)
{
	std::string strJson;
	char *serialized_string = NULL;
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	json_object_set_string(root_object, "portOffset", "10");
	json_object_set_string(root_object, "socketAddr", channel);

	serialized_string = json_serialize_to_string_pretty(root_value);
	strJson = serialized_string;
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);

	std::ofstream config(fileName, std::ios::out | std::ios::trunc);
	if (config.is_open())
	{
		config << strJson;
		return ERR_SUCCESS;
	}
	else
		return ERR_ERROR;
}

long getCurrentChannelFromConfigFile(const char *fileName, std::string &channel)
{
	std::ifstream fs(fileName);
	if (!fs.is_open()) return ERR_ERROR;

	std::string strJson((std::istreambuf_iterator<char>(fs)),
		std::istreambuf_iterator<char>());

	NetConfig config;
	JSON_Value *root_value = json_value_init_string(strJson.data());
	JSON_Object *root_object = json_value_get_object(root_value);

	if (json_object_has_value(root_object, "portOffset"))
		config.portOffset = json_object_get_string(root_object, "portOffset");
	if (json_object_has_value(root_object, "channelAddr"))
		config.channelAddr = json_object_get_string(root_object, "channelAddr");

	json_value_free(root_value);

	channel = config.channelAddr;
	return ERR_SUCCESS;
}
#endif //__linux__

bool getNetConfig(NetConfig &config)
{
	std::string currentChannel;
	bool bRet = getChannelInfo(currentChannel);
	config.channelAddr = currentChannel;
	return bRet;
}


bool storeChannelInfo(const std::string &channel)
{
#ifdef _WIN32
	setCurrentChannelIntoRegister(HKEY_LOCAL_MACHINE, REG_PORT_PATH, "currentChannel", channel.c_str());
#elif __linux__
	setCurrentChannelIntoConfigFile(LINUX_CFG_PATH, channel.c_str());
#endif // _WIN32
	return true;
}

bool getChannelInfo(std::string &channel)
{
#ifdef _WIN32
	getCurrentChannelFromRegister(HKEY_LOCAL_MACHINE, REG_PORT_PATH, "currentChannel", channel);
#elif __linux__
	getCurrentChannelFromConfigFile(LINUX_CFG_PATH, channel);
#endif //_WIN32
	std::string::size_type pos = channel.find("tcp://*");
	if (pos != std::string::npos)
	{
		pos = channel.find("*");
		channel.replace(pos, 1, "127.0.0.1");
	}
	return !channel.empty();
}
