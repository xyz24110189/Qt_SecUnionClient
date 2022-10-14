#pragma once
#include <string>
#include "libUnionAgent.h"

/**
* @brief 编码格式转换
* @param [conv] 转换字符串
*/
void encodingConv(std::string &conv);

/**
* @ brief 解析消息透传Json数据
* @ param [strJson]     json内容
* @ param [appName]     应用名称
* @ param [sendOrRecv]  发送/接受
* @ param [msgSign]     消息标识
* @ param [msg]         消息
*/
bool parseTransmitDataJson(const std::string &strJson/*in*/,
	std::string &appName/*out*/, 
	std::string &msg/*out*/);

/**
* @ brief 构造消息透传Json数据
* @ param [appName]     应用名称
* @ param [sendOrRecv]  发送/接受
* @ param [msgSign]     消息标识
* @ param [buf]         消息内容
* @ param [bufLen]      消息大小
* @ param [strJson]     返回json字符串
*/
bool buildTransmitDataJson(const char *appName, 
	const char *buf, 
	int bufLen, 
	std::string &strJson);

/*
* @ brief 构造程序json数据
* @ param [menuData] 菜单数据
* @ param [strJson]  返回json字符串
*/
bool buildProcessDataJson(const MENUDATA *menuData/*in*/,
	std::string &strJson/*out*/);

/**
* @ brief 构造托盘信息数据
* @ param [menuData] 托盘数据
* @ param [strJson]  返回json字符串
*/
bool buildTrayIconsDataJson(const TRAYICONS *trayIcons/*in*/,
	std::string &strJson/*out*/);

/**
* @ brief 构造托盘气泡数据
* @ param [trayTips] 冒泡信息
* @ param [strJson]  返回json数据
*/
bool buildTrayTipsDataJson(const TRAYTIPS *trayTips/*in*/, 
	std::string &strJson/*out*/);

/**
* @ brief 
* @ param [proram]  操作程序信息
* @ param [strJson] 返回json数据
*/
bool buildProgramOperDataJson(const PROGRAMDATA *proram/*in*/,
	std::string &strJson/*out*/);


/*
* @ brief 构造服务json数据
* @ param [vecData]  服务数据数组
* @ param [strJson]  返回json字符串
*/
//extern "C" bool buildServiceDataJson(int nSize/*in*/, const SERVICEDATA *vecData/*in*/,
//	std::string &strJson/*out*/);