#include "uniqueMrDatabase.h"
#include <string>
#include <cstdio>
#include <thread> // std::this_thread::sleep_for
#include <chrono> // std::chrono::seconds
#include <tools.h>
#include "DataParseEngine.h"

#define  DB_NAME  "uniqueManager.db"

UniqueMrDatabase::UniqueMrDatabase()
{
}

UniqueMrDatabase::~UniqueMrDatabase()
{
	CloseDb();
}

bool UniqueMrDatabase::InitDb()
{
	char *zErrMsg = 0;
	int rc;

	rc = sqlite3_config(SQLITE_CONFIG_SERIALIZED);
	if (rc) {
		LOG_ERROR << "Can't config SQLITE_CONFIG_SERIALIZED param";
		return false;
	}

	std::string dbPath = koal::tool::getAppDataPath() + "/database/" + DB_NAME;
	rc = sqlite3_open(dbPath.c_str(), &_db);

	if (rc) {
		LOG_ERROR << "Can't open database: " << sqlite3_errmsg(_db);
		return false;
	}
	else {
		LOG_INFO << "Opened database successfully.";
		bool bRet = CreateTable();

		if (!bRet)
		{
			std::this_thread::sleep_for(std::chrono::seconds(2));
			if (!CreateTable())
			{
				sqlite3_close(_db);
				exit(-1);
			}
		}
		return true;
	}
}

void UniqueMrDatabase::CloseDb()
{
	LOG_INFO << "UniqueMrDatabase::CloseDb begin...";
	if (_db)
	{
		sqlite3_close(_db);
		_db = nullptr;
	}
	LOG_INFO << "UniqueMrDatabase::CloseDb end...";
}

bool UniqueMrDatabase::HandleActionData(const ActionData &actionData, const std::string &content)
{
	LOG_INFO << "UniqueMrDatabase::HandleActionData begin...";
	bool bRet = false;

	switch (actionData.cmdCode)
	{
	case MSG_PROCESS_MONITOR_REGISTER:
	case MSG_PROCESS_CONTROL_REGISTER:
		bRet = InsertActionData(actionData, content);
		break;
	case MSG_PROCESS_MONITOR_UNREGISTER:
	case MSG_PROCESS_CONTROL_UNREGISTER:
		bRet = DeleteActionData(actionData);
		break;
	default:
		break;
	}

	LOG_INFO << "UniqueMrDatabase::HandleActionData end...";
	return bRet;
}

bool UniqueMrDatabase::HandleSeviceData(const SeviceData &seviceData, const std::string &content)
{
	LOG_INFO << "UniqueMrDatabase::HandleSeviceData begin...";
	bool bRet = false;
	switch (seviceData.cmdCode)
	{
	case MSG_SERVICE_CONTROL_REGISTER:
		bRet = InsertSeviceData(seviceData, content);
		break;
	case MSG_SERVICE_CONTROL_UNREGISTER:
		bRet = DeleteSeviceData(seviceData);
		break;
	default:
		break;
	}
	LOG_INFO << "UniqueMrDatabase::HandleSeviceData end...";
	return bRet;
}

bool UniqueMrDatabase::InsertActionData(const ActionData &actionData, const std::string &content)
{
	LOG_INFO << "UniqueMrDatabase::InsertActionData begin...";
	char *zErrMsg = 0;
	int rc;

	DataParseEngine parseEngine;
	//std::string json;
	//parseEngine.ActionObj2Json(actionData, json);

	/* Create SQL statement */
	int nSize = content.length() * 3;
	char *sql = new char[nSize];
	memset(sql, 0, nSize);
	std::snprintf(sql, nSize, "INSERT INTO EVENTS(APPNAME, APPID, CMDCODE, \
			PARENTMENUS, UNIQUEVALUE, ACTIONDATA) \
			VALUES('%s', '%s', %d, '%s', '%s', '%s')", \
			actionData.appName.c_str(), actionData.appId.c_str(), \
			actionData.cmdCode, actionData.parentMenus.c_str(), \
			actionData.actionName.c_str(), content.c_str());

	/* Execute SQL statement */
	rc = sqlite3_exec(_db, sql, callback, 0, &zErrMsg);

	if (rc != SQLITE_OK) {
		LOG_ERROR << "InsertActionData SQL error: " << zErrMsg;
		LOG_INFO << "UniqueMrDatabase::InsertActionData end...";
		sqlite3_free(zErrMsg);
		delete []sql;
		return false;
	}
	else {
		LOG_INFO << "InsertActionData Operation done successfully!";
		LOG_INFO << "UniqueMrDatabase::InsertActionData end...";
		delete[]sql;
		return true;
	}
}

