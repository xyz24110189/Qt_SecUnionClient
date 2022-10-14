#pragma once
#include <commonDef.h>
#include <vector>

class DataParseEngine
{
public:
	DataParseEngine();
	~DataParseEngine();
	
	static bool ParsonTransmitObj(const JSON_Value *jData, std::string &appName, std::string &msg);

	static bool ParsonActionObj(const JSON_Object *jData, std::vector<ActionData> &vData);
	static bool ParsonActionObj(const JSON_Object *jData, ActionData &opData);
	static bool ParsonActionInnerObj(const JSON_Object *jData, ActionData &opData);

	static bool ParsonServiceObj(const JSON_Object *jData, std::vector<SeviceData> &vData);
	static bool ParsonServiceOper(const JSON_Object *jData, ServiceOp &opData);

	static bool ParsonProgramOperObj(const JSON_Object *jData, ProgramData &proData);

private:
	static std::vector<std::string> Split(const std::string& s, char delimiter);
};

