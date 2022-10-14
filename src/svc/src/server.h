#pragma once
#include <msglib/msglibs.h>
#include <vector>
#include <string>
#include <functional>

using namespace koal ;

class ServerPipe : virtual public ImsgServer
{
	typedef std::function<void(unsigned short, const std::string &)> msgFunType;
	typedef std::function<void(void *)> logFunType;
public:
	static ServerPipe *instance();

	/**
	* @brief 反初始化
	*/
	void unInit();

	/**
	 * @brief 登录消息过滤
	 * @param [in] pAppName 应用名称
	 * @param [in] pAppID 应用ID
	 * @param [in] pAppToken 应用令牌
	 * @param [out] pTips 如果返回值为false,  可通过该参数设置提示, 长度不超过127个字节
	 * @return 返回true 登录通过， 返回false 登录失败
	 */
	bool onLogin(IClientItem * pCli, char * pTips) override;

	/*
    * @brief 心跳消息
    * @param pCli 客户端对象指针
    */
	bool onHb(IClientItem * pCli) override;

	/**
	 * @brief 客户端消息处理
	 * @param pCli 客户端对象指针
	 * @param msg  消息号
	 * @param buf  消息数据
	 * @param bufLen 消息长度
	 */
	bool onClientMsg(IClientItem * pfromCli, unsigned short msg, unsigned char * buf, int bufLen) override;

	/**
	 * @brief 注销通知
	 * @param [in] IClientItem 客户端
	 */
	void onLogout(IClientItem * pCli, bool isClean = true) override;

	/*
	* @brief 注册回调消息回调函数
	* @param [in] std::function   回调函数
	*/
	void msgCallBack(msgFunType msgCall)
	{
		m_msgCallback = msgCall;
	}

	/*
	* @brief 注册登录回调函数
	* @客户端标识
	*/
	void logInCallBack(logFunType logInCall)
	{
		m_logInCallback = logInCall;
	}

	/*
	* @brief 注册登出回调函数
	* @客户端标识
	*/
	void logOutCallBack(logFunType logOutCall)
	{
		m_logOutCallback = logOutCall;
	}

	/*
	* @brief 发送消息接口
	* @param [in] msg  消息号
	* @param [in] 消息体
	* @param [in] 消息体大小
	* @return true:成功 / false:失败
	*/
	bool sendMsg(const std::string &appId, 
		unsigned short msg, 
		unsigned char *buf, 
		int bufLen);

	/**
	* @ brief 根据appName获取appId
	* @ param [appName] 应用名称
	* @ param [appId]     应用ID
	*/
	bool getAppIdByAppName(const std::string &appName/*in*/, std::string &appId/*out*/);

	/**
	* @ brief 执行客户端登出通知托盘
	* @ param [pfromCli] 登出客户端
	*/
	bool logoutMsgToTray(IClientItem * pfromCli);

protected:
	void buildCmdJson(IClientItem * pfromCli, 
		unsigned short msg, 
		const std::string &strJson, 
		std::string &cmdJson/*out*/);

private:
	ServerPipe();
	~ServerPipe();

	/**
	* @brief 初始化
	*/
	void initChannel();
	int m_interval;
	ImsgServerSink  *m_pSink;
	std::vector<IClientItem *> m_vecClients;
	msgFunType m_msgCallback;
	logFunType m_logInCallback;
	logFunType m_logOutCallback;
};


