/**
 *  @file msglibc.h
 *  @brief msglib客户端的接口头文件
 *  @date 2020年01月02日
 *  @author sharp
 *  @email  yangcp@koal.com
 */ 
#ifndef THIS_IS_MSG_LIB_CLIENT_HEADER_FROM_SHARP_202001021219_fsfdsafdsa
#define THIS_IS_MSG_LIB_CLIENT_HEADER_FROM_SHARP_202001021219_fsfdsafdsa

#include "msglib.h"

namespace koal{

/**
 * 
 */ 
KoalInterface ImsgClient {
	/**
	* @ brief 断线重连
	*/
	virtual void onReLogin(bool loginStatus) = 0;

    /**
     * @breif出现异常退出
     */ 
    virtual void  onLogout() = 0 ;
    /**
     * @brief 服务器返回来的消息,不要在此执行耗时操作
     * 
     */ 
    virtual bool  onServerMsg(unsigned short msg,unsigned char * buf,int bufLen) = 0;
    /**
     * @brief 错误输出
     */ 
    virtual void  onError(int err,const char * tip) = 0;
};

/**
 * 
 */ 
KoalInterface ImsgClientSink {
    ///登录认证
    virtual bool   login(const char * pAppName, const char * pAppID, const char * pAppTokn, bool isOuter = false) = 0;
    ///发送消息
    virtual bool   sendMsg(unsigned short msg, unsigned char * buf, int bufLen, void * pOut, int outLen) = 0;
    ///发送消息
    virtual bool   postMsg(unsigned short msg,unsigned char * buf, int bufLen,void * pUserData) = 0;
    ///注销
    virtual void   logout() = 0;
};

}

using namespace koal ;

/**
 * @brief 创建客户端接口
 * @param pUrl 连接的字符串
 */ 
KOAL_EXPORT_MSGLIB ImsgClientSink * createMSGClient(ImsgClient * pClient, const char * pUrl);

/**
 * @brief 释放服务器接口
 */ 
KOAL_EXPORT_MSGLIB void freeMSGClient(ImsgClientSink * pSink); 

#endif
