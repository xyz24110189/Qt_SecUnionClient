
#ifndef THIS_IS_MSG_LIBS_FROM_SHARP_YOUNG_20200110_1918
#define THIS_IS_MSG_LIBS_FROM_SHARP_YOUNG_20200110_1918

#include <msglib/msglibs.h>
#include "deviceskt.h"
#include <stdlib.h>
#include <vector>
#include "cmd.h"
#include <map>
#include <list>
#include <locker.hpp>
#include <tools.h>

class tThread ;
namespace koal {
    class msgServerSink ;
    class clientItem ;
    class repSocket ;

    ///请求管理
    class reqMgr {
    public:
        struct reqData {
            ///请求发来的
            clientItem  *  fromItem ;
            cmdHeader   *  pHeader ;
            ///套接字
            repSocket   *  pSKT ;
            ///是否响应
            bool           bReponse;
            reqData() {
                reset();
            }
            void reset() {
                fromItem = NULL ;
                pHeader  = NULL ;
                bReponse = false ;
                pSKT = NULL ;
            }
        };
        ~reqMgr() ;
    public:
        void init() ;
        reqData * setReq(clientItem * pItem,  repSocket * pSKt ,cmdHeader * pHeader) ;
        void setReponse();
        reqData * getReq();
    private:
        ///初始化完毕，使用中都在读，故不用加锁
        std::map<unsigned int , reqData *>  m_reqMap ;
    };

    class repSocket :virtual public stSocket {
        enum {
            ///初始状态
            repThreadInit ,
            ///失败状态
            repThreadFailed,
            ///成功状态
            repThreadSucc,
            ///准备关闭
            repThreadCloseing,
            ///关闭完成
            repThreadClosed,
        };
        public:
        repSocket(msgServerSink * pSink);
        msgServerSink * getSink() {
            return mpSink;
        }

		bool initSock();
        bool start(const char * pUrl) ;
        bool process();
        void stop() ;
        void setReqid(unsigned int idx) {
            mReqID = idx ;
        }
        unsigned int getReqid() {
            return mReqID ;
        }
        private:
            std::string     mstrUrl ;
            /// init 1, failed 2 , succ 3
            volatile  char  mbSign ;
            ///线程指针
            tThread   *     mThread ;
            ///msgsink指针
            msgServerSink * mpSink;
            ///当前的请求ID
            unsigned int    mReqID;
    };

    class clientItem : public IClientItem  {
        public:
            struct appInfo {
                std::string  name ;
                std::string  id ;
                std::string  token ;
            };
            struct loginfo {
                unsigned int sessionID;
                std::string  ticket ;
                std::string  topic ;
            };
        public:
            const  char *  getAppName() {
                return app.name.c_str();
            }
            const char *  getAppID() {
                return app.id.c_str();
            }
			unsigned int getID() {
				return mcliID;
			}
        public:
            void setAppinfo(const char * pAppName , const char * pAppID , const char * pToken) {
                app.name = pAppName ;
                app.id   = pAppID ;
                app.token = pToken ;
            }
            void setLoginfo(unsigned int sessionID, const char * ticket, const char * topic) {
                sessionData.sessionID = sessionID ;
                sessionData.ticket = ticket ;
                sessionData.topic = topic ;
            }
            const appInfo & getApp() {
                return app ;
            }
            const loginfo & getSession() {
                return sessionData;
            }
            void   setID(unsigned int id) {
                mcliID = id ;
            }
            void  updateReqTime(int64  tm) {
            	mlastReqTime = tm ;
            }
            int64 getReqTime() {
            	return mlastReqTime;
            }
        private:
            appInfo      app ;
            loginfo      sessionData ;
            unsigned int mcliID;
            int64        mlastReqTime ;
    };

    class msgServerSink : public ImsgServerSink {
        public:
             /**
             * @brief 发送消息到客户端
             * @param  如果为NULL， 则广播该消息到所有客户端
             */
            virtual bool  sendMsg(IClientItem * pCli, unsigned short msg,unsigned char * buf, int bufLen);

            /**
             * @brief 踢掉客户端
             */ 
            virtual void  kickOff(const char * pCliID) ;
            /**
             * @brief 获取客户端指针
             */ 
            virtual IClientItem * getClientItem(const char * pAppID); 
        public:
            msgServerSink(ImsgServer * pServer,const char * pUrl) {
                mpServer   = pServer ;
                mstrHBUrl  = pUrl ;
                mstrUrlEx  = "ipc://" + tool::getUUID();
                mmaxPort = 0 ;
                hbThread = NULL;
                mRunning = false;
                mcheckThread = NULL ;
            }
            ///初始化服务
            bool  initServerSink() ;
            ///反初始化
            void  uninitServerSink();
            ///
            bool  onClientMsg(cmdHeader * pHeader, stSocket * pSkt);
            ///初始化REQ
            void  initReqData() { mreqMgr.init(); }
        public:
            ///thread func
            void  hbReqProcess();
            ///登录
            bool  onLogin(cmdHeader * pHeader,IClientItem * pItem);
            ///登出
            bool  onLogOut(cmdHeader * pHeader);
            ///心跳
            bool  onHb(cmdHeader * pHeader);
            ///timeout check
            void  checkTimeout();
        protected:
            ///===================================
            bool  initHBReq();
            void  uninitHBReq();
            
            ///===================================
            bool  initDevice();
            bool  initDeviceRep();
            void  uninitDeviceRep() ;
            
            ///===================================
            bool  initPub();
            void  uninitPub();

            ///====================================
            bool  pubMsg(unsigned int fromID,unsigned int toID,
            		koal::enMainMsg, koal::enFrameSubMsg,unsigned char * buf, int bufLen);
            ///获取随机ID
            unsigned int getRandID();

            bool  sndFrameMsg(unsigned int subCmd,unsigned char * buf, int bufLen);
        private:
            ///服务指针
            ImsgServer  * mpServer  ;
            ///REQ的URL front
            std::string   mstrUrl   ;
            ///REP URL
            std::string   mstrUrlEx  ;
            ///心跳的URL
            std::string   mstrHBUrl  ;
            ///PUB的URL   
            std::string   mpubStrUrl ;
            ///绑定得最大得端口记录
            unsigned int  mmaxPort  ;
            ///记录上协议
            std::string   mproto ;
            ///消息路由设备
            deviceskt     mDevice   ;
            stSocket      mHBRep    ;
            tThread  *    hbThread  ; 
            std::vector<repSocket *>  mRepSktVt ;
            ///广播锁
            tLocker       mpubLocker;
            stSocket      mpubReq;
            ///运行标识
            volatile bool mRunning;
        private:
            clientItem *  getFreeItem();
            void          storeItem(clientItem *);
        private:
            std::map<unsigned int, clientItem *>  mClientItemmMap;
            tLocker      mClientItemLocker;
            std::list<clientItem *> mFreeClientItemList;
            tLocker      mfreeClientItemLocker;
            clientItem * getClientItem(unsigned int id);
            ///请求管理 
            reqMgr       mreqMgr ;
            ///检查超时的线程
            tThread  *   mcheckThread ;
    };
}

#endif
