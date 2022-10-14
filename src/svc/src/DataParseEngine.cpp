#include "DataParseEngine.h"
#include <sstream>

DataParseEngine::DataParseEngine()
{
}

DataParseEngine::~DataParseEngine()
{
}

bool DataParseEngine::ParsonTransmitObj(const JSON_Value *jData, std::string &appName, std::string &msg)
{
	JSON_Object *root_obj = json_value_get_object(jData);
	if (!root_obj) return false;

	BaseData data;
	if (json_object_has_value(root_obj, "appName"))
		data.appName = json_object_get_string(root_obj, "appName");
	if (json_object_has_value(root_obj, "appId"))
		data.appId = json_object_get_string(root_obj, "appId");
	if (json_object_has_value(root_obj, "cmdCode"))
		data.cmdCode = static_cast<int32_t>(json_object_get_number(root_obj, "cmdCode"));

	JSON_Object *content = nullptr;
	if (json_object_has_value(root_obj, "content"))
		content = json_object_get_object(root_obj, "content");

	if (content)
	{
		if (json_object_has_value(content, "appName"))
			appName = json_object_get_string(content, "appName");
		json_object_set_string(content, "appName", data.appName.c_str());

		char *serialized_string = nullptr;
		serialized_string = json_serialize_to_string_pretty(jData);
		msg = serialized_string;
		json_free_serialized_string(serialized_string);
	}
	return true;
}

bool DataParseEngine::ParsonActionObj(const JSON_Object *jData, std::vector<ActionData> &vData)
{
	ActionData data;
	if (json_object_has_value(jData, "appName"))
		data.appName = json_object_get_string(jData, "appName");
	if (json_object_has_value(jData, "appId"))
		data.appId = json_object_get_string(jData, "appId");
	if (json_object_has_value(jData, "cmdCode"))
		data.cmdCode = static_cast<int32_t>(json_object_get_number(jData, "cmdCode"));

	JSON_Array *content = nullptr;
	if (json_object_has_value(jData, "content"))
		content = json_object_get_array(jData, "content");

	if (content)
	{
		for (int i = 0; i < json_array_get_count(content); ++i)
		{
			JSON_Object *item = json_array_get_object(content, i);
			if (!item) continue;

			if (json_object_has_value(item, "priority"))
				data.priority = static_cast<unsigned short>(json_object_get_number(item, "priority"));
			if (json_object_has_value(item, "parentMenus"))
				data.parentMenus = json_object_get_string(item, "parentMenus");
			if (json_object_has_value(item, "actName"))
				data.actionName = json_object_get_string(item, "actName");
			if (json_object_has_value(item, "cmd"))
				data.cmd = json_object_get_string(item, "cmd");

			JSON_Array *argsArray = nullptr;
			if (json_object_has_value(item, "args"))
				argsArray = json_object_get_array(item, "args");

			data.args = "";
			if (argsArray && json_array_get_count(argsArray) > 0)
			{
				for (int j = 0; j < json_array_get_count(argsArray); ++j)
				{
					data.args += json_array_get_string(argsArray, j);
					data.args += " ";
				}
				data.args.erase(data.args.length() - 1);
			}
			vData.emplace_back(data);
		}
	}
	return true;
}

bool DataParseEngine::ParsonActionInnerObj(const JSON_Object *jData, ActionData &opData)
{
	JSON_Object *content = nullptr;
	if (json_object_has_value(jData, "content"))
		content = json_object_get_object(jData, "content");

	if (content)
	{
		if (json_object_has_value(content, "appName"))
			opData.appName = json_object_get_string(content, "appName");
		if (json_object_has_value(content, "appId"))
			opData.appId = json_object_get_string(content, "appId");
		if (json_object_has_value(content, "cmdCode"))
			opData.cmdCode = static_cast<int32_t>(json_object_get_number(content, "cmdCode"));
		if (json_object_has_value(content, "priority"))
			opData.priority = static_cast<unsigned short>(json_object_get_number(content, "priority"));
		if (json_object_has_value(content, "parentMenus"))
			opData.parentMenus = json_object_get_string(content, "parentMenus");
		if (json_object_has_value(content, "actName"))
			opData.actionName = json_object_get_string(content, "actName");
		if (json_object_has_value(content, "cmd"))
			opData.cmd = json_object_get_string(content, "cmd");
		if (json_object_has_value(content, "args"))
			opData.args = json_object_get_string(content, "args");
	}
	return true;
}


