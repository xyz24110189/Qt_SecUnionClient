/**
 * @brief ImsgClientSink接口实现
 * sharp.young
 * yangcp@koal.com
 * 20200108
 */ 
#ifndef THIS_IS_MSGLIBC_HPP_FROM_SHARP_20200108_dasfda
#define THIS_IS_MSGLIBC_HPP_FROM_SHARP_20200108_dasfda

#include <plog/Log.h>
#include <msglib/msglibc.h>
#include <3rd/nano/include/reqrep.h>
#include <3rd/nano/include/pubsub.h>
#include "cmd.h"
#include <reqTreadPool.hpp>
#include <tools.h>
#include "skt.hpp"
#include <string>
#include <time.h>

///线程执行方法
static  THRED_RETURN __stdcall msgClientSinkHbThread(void * pParam) ;

namespace koal {
    class msgClientSink : public ImsgClientSink {
        enum {
            enlogout,
            enlogin,
        };
        ///服务器数据
        struct stSessionData {
        	unsigned int   id;
            unsigned int   sessionID;
            std::string    ticket;
            std::string    hbUrl;
            std::string    pubUrl;
            std::string    topic;
            unsigned short hbTimeOut ;
            stSessionData() {
            	id = 0 ;
            	sessionID = 0 ;
            	hbTimeOut = 0;
            }
            void set(cmdDataLoginResult & result) {
                sessionID = result.sessionID;
                ticket = result.ticket;
                hbUrl  = result.reqUrl;
                pubUrl = result.pubUrl;
                topic = result.topic;
                hbTimeOut = result.hbTimeOut;
                id = result.id;
            }
        };
        public:
            ///登录认证
            virtual bool   login(const char * pAppName, const char * pAppID, const char * pAppTokn, bool isOuter = false)  {
				///外部接入客户端间隔位40秒
				if (isOuter) nInterval = 8; 
            	///因为调用不频繁，可以用同一个临界区保护
				LOG_INFO << "login appName = " << pAppName << " appId = " << pAppID << " appToken = " << pAppTokn;
                tLockHelper helper(&msndLocker); 
                if(mClientStatus == enlogin) {
                	char err[128] = "";
					sprintf(err,"logined");
					mpClient->onError(ERR_PARAMTER,err);
					return false;
				}

                if(strlen(pAppName) > APPNAME_LEN || strlen(pAppID) > APPNAME_LEN || strlen(pAppTokn) > APPNAME_LEN) {
                    char err[128] = "";
                    sprintf(err,"参数过长,参数长度不能超过%d个字节.",APPNAME_LEN);
                    mpClient->onError(ERR_PARAMTER,err);
                    return false ;
                }
                char buffer[256]="";
                cmdDataLogin loginParam ;
				memset(&loginParam, 0, sizeof(cmdDataLogin));
                strcpy(loginParam.appID,pAppID);
                strcpy(loginParam.appName,pAppName);
                strcpy(loginParam.appToken,pAppTokn);
				_loginParam = loginParam;

                int reqID = getIdx();
                BUILD_PARAM(buffer,enFrameMsg,enSubmsgLogin,reqID,loginParam);
                char resultBuf[1024]="";
                memset(&resultBuf,0,sizeof(resultBuf));
                ///发送登录请求
                if(!sendToServer(true,(cmdHeader *)buffer,resultBuf,sizeof(cmdDataLoginResult))) {
					LOG_ERROR << "sendToServer error";
                    return false;
                }
                ///数据拷贝
                cmdDataLoginResult result;
                memcpy(&result,resultBuf,sizeof(cmdDataLoginResult));
                if(result.sessionID == 0) {
					LOG_ERROR << "SessionID = 0";
                    return false ;
                }
                sessionData.set(result) ;
                ///连接心跳
                if(!mreqSKT.connect(sessionData.hbUrl.c_str())) {
					LOG_ERROR << "mreqSKT.connect false  url = " << sessionData.hbUrl.c_str();
                    return false ;
                }
                ///连接广播
                if(!msubSKT.connect(sessionData.pubUrl.c_str(),1)) {
					LOG_ERROR << "msubSKT.connect false  url = " << sessionData.pubUrl.c_str();
                    return false ;
                }
				/*  We want all messages, so just subscribe to the empty value. */
				if (nn_setsockopt(msubSKT.getSKT(), NN_SUB, NN_SUB_SUBSCRIBE, "", 0) < 0) {
					LOG_ERROR << "nn_setsockopt: " << nn_strerror(nn_errno());
					nn_close(msubSKT.getSKT());
					return false;
				}
                ///启动监听线程
                if( hbExit) {
                    hbExit = false ;
                    if(!hbThread->start(&msgClientSinkHbThread)) {
                        sessionData.sessionID = 0 ;
                        return false ;
                    }
                }
                mClientStatus = enlogin;
                return true;
            }

