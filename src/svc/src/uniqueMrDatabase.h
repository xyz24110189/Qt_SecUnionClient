#pragma once
#include "commonDef.h" 
#include <sqlite3/sqlite3.h>
#include <string>
#include <vector>

class UniqueMrDatabase
{
public:
	UniqueMrDatabase();
	~UniqueMrDatabase();

	UniqueMrDatabase(const UniqueMrDatabase &other) = delete;
	UniqueMrDatabase &operator=(const UniqueMrDatabase &other) = delete;

	bool InitDb();
	void CloseDb();
	bool HandleActionData(const ActionData &actionData, const std::string &content);
	bool HandleSeviceData(const SeviceData &seviceData, const std::string &content);
	bool DeleteAppData(const BaseData &baseData);
	bool QueryEvents(std::vector<std::string> &strVec);
	bool GetLocalTGT(std::string &strTGT);

protected:
	bool CreateTable();
	bool TableExistJudge();
	bool InsertActionData(const ActionData &actionData, const std::string &content);
	bool InsertSeviceData(const SeviceData &serviceData, const std::string &content);
	bool DeleteActionData(const ActionData &actionData);
	bool DeleteSeviceData(const SeviceData &serviceData);

private:
	static int callback(void *param, int argc, char **argv, char **azColName);
	static int QueryStrCallback(void *param, int argc, char **argv, char **azColName);
	static int QueryStrVecCallback(void *param, int argc, char **argv, char **azColName);
	static int TableExistCallback(void *param, int argc, char **argv, char **azColName);

private:
	sqlite3 *_db = nullptr;
};


