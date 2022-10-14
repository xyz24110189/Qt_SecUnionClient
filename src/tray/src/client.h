#pragma once
#include <msglib/msglibc.h>
#include <stdio.h>
#include <string>
#include <functional>

using namespace koal;

class ClientPipe : virtual public ImsgClient
{
public:
	static ClientPipe *instance();

	void init();

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

	/*
	* @brief 注册重新登录回调函数
	* @param [in] std::function   回调函数
	*/
	void reLoginCallBack(std::function<void(bool)> reLoginCall)
	{
		m_reLoginCallBack = reLoginCall;
	}

	/*
	* @brief 注册回调消息回调函数
	* @param [in] std::function   回调函数
	*/
	void msgCallBack(std::function<void(const std::string &)> msgCall) 
	{
		m_msgCallBack = msgCall;
	}

	/*
	* @brief 发送消息
	*/
	bool sendMsg(unsigned short msg, unsigned char *buf, int bufLen);

private:
	ClientPipe();
	~ClientPipe();

	bool initChannel();

	ImsgClientSink * m_pSink;
	std::function<void(bool)> m_reLoginCallBack;
	std::function<void(const std::string &)> m_msgCallBack;
};