			///断线重连
			virtual bool reLogin()
			{
				LOG_INFO << "reLogin begin. reloginCount = " <<  reConnectCount / nInterval;
				if (!mhbSKT.connect(mstrUrl.c_str()))
				{
					LOG_ERROR << "mreqSKT.connect false  url = " << sessionData.hbUrl.c_str();
					return false ;
				}

				mClientStatus = enlogout;
				//_loginParam
				bool bRet = login(_loginParam.appName, _loginParam.appID, _loginParam.appToken, nInterval == 8 ? true : false);
				if (bRet) mpClient->onReLogin(true);
				LOG_INFO << "reLogin end..";
				return true;
			}

            ///发送消息
            virtual bool sendMsg(unsigned short msg, unsigned char * buf, int bufLen, void * pOut,int outLen)  {
                tLockHelper helper(&msndLocker); 
                int sndLen = sizeof(cmdHeader) + bufLen ;
                ///申请发送缓存
                if(mSendBufferLen < sndLen) {
                    if(mpSndBuffer) {
                        delete []mpSndBuffer;
                    }
                    mpSndBuffer = new  (std::nothrow) unsigned char [sndLen];
                }
                if(mpSndBuffer==NULL) {
                    char err[128] = "" ;
                    sprintf(err, "buffer空间创建失败！！");
                    mpClient->onError(ERR_PARAMTER,err) ;
                    return false ;
                }
                BUILD_PARAM_EX(mpSndBuffer,enCustomMsg,msg,getIdx(),buf,bufLen);
                return sendToServer(false,(cmdHeader *)mpSndBuffer,pOut,outLen);
            }
            ///发送消息
            virtual bool   postMsg(unsigned short msg,unsigned char * buf, int bufLen,void * pUserData) {
            	return true ;
            }
            ///注销
            virtual void   logout() {
                ///因为调用不频繁，可以用同一个临界区保护
                tLockHelper helper(&msndLocker); 
                mClientStatus = enlogout ;
                cmdHeader header ;
                memset(&header,0,sizeof(cmdHeader));
                header.idx = getIdx();
                header.mainMsg = enFrameMsg;
                header.subMsg = enSubmsgLogout;
                sendToServer(true,&header,NULL,0);
            } 
        public:
           msgClientSink(ImsgClient * pClient,const char * pUrl) {
               mpClient = pClient ;
               hbThread = NULL ;
               mstrUrl = pUrl ;
               hbExit = true ;
               mSendBufferLen = 0 ;
               mpSndBuffer = NULL ;
               mClientStatus = enlogout;
			   reConnectCount = 0;
			   nInterval = 6;
           }
           bool  initClient() {
        	   LOG_INFO << "initClient  start...";
               if(!hbThread) {
            	    LOG_INFO << "create thread";
                    tThread::tParam param ;
                    param.name =   "msgClientSink_hb";
                    param.pParam = this ;
                    hbThread = new tThread(param);
               }
               LOG_INFO << "mreqSKT.init REQ";
               ///初始化
               if(!mreqSKT.init(NN_REQ)) {
                   return false ;
               }
               LOG_INFO << "mhbSKT.init REQ";
               ///初始化
               if(!mhbSKT.init(NN_REQ)) {
                   return false ;
               }
               LOG_INFO << "msubSKT.init SUB";
               ///初始化
               if(!msubSKT.init(NN_SUB)) {
                   return false ;
               }
               LOG_INFO << "mhbSKT.connect url = " << mstrUrl.c_str();
               ///连接服务器
			   LOG_INFO << "initClient  end...";
               return mhbSKT.connect(mstrUrl.c_str());
           }
           void  uninitClient() {
			   hbExit = true;
               if(hbThread) {
                   hbThread->stop();
                   delete hbThread ;
                   hbThread = NULL ;
               }
			   mreqSKT.close();
			   mhbSKT.close();
			   msubSKT.close();
               if(mpSndBuffer) {
                  delete []mpSndBuffer;
                  mSendBufferLen = 0 ;   
               }
           }
           ~msgClientSink() {
               //uninitClient();
           }
           ///发送消息到服务器 
           bool  sendToServer(bool bframe, cmdHeader * pHeader,void * pOut,int outlen) {
			   printf("sendToServer begin...\n");
               int len = sizeof(cmdHeader) + pHeader->datLen;
               pHeader->fromID = sessionData.id;
               stSocket * pSKT = NULL ;
               printf("1 is frame message = %d\n",bframe);
               if(bframe) {
            	   pSKT = &mhbSKT;
               } else {
            	   pSKT = &mreqSKT;
               }
               printf("2 send message!");
               if(pSKT->snd((const unsigned char *)pHeader,len) != len) {
				   LOG_ERROR << "send messsage failed!  brame = " <<  bframe;
                   return false ;
               }
               printf("3 receive message!\n");
               unsigned char * pBuf = NULL ;
               int retLen = pSKT->recv(&pBuf);
               if(retLen < 0) {
				    LOG_ERROR << "receive message failed! retLen = " << retLen;
                   return false ;
               }
               printf("4 buffer size =  %d , receive message size = %d\n",(sizeof(cmdHeader) + outlen),retLen);
               if((sizeof(cmdHeader) + outlen) > retLen) {
            	   pSKT->fremsg(pBuf);
                   return false ;
               }
               printf("5  ...\n");
               cmdHeader * recvHeader = (cmdHeader *)pBuf ;
               if(outlen > recvHeader->datLen) {
            	   pSKT->fremsg(pBuf);
            	   return false ;
               }
               printf("6  response message deal final...\n");
               if(pOut || outlen)
            	   memcpy(pOut,recvHeader->data,outlen);
               ///释放内存
               if(pBuf) {
            	   pSKT->fremsg(pBuf);
               }
			   printf("sendToServer begin...\n");
               return true;
           }
           unsigned  int  getIdx() {
               static unsigned  int g_idx = time(NULL);
               return g_idx;
           }
           ///心跳推送处理
           void  hbProcess() {
        	   struct nn_pollfd pfd[1];
               unsigned int sessID = sessionData.sessionID;
               int64 lastHbTime = tool::getStartMsec();
               while(!hbExit) {
                   int64 curTime = tool::getStartMsec();
                   ///判断是否达到心跳条件
                   if((curTime - lastHbTime) > 5000/*sessionData.hbTimeOut * 1*/) {
                	    tLockHelper helper(&msndLocker);
                	    char buffer[1024]="";
                	    cmdDataHb hbData ;
                	    hbData.sessionID = sessID;
                	    strcpy(hbData.ticket,sessionData.ticket.c_str());
                	    hbData.localTime = tool::getLocalTime();
                        ///心跳;
                	    int reqID = getIdx();
                	    BUILD_PARAM(buffer,enFrameMsg,enSubHb,reqID,hbData);
                        if(!sendToServer(true,(cmdHeader *)buffer,NULL,0)) {
                        	printf("heart beat msg send error\n");
							mpClient->onReLogin(false);
							reConnectCount += 1;
							if (reConnectCount % nInterval == 0)
								reLogin();
                        }
                        lastHbTime = curTime ;
                   }
                   ///收取广播消息
                   {
                	   ///收到的数据长度要大于最小需要的长度
					   pfd[0].fd = msubSKT.getSKT();
					   pfd[0].events = NN_POLLIN;
					   pfd[0].revents = 0;
					   ///10MS轮询一次
					   if(nn_poll (pfd, 1, 1000) < 0){
							break ;
					   }
					   if ((pfd[0].revents & NN_POLLIN) == 0) {
						   printf("hbProcess continue\n");
						   continue;
					   }
                       unsigned char * pbuf = NULL ;
                       int retLen = msubSKT.recv(&pbuf);
                       if(retLen < 0) {
                           continue ;
                       }
                       cmdHeader * pHeader = (cmdHeader * )pbuf;
                       if(retLen >= (sizeof(cmdHeader) + pHeader->datLen))
                       {
                            ///判断是否广播 或者是发给自己的
                            if((pHeader->toID == 0) || (pHeader->toID == sessionData.id)) {
                                if(pHeader->mainMsg == enFrameMsg) {
                                    onFrame(pHeader->subMsg,pHeader->data,pHeader->datLen);
                                } else if(pHeader->mainMsg == enCustomMsg) {
                                    mpClient->onServerMsg(pHeader->subMsg,pHeader->data,pHeader->datLen);   
                                }
                            } 
                       }
                   }
                   tool::sleep(10);     
               }
			   LOG_INFO << "hbProcess exit!";
           }
        protected:
           bool onFrame(unsigned short msg,unsigned char * buf,int bufLen) {
        	   switch(msg) {
        	   case enSubmsgLogin: {

        		   	   break ;
        	   	   }
        	   case enSubHb: {
        		   	   break ;
        	   	   }
        	   case enSubmsgLogout: {
        		   	   break ;
        	   	   }
        	   case enAutoResp: {
        		   	   break ;
        	   	   }
        	   }
               return true ;
           }
        private:
           ///表示心跳线程是否退出
           volatile bool   hbExit ;
		   volatile unsigned short reConnectCount;
		   volatile unsigned short nInterval;
		   
		   cmdDataLogin _loginParam;

           std::string  mstrUrl ;
           ///
           ImsgClient * mpClient ;
           ///异步发送线程池
           reqThreadPool<cmdHeader> mAsyncSendPool ;
           ///send msg locker
           tLocker      msndLocker ;
           ///session data
           stSessionData sessionData; 
           tThread  *   hbThread ;  
           ///请求SKT
           stSocket     mreqSKT;
           stSocket     mhbSKT ;
           stSocket     msubSKT;
           ///发送缓存
           unsigned char * mpSndBuffer ;
           int          mSendBufferLen ;
           ///状态
           int          mClientStatus;
    };
}
using namespace koal ;

THRED_RETURN __stdcall msgClientSinkHbThread(void * pParam) {
	tThread::tParam * _pParam = (tThread::tParam * )pParam ;
    msgClientSink * pSink = (msgClientSink * )_pParam->pParam ;
    pSink->hbProcess();
	return 0;
}

#endif