bool DataParseEngine::ParsonActionObj(const JSON_Object *jData, ActionData &opData)
{
	if (json_object_has_value(jData, "appName"))
		opData.appName = json_object_get_string(jData, "appName");
	if (json_object_has_value(jData, "appId"))
		opData.appId = json_object_get_string(jData, "appId");
	if (json_object_has_value(jData, "cmdCode"))
		opData.cmdCode = static_cast<int32_t>(json_object_get_number(jData, "cmdCode"));

	JSON_Object *content = nullptr;
	if (json_object_has_value(jData, "content"))
		content = json_object_get_object(jData, "content");

	if (content)
	{
		if (json_object_has_value(content, "priority"))
			opData.priority = static_cast<unsigned short>(json_object_get_number(content, "priority"));
		if (json_object_has_value(content, "parentMenus"))
			opData.parentMenus = json_object_get_string(content, "parentMenus");
		if (json_object_has_value(content, "actName"))
			opData.actionName = json_object_get_string(content, "actName");
		if (json_object_has_value(content, "cmd"))
			opData.cmd = json_object_get_string(content, "cmd");

		JSON_Array *argsArray = nullptr;
		if (json_object_has_value(content, "args"))
			argsArray = json_object_get_array(content, "args");

		opData.args = "";
		if (argsArray && json_array_get_count(argsArray) > 0)
		{
			for (int i = 0; i < json_array_get_count(argsArray); ++i)
			{
				opData.args += json_array_get_string(argsArray, i);
				opData.args += " ";
			}
			opData.args.erase(opData.args.length() - 1);
		}
	}
	return true;
}

bool DataParseEngine::ParsonServiceObj(const JSON_Object *jData, std::vector<SeviceData> &vData)
{
	SeviceData data;
	if (json_object_has_value(jData, "appName"))
		data.appName = json_object_get_string(jData, "appName");
	if (json_object_has_value(jData, "appId"))
		data.appId = json_object_get_string(jData, "appId");
	if (json_object_has_value(jData, "cmdCode"))
		data.cmdCode = static_cast<int32_t>(json_object_get_number(jData, "cmdCode"));

	JSON_Array *content = nullptr;
	if (json_object_has_value(jData, "content"))
		content = json_object_get_array(jData, "content");

	if (content)
	{
		for (int i = 0; i < json_array_get_count(content); ++i)
		{
			JSON_Object *item = json_array_get_object(content, i);
			if (!item) continue;

			if (json_object_has_value(item, "priority"))
				data.priority = static_cast<unsigned short>(json_object_get_number(item, "priority"));
			if (json_object_has_value(item, "parentMenus"))
				data.parentMenus = json_object_get_string(item, "parentMenus");
			if (json_object_has_value(item, "actName"))
				data.actionName = json_object_get_string(item, "actName");
			if (json_object_has_value(item, "cmd"))
				data.serverName = json_object_get_string(item, "cmd");

			JSON_Array *argsArray = nullptr;
			if (json_object_has_value(item, "args"))
				argsArray = json_object_get_array(item, "args");

			data.args = "";
			if (argsArray && json_array_get_count(argsArray) > 0)
			{
				for (int j = 0; j < json_array_get_count(argsArray); ++j)
				{
					data.args += json_array_get_string(argsArray, j);
					data.args += '&';
				}
				data.args.erase(data.args.length() - 1);
			}
			vData.emplace_back(data);
		}
	}

	return true;
}

bool DataParseEngine::ParsonServiceOper(const JSON_Object *jData, ServiceOp &opData)
{
	if (json_object_has_value(jData, "appName"))
		opData.appName = json_object_get_string(jData, "appName");
	if (json_object_has_value(jData, "appId"))
		opData.appId = json_object_get_string(jData, "appId");
	if (json_object_has_value(jData, "cmdCode"))
		opData.cmdCode = static_cast<int32_t>(json_object_get_number(jData, "cmdCode"));

	JSON_Object *content = nullptr;
	if (json_object_has_value(jData, "content"))
		content = json_object_get_object(jData, "content");

	if (content)
	{
		if (json_object_has_value(content, "serverName"))
			opData.serviceName = json_object_get_string(content, "serverName");
		if (json_object_has_value(content, "args"))
			opData.args = json_object_get_string(content, "args");
	}
	return true;
}

bool DataParseEngine::ParsonProgramOperObj(const JSON_Object *jData, ProgramData &proData)
{
	if (json_object_has_value(jData, "appName"))
		proData.appName = json_object_get_string(jData, "appName");
	if (json_object_has_value(jData, "appId"))
		proData.appId = json_object_get_string(jData, "appId");
	if (json_object_has_value(jData, "cmdCode"))
		proData.cmdCode = static_cast<int32_t>(json_object_get_number(jData, "cmdCode"));

	JSON_Object *content = nullptr;
	if (json_object_has_value(jData, "content"))
		content = json_object_get_object(jData, "content");

	if (content)
	{
		if (json_object_has_value(content, "exePath"))
			proData.exePath = json_object_get_string(content, "exePath");
		if (json_object_has_value(content, "exeParam"))
			proData.exeParam = json_object_get_string(content, "exeParam");
		if (json_object_has_value(content, "operation"))
			proData.oper = static_cast<uint16_t>(json_object_get_number(content, "operation"));
	}
	return true;
}

std::vector<std::string> DataParseEngine::Split(const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}
