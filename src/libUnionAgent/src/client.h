#pragma once
#include "libUnionAgent.h"
#include <msglib/msglibc.h>
#include <string>
#include <functional>

using namespace koal;

class ClientPipe : virtual public ImsgClient
{
public:
	ClientPipe();
	~ClientPipe();

	bool init(const LOGINDATA &logData);

	/**
	* @ brief 断线重连
	*/
	void onReLogin(bool loginStatus) override;

	/**
	* @breif出现异常退出
	*/
	void  onLogout() override;

	/**
	* @brief 服务器返回来的消息,不要在此执行耗时操作
	*
	*/
	bool  onServerMsg(unsigned short msg, unsigned char * buf, int bufLen) override;

	/**
	* @brief 错误输出
	*/
	void  onError(int err, const char * tip) override;

	/**
	* @brief 注册重新登录回调函数
	*/
	void reLoginCallBack(std::function<void(bool)> reLoginFun)
	{
		m_reLoginFun = reLoginFun;
	}

	/*
	* @brief 注册回调消息回调函数
	* @param [in] std::function   回调函数
	*/
	void msgCallBack(std::function<void(const char *, const char *, int, void *)> msgFun, void *param) 
	{
		m_msgFun = msgFun;
		m_param = param;
	}

	/*
	* @brief 发送消息
	*/
	bool sendMsg(unsigned short msg, unsigned char *buf, int bufLen);

private:
	ClientPipe(const ClientPipe &);
	ClientPipe &operator=(const ClientPipe &);

	bool initChannel();
	bool initLog();

	void *m_param;
	ImsgClientSink * m_pSink;
	std::function<void(bool)> m_reLoginFun;
	std::function<void(const char *, const char *, int, void *)> m_msgFun;
};