bool UniqueMrDatabase::InsertSeviceData(const SeviceData &serviceData, const std::string &content)
{
	LOG_INFO << "UniqueMrDatabase::InsertSeviceData begin...";
	char *zErrMsg = 0;
	int rc;

	DataParseEngine parseEngine;
	//std::string json;
	//parseEngine.ServiceObj2Json(serviceData, json);

	/* Create SQL statement */
	int nSize = content.length() * 3;
	char *sql = new char[nSize];
	memset(sql, 0, nSize);
	std::snprintf(sql, nSize, "INSERT INTO EVENTS(APPNAME, APPID, CMDCODE, \
			PARENTMENUS, UNIQUEVALUE, ACTIONDATA) \
			VALUES('%s', '%s', %d, '%s' ,'%s', '%s')", \
			serviceData.appName.c_str(), serviceData.appId.c_str(), \
			serviceData.cmdCode, serviceData.parentMenus.c_str(), \
			serviceData.actionName.c_str(), content.c_str());

	/* Execute SQL statement */
	rc = sqlite3_exec(_db, sql, callback, 0, &zErrMsg);

	if (rc != SQLITE_OK) {
		LOG_ERROR << "InsertSeviceData SQL error: " << zErrMsg;
		LOG_INFO << "UniqueMrDatabase::InsertSeviceData end...";
		sqlite3_free(zErrMsg);
		delete []sql;
		return false;
	}
	else {
		LOG_INFO << "InsertSeviceData Operation done successfully!";
		LOG_INFO << "UniqueMrDatabase::InsertSeviceData end...";
		delete[]sql;
		return true;
	}
}

bool UniqueMrDatabase::QueryEvents(std::vector<std::string> &strVec)
{
	char *zErrMsg = 0;
	int rc;

	/* Create SQL statement */
	char sql[BUFSIZ] = { 0 };
	std::snprintf(sql, BUFSIZ, "SELECT ACTIONDATA from EVENTS");

	/* Execute SQL statement */
	rc = sqlite3_exec(_db, sql, QueryStrVecCallback, (void*)&strVec, &zErrMsg);

	if (rc != SQLITE_OK) {
		LOG_ERROR << "QueryLocalTGT SQL error: ", zErrMsg;
		sqlite3_free(zErrMsg);
		return false;
	}
	else {
		LOG_INFO << "QueryLocalTGT Operation done successfully";
		return !strVec.empty();
	}
}


bool UniqueMrDatabase::GetLocalTGT(std::string &strTGT)
{
	char *zErrMsg = 0;
	int rc;

	/* Create SQL statement */
	char sql[BUFSIZ] = { 0 };
	std::snprintf(sql, BUFSIZ, "SELECT TGTDATA from TGT");

	/* Execute SQL statement */
	rc = sqlite3_exec(_db, sql, QueryStrCallback, (void*)&strTGT, &zErrMsg);

	if (rc != SQLITE_OK) {
		LOG_ERROR << "GetLocalTGT SQL error: ", zErrMsg;
		sqlite3_free(zErrMsg);
		return false;
	}
	else {
		LOG_INFO << "GetLocalTGT Operation done successfully";
		return !strTGT.empty();
	}
}

bool UniqueMrDatabase::DeleteActionData(const ActionData &actionData)
{
	LOG_INFO << "UniqueMrDatabase::DeleteActionData begin...";
	char *zErrMsg = 0;
	int rc;

	/* Create SQL statement */
	char sql[BUFSIZ] = { 0 };
	std::snprintf(sql, BUFSIZ, "DELETE FROM EVENTS WHERE APPNAME='%s' \
				AND APPID='%s' AND CMDCODE=%d AND PARENTMENUS='%s' \
				AND UNIQUEVALUE='%s'", \
				actionData.appName.c_str(), actionData.appId.c_str(), \
				actionData.cmdCode - 1, actionData.parentMenus.c_str(), \
				actionData.actionName.c_str());


	/* Execute SQL statement */
	rc = sqlite3_exec(_db, sql, callback, 0, &zErrMsg);

	if (rc != SQLITE_OK) {
		LOG_ERROR << "DeleteActionData SQL error: ", zErrMsg;
		LOG_INFO << "UniqueMrDatabase::DeleteActionData end...";
		sqlite3_free(zErrMsg);
		return false;
	}
	else {
		LOG_INFO << "DeleteActionData Operation Done successfully.";
		LOG_INFO << "UniqueMrDatabase::DeleteActionData end...";
		return true;
	}
}

bool UniqueMrDatabase::DeleteSeviceData(const SeviceData &serviceData)
{
	LOG_INFO << "UniqueMrDatabase::DeleteSeviceData begin...";
	char *zErrMsg = 0;
	int rc;

	/* Create SQL statement */
	char sql[BUFSIZ] = { 0 };
	std::snprintf(sql, BUFSIZ, "DELETE FROM EVENTS WHERE APPNAME='%s' \
				AND APPID='%s' AND CMDCODE=%d AND PARENTMENUS='%s'\
				AND UNIQUEVALUE='%s'", \
				serviceData.appName.c_str(), serviceData.appId.c_str(), \
				serviceData.cmdCode - 1, serviceData.parentMenus.c_str(), \
				serviceData.actionName.c_str());


	/* Execute SQL statement */
	rc = sqlite3_exec(_db, sql, callback, 0, &zErrMsg);

	if (rc != SQLITE_OK) {
		LOG_ERROR << "DeleteActionData SQL error: ", zErrMsg;
		LOG_INFO << "UniqueMrDatabase::DeleteSeviceData end...";
		sqlite3_free(zErrMsg);
		return false;
	}
	else {
		LOG_INFO << "DeleteActionData Operation Done successfully.";
		LOG_INFO << "UniqueMrDatabase::DeleteSeviceData end...";
		return true;
	}
}

