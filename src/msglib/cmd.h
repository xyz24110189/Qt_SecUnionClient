/**
 *  @file  cmd.h
 *  @brief 内部得消息头文件
 *  @date  2020年01月02日
 *  @author sharp
 *  @email  yangcp@koal.com
 */ 
#ifndef THIS_IS_MSGLIC_CMD_HEADER_FROM_YCP_AT_202001021453_fdsddfdsa
#define THIS_IS_MSGLIC_CMD_HEADER_FROM_YCP_AT_202001021453_fdsddfdsa
#pragma pack(push, 1)

namespace  koal  {
#define  BUILD_PARAM(buf,mMsg,sMsg,reqid,reqData) \
        { cmdHeader * pHeader = (cmdHeader *)buf ; \
          pHeader->idx = reqid ; \
          pHeader->mainMsg = mMsg; \
          pHeader->subMsg  = sMsg ; \
          pHeader->datLen  = sizeof(reqData) ; \
          memcpy(pHeader->data,&reqData,pHeader->datLen); }

#define  BUILD_PARAM_EX(buf,mMsg,sMsg,reqid,reqData,len) \
        { cmdHeader * pHeader = (cmdHeader *)buf ; \
          pHeader->idx = reqid ; \
          pHeader->mainMsg = mMsg; \
          pHeader->subMsg  = sMsg ; \
          pHeader->datLen  = len ; \
          memcpy(pHeader->data,reqData,pHeader->datLen); }

       

#define  APPNAME_LEN  36

    ///主消息定义
    enum  enMainMsg {
        ///框架消息
        enFrameMsg,
        ///用户自定义消息C
        enCustomMsg,
    };

    ///enFrameMsg  子消息号
    enum enFrameSubMsg {
        ///登录
        enSubmsgLogin,
        ///心跳
        enSubHb,
        ///登出
        enSubmsgLogout,
        ///自动回复
        enAutoResp,
    };

    ///enCustomMsg 子消息号
    enum enCustomSubMsg {

    };

    ///错误编号
    enum enFrameError {
        enFrameErrreLogon = 1,
        enFrameLogonErrorFromUser,
    };

    struct cmdHeader  {
        ///消息编号
        unsigned int   idx ;
        ///框架消息
        unsigned short mainMsg;
        ///自定义消息
        unsigned short subMsg;
        ///发送消息的ID 0 is system
        unsigned int   fromID;
        ///接收的ID
        unsigned int   toID ;
        ///data的长度
        unsigned int   datLen ;
        unsigned char  data[0];
    };
    ///
    struct cmdBaseResult {
    	 ///0为成功，非零为失败
    	 unsigned short error ;
    };

    inline int getCmdLen(int len) {
        return sizeof(cmdHeader) + len;
    }

    /**
     * @brief 登录
     */ 
    struct cmdDataLogin {
        ///应用名称
        char appName[APPNAME_LEN + 1];
        ///应用ID
        char appID[APPNAME_LEN + 1];
        ///应用令牌
        char appToken[APPNAME_LEN + 1];
        ///客户端版本 
        unsigned int  version ;
    };
    struct cmdDataLoginResult : public cmdBaseResult{
        ///id
        unsigned int   id    ;
        unsigned int   sessionID ;
        char  ticket[33];
        ///req socket use 如果error非零，则为错误提示
        char  reqUrl[128];
        ///广播专用
        char  pubUrl[128];
        char  topic[33] ;
        ///心跳间隔，秒
        unsigned short hbTimeOut ;
    };

    /**
     * @brief 心跳
     */ 
    struct cmdDataHb {
    	///cliid
    	unsigned int   cliid;
    	///session id
    	unsigned int   sessionID ;
    	///ticket
    	char  ticket[33];
    	///localTime
    	unsigned int   localTime ;
    };

    /**
     * @brief logout
     */
    struct cmdDataLogout {
    	///cliID
    	unsigned int  cliID ;
    	///应用名称
    	char appName[APPNAME_LEN + 1];
    	///应用ID
    	char appID[APPNAME_LEN + 1];
    };
}
#pragma pack(pop) 
#endif
