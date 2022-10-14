/**
 *  @file  msglibs.h
 *  @brief msglib服务器的接口头文件
 *  @date  2020年01月02日
 *  @author sharp
 *  @email  yangcp@koal.com
 */ 
#ifndef THIS_IS_MSG_LIB_CLIENT_HEADER_FROM_SHARP_202001021219_hfdfg
#define THIS_IS_MSG_LIB_CLIENT_HEADER_FROM_SHARP_202001021219_hfdfg

#include "msglib.h"

namespace koal {
/**
 * 
 */
 KoalInterface IClientItem {
    virtual const char *  getAppName() = 0 ;
    virtual const char *  getAppID() = 0 ;
	virtual unsigned int  getID() = 0;
 };

/**
 * @brief 消息服务
 */ 
KoalInterface ImsgServer {
    /**
     * @brief 登录消息过滤 
     * @param [in] pAppName 应用名称
     * @param [in] pAppID 应用ID 
     * @param [in] pAppToken 应用令牌
     * @param [out] pTips 如果返回值为false,  可通过该参数设置提示, 长度不超过127个字节
     * @return 返回true 登录通过， 返回false 登录失败
     */ 
    virtual bool onLogin(IClientItem * pCli, char * pTips) = 0;

    /*
    * @brief 心跳消息
    * @param pCli 客户端对象指针
    */
    virtual bool onHb(IClientItem * pCli) = 0;

    /**
     * @brief 客户端消息处理
     * @param pCli 客户端对象指针
     * @param msg  消息号
     * @param buf  消息数据
     * @param bufLen 消息长度
     */ 
    virtual bool onClientMsg(IClientItem * pfromCli, unsigned short msg,unsigned char * buf, int bufLen) = 0;

    /**
     * @brief 注销通知
     * @param [in] IClientItem 客户端
	 * @param [in] isClean 是否清除
     */ 
    virtual void onLogout(IClientItem * pCli, bool isClean = true) = 0 ;
};
/**
 */ 
KoalInterface ImsgServerSink {
    /**
     * @brief  发送消息到客户端
     * @param  如果为NULL， 则广播该消息到所有客户端
     */
    virtual bool sendMsg(IClientItem * pCli,unsigned short msg,unsigned char * buf, int bufLen) = 0;

    /**
     * @brief 踢掉客户端
     */ 
    virtual void kickOff(const char * pCliID) = 0;

    /**
     * @brief 根据AppID获取
     */ 
    virtual IClientItem * getClientItem(const char * pAppID) = 0;
};
}

using namespace koal ;
/**
 * @brief 创建服务器接口
 * @param
 * @param pUrl 连接的字符串
 */ 
KOAL_EXPORT_MSGLIB ImsgServerSink * createMSGServer(ImsgServer * pSvr, const char * pUrl);

/**
 * @brief 释放服务器接口
 */ 
KOAL_EXPORT_MSGLIB void freeMSGServer(ImsgServerSink * pSink); 

#endif