bool UniqueMrDatabase::DeleteAppData(const BaseData &baseData)
{
	LOG_INFO << "UniqueMrDatabase::DeleteAppData begin...";
	char *zErrMsg = 0;
	int rc;

	/* Create SQL statement */
	char sql[BUFSIZ] = { 0 };
	std::snprintf(sql, BUFSIZ, "DELETE FROM EVENTS WHERE \
		APPNAME='%s' AND APPID='%s' AND CMDCODE!=%d", \
		baseData.appName.c_str(), 
		baseData.appId.c_str(), 
		MSG_PROCESS_MONITOR_REGISTER);


	/* Execute SQL statement */
	rc = sqlite3_exec(_db, sql, callback, 0, &zErrMsg);

	if (rc != SQLITE_OK) {
		LOG_ERROR << "DeleteAppData SQL error: ", zErrMsg;
		LOG_INFO << "UniqueMrDatabase::DeleteAppData end...";
		sqlite3_free(zErrMsg);
		return false;
	}
	else {
		LOG_INFO << "DeleteAppData Operation Done successfully.";
		LOG_INFO << "UniqueMrDatabase::DeleteAppData end...";
		return true;
	}
}

bool UniqueMrDatabase::CreateTable()
{
	if (TableExistJudge()) return true;

	char *zErrMsg = 0;
	int rc;

	/* Create SQL statement */
	char *sql = "CREATE TABLE EVENTS(" \
		"ID INTEGER PRIMARY KEY AUTOINCREMENT  NOT NULL," \
		"APPNAME                            TEXT  NOT NULL," \
		"APPID                              TEXT  NOT NULL," \
		"CMDCODE                            INTEGER  NOT NULL," \
		"PARENTMENUS                        TEXT  NOT NULL," \
		"UNIQUEVALUE                        TEXT  NOT NULL," \
		"ACTIONDATA                         TEXT  NOT NULL," \
		"UNIQUE(APPNAME, APPID, CMDCODE, PARENTMENUS, UNIQUEVALUE))";

	/* Execute SQL statement */
	rc = sqlite3_exec(_db, sql, callback, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		LOG_ERROR << "CreateTable SQL error: " << zErrMsg;
		sqlite3_free(zErrMsg);
		return false;
	}
	else {
		LOG_INFO << "CreateTable Operator Done successfully.";
		return true;
	}
}

bool UniqueMrDatabase::TableExistJudge()
{
	char *zErrMsg = 0;
	int rc;
	bool hasTable = false;

	/* Create SQL statement */
	char sql[BUFSIZ] = { 0 };
	std::snprintf(sql, BUFSIZ, "SELECT name FROM sqlite_master WHERE type='table' AND name='%s'", "EVENTS");

	/* Execute SQL statement */
	rc = sqlite3_exec(_db, sql, TableExistCallback, (void*)&hasTable, &zErrMsg);

	if (rc != SQLITE_OK) {
		LOG_ERROR << "TableExistJudge SQL error:" << zErrMsg;
		sqlite3_free(zErrMsg);
		return false;
	}
	else {
		LOG_INFO << "TableExistJudge Operation done successfully";
		return hasTable;
	}
}

int UniqueMrDatabase::callback(void *param, int argc, char **argv, char **azColName)
{
	int i;
	for (i = 0; i < argc; i++) {
		LOG_INFO << "select from table " << azColName[i] << " " << (argv[i] ? argv[i] : "NULL");
	}
	return 0;
}

int UniqueMrDatabase::QueryStrCallback(void *param, int argc, char **argv, char **azColName)
{
	std::string *strTGT = (std::string *)param;

	int i;
	for (i = 0; i < argc; i++) {
		*strTGT = argv[i];
		LOG_INFO << "select TGT from table" << azColName[i] << " " << (argv[i] ? argv[i] : "NULL");
	}
	return 0;
}

int UniqueMrDatabase::QueryStrVecCallback(void *param, int argc, char **argv, char **azColName)
{
	std::vector<std::string> *strTGT = (std::vector<std::string> *)param;

	int i;
	for (i = 0; i < argc; i++) {
		strTGT->emplace_back(argv[i]);
		LOG_INFO << "select TGT from table" << azColName[i] << " " << (argv[i] ? argv[i] : "NULL");
	}
	return 0;
}

int UniqueMrDatabase::TableExistCallback(void *param, int argc, char **argv, char **azColName)
{
	bool *hasTable = (bool *)param;

	if (argc > 0)
		*hasTable = true;

	LOG_INFO << "TableExistCallback hasTable = " << *hasTable;
	return 0;
}



