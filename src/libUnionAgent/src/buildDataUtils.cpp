#include "buildDataUtils.h"
#include "parson.h"
#include "tools.h"
#include <string.h>
#include <utf8.h>
#include <vector>
#include <sstream>

extern Encoding g_enc;

/**
* @ brief 分割字符串
* @ param [str] 待分割字符串
* @ param [delimiter] 分割字符
*/
std::vector<std::string> imp_split(const std::string& str/*in*/, char delimiter/*in*/);

/**
* @ brief 完全替换字符串
* @ param [data] 字符串
* @ param [toSearch] 查找字符串
* @ param [replaceStr] 替换字符串
*/
void imp_replace_all(std::string & data, std::string toSearch, std::string replaceStr);

void encodingConv(std::string &conv)
{
	if (g_enc == Encoding::GB2312)
	{
		int nSize = conv.size() * 2;
		char *tmp = new char[nSize];
		memset(tmp, 0, nSize);
		gb_to_utf8(conv.c_str(), tmp, nSize);
		conv = tmp;
		imp_replace_all(conv, "\\\\", "\\\/");
		delete []tmp;
	}
}

bool parseTransmitDataJson(const std::string &strJson/*in*/,
	std::string &appName/*out*/, 
	std::string &msg/*out*/)
{
	JSON_Value  *root_value  = json_parse_string(strJson.c_str());
	JSON_Object *root_object = json_value_get_object(root_value);

	JSON_Object *content_object = json_object_get_object(root_object, "content");
	appName = json_object_get_string(content_object, "appName");
	msg = json_object_get_string(content_object, "message");
	json_value_free(root_value);
	return true;
}

bool buildTransmitDataJson(const char *appName, 
	const char *buf, 
	int bufLen, 
	std::string &strJson)
{
	std::string strMsg((const char*)buf, bufLen);
	char *serialized_string = NULL;
	JSON_Value  *root_value  = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	json_object_set_string(root_object, "appName", appName);
	json_object_set_string(root_object, "message", strMsg.c_str());
	serialized_string = json_serialize_to_string_pretty(root_value);
	strJson = serialized_string;
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);

	encodingConv(strJson);
	return !strJson.empty();
}

bool buildProcessDataJson(const MENUDATA *menuData/*in*/,
	std::string &strJson/*out*/)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);

	JSON_Value *args_value = json_value_init_array();
	JSON_Array *args_array = json_value_get_array(args_value);

	char *serialized_string = NULL;
	json_object_set_number(root_object, "priority", menuData->priority);
	json_object_set_string(root_object, "parentMenus", menuData->parentMenuPath);
	json_object_set_string(root_object, "actName", menuData->actionName);
	json_object_set_string(root_object, "cmd", menuData->exePath);

	std::vector<std::string> strVec = imp_split(menuData->exeParam, ';');
	for (int j = 0; j < strVec.size(); j++)
		json_array_append_string(args_array, strVec[j].c_str());

	json_object_set_value(root_object, "args", args_value);

	serialized_string = json_serialize_to_string_pretty(root_value);
	strJson = serialized_string;
	
	encodingConv(strJson);
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
	return !strJson.empty();
}

bool buildTrayIconsDataJson(const TRAYICONS *trayIcons/*in*/,
	std::string &strJson/*out*/)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	char *serialized_string = NULL;
	json_object_set_string(root_object, "iconPath", trayIcons->iconPath);
	json_object_set_string(root_object, "holdTips", trayIcons->tips);
	serialized_string = json_serialize_to_string_pretty(root_value);
	strJson = serialized_string;
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);

	encodingConv(strJson);
	return !strJson.empty();
}

bool buildTrayTipsDataJson(const TRAYTIPS *trayTips/*in*/, 
	std::string &strJson/*out*/)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	char *serialized_string = NULL;
	json_object_set_string(root_object, "tips", trayTips->tips);
	json_object_set_number(root_object, "holdTime", trayTips->holdTime);
	serialized_string = json_serialize_to_string_pretty(root_value);
	strJson = serialized_string;
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);

	encodingConv(strJson);
	return !strJson.empty();
}

bool buildProgramOperDataJson(const PROGRAMDATA *proram/*in*/,
	std::string &strJson/*out*/)
{
	JSON_Value *root_value = json_value_init_object();
	JSON_Object *root_object = json_value_get_object(root_value);
	
	char *serialized_string = NULL;
	json_object_set_string(root_object, "exePath", proram->exePath);
	json_object_set_string(root_object, "exeParam", proram->exeParam);
	json_object_set_number(root_object, "operation", proram->oper);

	serialized_string = json_serialize_to_string_pretty(root_value);
	strJson = serialized_string;
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
	
	encodingConv(strJson);
	return !strJson.empty();
}

/*bool buildServiceDataJson(int nSize, const SERVICEDATA *vecData,
	std::string &strJson)
{
	JSON_Value *root_arrya_value = json_value_init_array();
	JSON_Array *root_array = json_value_get_array(root_arrya_value);

	char *serialized_string = NULL;
	for (int i = 0; i < nSize; i++)
	{
		JSON_Value *root_value = json_value_init_object();
		JSON_Object *root_object = json_value_get_object(root_value);

		JSON_Value *args_value = json_value_init_array();
		JSON_Array *args_array = json_value_get_array(args_value);

		json_object_set_string(root_object, "actName", vecData[i].strActionName);
		json_object_set_string(root_object, "cmd", vecData[i].strServiceName);

		std::vector<std::string> strVec = imp_split(vecData[i].argv, ';');
		for (int j = 0; j < strVec.size(); j++)
		{
			json_array_append_string(args_array, strVec[j].c_str());
		}

		json_object_set_value(root_object, "args", args_value);
		json_array_append_value(root_array, root_value);
	}

	serialized_string = json_serialize_to_string_pretty(root_arrya_value);
	strJson = serialized_string;
	json_free_serialized_string(serialized_string);
	json_value_free(root_arrya_value);

	return !strJson.empty();

	nlohmann::json root = nlohmann::json::array();
	for (int i = 0; i < nSize; i++)
	{
		nlohmann::json process = {
			{ "actName", vecData[i].strActionName },
			{ "cmd",     vecData[i].strServiceName },
		};

		nlohmann::json argsObj = nlohmann::json::array();
		std::vector<std::string> strVec = imp_split(vecData[i].argv, ';');
		for (const auto &item : strVec)
		{
			argsObj.emplace_back(item);
		}
		process["args"] = argsObj;

		root.push_back(process);
	}

	strJson = root.dump();
	return !strJson.empty();
}*/

std::vector<std::string> imp_split(const std::string& str/*in*/, char delimiter/*in*/)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(str);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}

void imp_replace_all(std::string & data, std::string toSearch, std::string replaceStr)
{
	// Get the first occurrence
	size_t pos = data.find(toSearch);

	// Repeat till end is reached
	while( pos != std::string::npos)
	{
		// Replace this occurrence of Sub String
		data.replace(pos, toSearch.size(), replaceStr);
		// Get the next occurrence from the current position
		pos =data.find(toSearch, pos + replaceStr.size());
	}
}

