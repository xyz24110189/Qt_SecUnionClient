#include <utils.h>
#include "msglibs.hpp"
#include <tools.h>
#include <3rd/nano/include/reqrep.h>
#include <3rd/nano/include/pubsub.h>

///线程数
#define  REP_THREAD_COUNT  10
#define  HB_TIME_OUT  (30*1000)
///心跳线程 1个线程
THRED_RETURN __stdcall msgServerSinkHbThread(void * pParam) ;
///repSocket
THRED_RETURN __stdcall repSocketThread(void * pParam) ;
///check timeout
THRED_RETURN __stdcall checkTMThread(void * pParam) ;

namespace koal {
///===================================reqMgr===========================
    void reqMgr::init() {
        unsigned int tid = tool::getTID();
        std::map<unsigned int , reqData *>::iterator iter = m_reqMap.find(tid);
        if(iter != m_reqMap.end()) {
            reqData * pReq = iter->second;
            delete pReq ;
        }
        m_reqMap[tid] = new reqData() ;
    }
    reqMgr::reqData * reqMgr::setReq(clientItem * pItem , repSocket * pSKt , cmdHeader * pHeader) {
        unsigned int tid = tool::getTID();
        std::map<unsigned int , reqData *>::iterator iter = m_reqMap.find(tid);
        if(iter != m_reqMap.end()) {
            reqData * pReq = iter->second ;
            iter->second->fromItem = pItem ;
            iter->second->pHeader  = pHeader ;
            iter->second->pSKT = pSKt ;
            return pReq ;
        }
        return NULL ;
    }
    void reqMgr::setReponse() {
    	unsigned int tid = tool::getTID();
		std::map<unsigned int , reqData *>::iterator iter = m_reqMap.find(tid);
		if(iter != m_reqMap.end()) {
			iter->second->bReponse = true ;
		}
    }
    reqMgr::reqData * reqMgr::getReq() {
        unsigned int tid = tool::getTID();
        std::map<unsigned int , reqData *>::iterator iter = m_reqMap.find(tid);
        if(iter != m_reqMap.end()) {
            return iter->second ;
        }
        return NULL ;
    }
    reqMgr::~reqMgr()  {
        std::map<unsigned int , reqData *>::iterator iter = m_reqMap.begin();
        while (iter != m_reqMap.end()) {
           delete iter->second ;
           iter++;
        }
        m_reqMap.clear();
    }
///===================================repSocket========================
    repSocket::repSocket(msgServerSink * pSink) {
            tThread::tParam param ;
            param.name   = "repSocket";
            param.pParam = this ;
            mThread =  new (std::nothrow)tThread(param);
            mpSink  = pSink;
            mbSign  = repThreadInit ;
    }

	bool repSocket::initSock()
	{
		printf("repSocket::process 1 \n");
		if (!stSocket::init(NN_REP)) {
			mbSign = repThreadFailed;
			return false;
		}
		printf("repSocket::process 2 %d\n", mstrUrl.c_str());

		///连接
		/*if (!connect(mstrUrl.c_str(), 1)) {
			mbSign = repThreadFailed;
			return false;
		}*/
		int bind1Count = 0;
	bindHB_again:
		if (!bind(mstrUrl.c_str()))
		{
			bind1Count++;
			if (bind1Count < 10) {
				goto bindHB_again;
			}
			else {
				return false;
			}
		}

		printf("repSocket::initSock success!!!\n");
		return true;
	}

    bool repSocket::start(const char * pUrl) {
        if(!mThread) {
            return false ;
        }
        printf("repSocket::start......%d  %s\n",pUrl,pUrl);
        mstrUrl = pUrl;
		bool bRet = initSock();
		if (!bRet) return false;
        ///启动线程
        if(!mThread->start(&repSocketThread)) {
            return false ;
        }
        printf("repSocket::finish......\n");
        int nTime = 0 ;
        ///等待1秒
        while ((nTime < 50) && (mbSign == repThreadInit)) {
           tool::sleep(10);
           nTime++ ;
        }
        return (mbSign == repThreadSucc);
    }

    bool repSocket::process() {
        ///初始化
    	/*printf("repSocket::process 1 \n");
        if(!stSocket::init(NN_REP)) {
            mbSign = repThreadFailed ;
            return false ;
        }
        printf("repSocket::process 2 %d\n",mstrUrl.c_str());*/

        ///连接
        /*if(!connect(mstrUrl.c_str(),1)) {
            mbSign = repThreadFailed ;
            return false ;
        }*/

		/*int bind1Count = 0;
	bindHB_again:
		if (!bind(mstrUrl.c_str()))
		{
			bind1Count++;
			if (bind1Count < 10) {
				goto bindHB_again;
			}
			else {
				return false;
			}
		}*/

        printf("repSocket::process 3 \n");
        ///初始化请求REQ数据
        mpSink->initReqData();
        mbSign = repThreadSucc ;
        struct nn_pollfd pfd[1];
        printf("repSocket::process 4 \n");
        while(mbSign == repThreadSucc) {
            pfd[0].fd = mSKT;
            pfd[0].events = NN_POLLIN;
            pfd[0].revents = 0;
            ///10MS轮询一次
            if(nn_poll (pfd, 1, 1000) < 0){
            	 printf("poll error \n");
                 break ;
            }  
            if ((pfd[0].revents & NN_POLLIN) == 0) {
                continue;
            }
            unsigned char * pBuf = NULL ;
            int retLen = recv(&pBuf);
            cmdHeader * pHeader = (cmdHeader *)pBuf;
            ///长度比包头短
            if(retLen < sizeof(cmdHeader)) {
                printf("continue retlen = %d, %d\n",retLen,sizeof(cmdHeader));
                goto next_goon ;
            }
            ///判断是否小于一个完整包
            if(retLen < (sizeof(cmdHeader) + pHeader->datLen)) {
                printf("continue retlen = %d, sum = %d\n",retLen,(sizeof(cmdHeader) + pHeader->datLen));
                goto next_goon ;
            }
            ///调用SINK的响应方法
            mpSink->onClientMsg(pHeader,dynamic_cast<stSocket *>(this));
 next_goon:
 	 	 	if(pBuf)
 	 	 		fremsg(pBuf);
        }
        printf("repSocket::process 5\n");
        mbSign = repThreadClosed;
        return true ;
    }
    void  repSocket::stop() {
        if(mbSign == repThreadSucc) {
            mbSign = repThreadCloseing;
            while (true) {
                if(mbSign == repThreadClosed)
                    break ;
                tool::sleep(5);
            }
        }
        ///关闭SOCKET
        close();
        if(mThread) {
            mThread->stop();
            delete mThread ;
            mThread = NULL ;
        }
    }
///===================================msgServerSink=================================
    bool msgServerSink::initHBReq() {
        std::string proto  = mproto;
        std::string port ;
        int  bind1Count = 0 ;
        int  nport      = mmaxPort ;
        char tmp[32]    = "";
        ///初始化HB
        if(!mHBRep.init(NN_REP)) {
            return false ;
        }
   bindHB_again:
        ///绑定URL
        if(!mHBRep.bind(mstrHBUrl.c_str())) {
			nport++;
			bind1Count++;
			sprintf(tmp, "%d", nport);
			port = tmp;
            if(proto == "tcp" || proto == "ws") {
                mstrHBUrl = proto + "://127.0.0.1:" + port; 
            } else { ///如果不是TCP或者WS， 随机生成
				mstrHBUrl = proto + "://" + tool::getUUID();
            }
            if(bind1Count < 10) {
            	goto bindHB_again;
            } else {
            	return false ;
            }
        }
        ///记录上端口
        mmaxPort = nport;
        ///启动线程
        if(hbThread==NULL) {
            tThread::tParam param ;
            param.name = "hb_thread";
            param.pParam = this ;
            hbThread = new tThread(param);
        }
        printf("hb url = %s , %d\n",mstrHBUrl.c_str(),mmaxPort);
        return hbThread->start(&msgServerSinkHbThread) ;
    }
    void msgServerSink::uninitHBReq() {
        if(hbThread) {
            hbThread->stop();
            delete hbThread ;
            hbThread = NULL ;
        }
    }

    void msgServerSink::checkTimeout() {
    	const int64 timeOUT = 30 * 1000;
    	int64  lastTM = tool::getStartMsec();
    	while(mRunning) {
    		int curTM = tool::getStartMsec();
    		if( (curTM - lastTM) > (timeOUT) ) {
    			tLockHelper helper(&mClientItemLocker);
    			std::map<unsigned int, clientItem *>::iterator iter = mClientItemmMap.begin();
    			while(iter != mClientItemmMap.end()) {
    				clientItem * pItem = iter->second ;
    				if((curTM - pItem->getReqTime()) > timeOUT) {
    					mClientItemmMap.erase(iter++);
						//上层进行处理
						IClientItem *pTempItem = getClientItem(pItem->getAppID());
						mpServer->onLogout(pItem, pTempItem ? false : true);

    					cmdDataLogout  logoutData;
    					logoutData.cliID = pItem->getID();
    					strcpy(logoutData.appName,pItem->getAppName());
    					strcpy(logoutData.appID,pItem->getAppID());
    					pubMsg(0,0,enFrameMsg,enSubmsgLogout,(unsigned char *)&logoutData,sizeof(logoutData));
    					storeItem(pItem);
    				} else
    					iter++;
    			}
    		}
    		tool::sleep(100);
    	}
    }

    void msgServerSink::hbReqProcess() {
        struct nn_pollfd pfd[1];
        while(mRunning) {
            pfd[0].fd = mHBRep.getSKT();
            pfd[0].events = NN_POLLIN;
            pfd[0].revents = 0;
            ///10MS轮询一次
            if(nn_poll (pfd, 1, 1000) < 0){
                 break ;
            }  
            if ((pfd[0].revents & NN_POLLIN) == 0) {
            	printf("hbReqProcess continue\n");
                continue;
            }
            printf("POLLIN 1  \n");
            unsigned char * buff = NULL ;
            int retLen = mHBRep.recv(&buff);
            cmdHeader * pHeader = (cmdHeader *)buff;
            printf("POLLIN data = %d\n",retLen);
            if(retLen < sizeof(cmdHeader)) {
                printf("hbReqProcess:ret(%d) < header(%d)\n",retLen,sizeof(cmdHeader));
                goto  next1_goon ;
            }
            ///判断是否小于一个完整包
            if(retLen < (sizeof(cmdHeader) + pHeader->datLen)) {
                printf("hb continue retlen = %d, sum = %d\n",retLen,(sizeof(cmdHeader) + pHeader->datLen));
                goto next1_goon ;
            }
            ///调用SINK的响应方法
            onClientMsg(pHeader,&mHBRep);
 next1_goon:
 	 	 	if(buff)
 	 	 		mHBRep.fremsg(buff);
        }
        return  ;
    }
 
    bool msgServerSink::initDevice() {
        std::string proto = mproto;
        std::string port;
        int bind1Count = 0 ;
        int nport = mmaxPort;
        char tmp[32] = "";
	bind1_again:
		nport++;
		bind1Count++;
		sprintf(tmp, "%d", nport);
		port = tmp;
        ///如果是TCP或者WS，必然有端口
        if(proto == "tcp" || proto == "ws") {
            mstrUrl = proto + "://127.0.0.1:" + port; 
        } else { ///如果不是TCP或者WS， 随机生成URL
			mstrUrl = proto + "://" + tool::getUUID();
        }

        ///绑定前端URL
        /*if(!mDevice.bind1(NN_REQ,mstrUrl.c_str())) {
            if(bind1Count < 10) {
                goto bind1_again ;
            }
            return false ;
        }*/

        ///绑定后端URL
        /*if(!mDevice.bind2(NN_REP,mstrUrlEx.c_str())) {
            return false ;
        }*/

		///记录上端口
		mmaxPort = nport;
        printf("device url1 = %s, url2 = %s\n",mstrUrl.c_str(),mstrUrlEx.c_str());
        return true ;
    }

    bool msgServerSink::initDeviceRep() {
        uninitDeviceRep();
        ///初始化设备服务
        if(!initDevice()) {
            return false ;
        }
		//mDevice.startDevice();

        printf("initDeviceRep start\n");
        ///连接
        for(int i = 0 ; i < 1 ; i++) {
            repSocket * pSKT = new repSocket(this);
            printf("pSKT->start %d, %s\n",pSKT,mstrUrlEx.c_str());
            if(!pSKT->start(mstrUrl.c_str()/*mstrUrlEx.c_str()*/)) {
                return false;
            }
            mRepSktVt.push_back(pSKT);
        }
        printf("initDeviceRep finish\n");
        return true ;
    }
    void msgServerSink::uninitDeviceRep() {
        ///先释放响应SKT
         for(int i = 0 ; i < mRepSktVt.size() ; i++) {
            repSocket * pSKT = mRepSktVt[i];
            pSKT->stop();
            delete pSKT ;
        }
        mRepSktVt.clear();
    }

    bool msgServerSink::initPub() {
    	printf(" msgServerSink::initPub \n");
		std::string port;
        int  bind1Count = 0 ;
        char tmp[32]="";
        int nprot = mmaxPort;
        if(!mpubReq.init(NN_PUB)) {
        	return false ;
        }
        ///初始化PUB
    bindPub_again:
    	printf(" msgServerSink::bindagain : %d \n",bind1Count);
		bind1Count++;
		nprot++;
		sprintf(tmp, "%d", nprot);
		port = tmp;
        ///如果是TCP或者WS，必然有端口
        if((mproto == "tcp" || mproto == "ws")) {
            mpubStrUrl = mproto + "://127.0.0.1:" + port;
        } else { ///如果不是TCP或者WS， 随机生成URL
			mpubStrUrl = mproto + "://" + tool::getUUID();
        }
        printf("pub url0 = %s\n",mpubStrUrl.c_str());
        ///绑定前端URL
        if(!mpubReq.bind(mpubStrUrl.c_str())) {
            if(bind1Count < 10) {
                goto bindPub_again ;
            }
            return false ;
        }
        printf("pub url = %s\n",mpubStrUrl.c_str());
		///记录上端口
		mmaxPort = nprot;
        return true ;
    }
    void msgServerSink::uninitPub() {
        ///关闭
        mpubReq.close();
    }

    bool  msgServerSink::initServerSink() {
        mRunning = true ;
        std::string proto ;
        std::string port ;
        printf("initServerSink %s\n",mstrHBUrl.c_str());
		if(!tool::getIPCUrl(mstrHBUrl.c_str(), proto, port)) {
			printf("initServerSink return false\n");
			return false ;
		}
		///记录上端口
		mmaxPort = atoi(port.c_str());
		mproto = proto;
		printf("strat msgServerSink::initHBReq() \n");
		///初始化心跳
		if(!initHBReq()) {
			printf("strat msgServerSink::initHBReq() failed\n");
			uninitServerSink();
			return false ;
		}
		//初始化成功后保存信息
		storeChannelInfo(mstrHBUrl);

        printf("strat msgServerSink::initServerSink() initPub\n");
        ///初始化PUB
        if(!initPub()) {
        	printf("strat msgServerSink::initPub() failed\n");
            uninitServerSink();
            return false ;
        }  
        printf("strat msgServerSink::initDeviceRep() \n");
        ///初始化REP
        if(!initDeviceRep()) {
        	printf("strat msgServerSink::initDeviceRep() failed\n");
            uninitServerSink();
            return false ;
        }
        tThread::tParam param ;
		param.name   =  "ckTMThread";
		param.pParam =  this ;
		mcheckThread =  new (std::nothrow)tThread(param);
		if(!mcheckThread->start(&checkTMThread)) {
			return false ;
		}
        printf("initServerSink succed\n");
        return true ;
    }
    void  msgServerSink::uninitServerSink() {
        mRunning = false ;
        printf("uninitServerSink1 \n");
        uninitHBReq();
        printf("uninitServerSink2 \n");
        uninitDeviceRep();
        printf("uninitServerSink3 \n");
        uninitPub();
        printf("uninitServerSink4 \n");
        ///清理数据
        std::map<unsigned int, clientItem *>::iterator iter =  mClientItemmMap.begin();
        while (iter != mClientItemmMap.begin()) {
            delete iter->second;
            iter++;
        }   
        mClientItemmMap.clear();
        printf("uninitServerSink5 \n");
    }
    ///广播消息
    bool  msgServerSink::pubMsg(unsigned int fromID,unsigned int toID,
    		koal::enMainMsg mainMsg, koal::enFrameSubMsg subMsg,unsigned char * buf, int bufLen) {
        tLockHelper locker(&mpubLocker);
        unsigned char * pubuf = mpubReq.getmsg(sizeof(cmdHeader) + bufLen);
        if(pubuf == NULL) {
            return false ;
        }
        cmdHeader * pHeader = (cmdHeader *)pubuf ;
        pHeader->fromID = fromID ;
        pHeader->toID = toID ;
        pHeader->mainMsg = mainMsg;
        pHeader->subMsg = subMsg ;
        pHeader->datLen = bufLen ;
        memcpy(pHeader->data,buf,bufLen);
        unsigned char * ppubuf = (unsigned char *)&pubuf;
        int rc = mpubReq.snd(ppubuf,NN_MSG,NN_DONTWAIT);
        return (rc > 0);
    }
    /**
     * @brief 发送消息到客户端
     * @param  如果为NULL， 则广播该消息到所有客户端
     */
    bool msgServerSink::sendMsg(IClientItem * pCli, unsigned short msg,unsigned char * buf, int bufLen) {
        if(pCli==NULL) { /// 广播
           return  pubMsg(0,0,enCustomMsg,(enFrameSubMsg)msg,buf,bufLen);
        } else { ///单发, 有可能走广播，也有可能走响应
            clientItem * pItem = dynamic_cast<clientItem *>(pCli);
            ///获取请求数据
            reqMgr::reqData * pReq = mreqMgr.getReq();
            if(!pReq) {
            	return pubMsg(0,pItem->getID(),enCustomMsg,(enFrameSubMsg)msg,buf,bufLen);
            }
            ///发送请求和接受请求为同一个客户端
            if(pCli == pReq->fromItem) {
                ///申请内存
                unsigned char * pBuf = pReq->pSKT->getmsg(sizeof(cmdHeader)+bufLen);
                if(pBuf == NULL) {
                    return false ;
                }
                ///设置数据
                cmdHeader * pHeader = (cmdHeader *)pBuf;
                pHeader->idx = pReq->pSKT->getReqid();
                pHeader->mainMsg = enCustomMsg ;
                pHeader->subMsg  = msg ;
                pHeader->fromID = pReq->fromItem->getID();
                pHeader->toID = pHeader->fromID;
                if(bufLen > 0)
                    memcpy(pHeader->data,buf,bufLen);
                pHeader->datLen = bufLen ;
                ///把指针的指针强转为指针
                unsigned char * ppBuf = (unsigned char *)&pBuf;
                int ret = pReq->pSKT->snd(ppBuf,NN_MSG,NN_DONTWAIT);
                ///设置回复标识
                pReq->bReponse = true;
                return !(ret < 0);    
            } else { ///发送请求和接受请求不是同一个客户端     
                return pubMsg(pReq->fromItem->getID(),pItem->getID(),enCustomMsg,(enFrameSubMsg)msg,buf,bufLen);
            }
        }
        return true ; 
    }

    /**
     * @brief 踢掉客户端
     */ 
    void msgServerSink::kickOff(const char * pCliID) {

    }

    IClientItem *  msgServerSink::getClientItem(const char * pAppID) {
        tLockHelper helper(&mClientItemLocker);
        std::map<unsigned int, clientItem *>::iterator iter = mClientItemmMap.begin();
        while(iter != mClientItemmMap.end()) {
            clientItem * pItem = iter->second ;
            ///判断应用名
            if( strcmp(pItem->getAppID(),pAppID) == 0 )
                return  dynamic_cast<IClientItem *>(pItem);
            iter++;
        }
        return NULL ;
    }

    unsigned int msgServerSink::getRandID() {
       static unsigned int __id = tool::getLocalTime();
       __id = __id + rand() % 100 ;
       return __id ;
    }

    ///登录
    bool msgServerSink::onLogin(cmdHeader * pHeader,IClientItem * pItem) {
        ///判断长度数据长度是否符合
        if(pHeader->datLen != sizeof(cmdDataLogin)) {
            return false ;
        }
        cmdDataLogin * pLogon = (cmdDataLogin *)(pHeader->data) ;
		pLogon->appID[APPNAME_LEN + 1] = 0;
        printf("dateLen = %d, %d, %s\n",pHeader->datLen,pLogon,pLogon->appID);
        ///判断是否登录
        if(pItem)
        	pItem = getClientItem(pLogon->appID);
        printf("pItem = %d \n",pItem);
        cmdDataLoginResult result ;

        memset(&result,0,sizeof(cmdDataLoginResult));
        if(pItem) { ///重复登录
            result.error = enFrameErrreLogon;
            sprintf(result.reqUrl,"already logon");
            printf("relogin  ..... %s\n",pLogon->appID);
        } else {
            clientItem * item =  getFreeItem();
            item->setAppinfo(pLogon->appName,pLogon->appID,pLogon->appToken);
            item->setLoginfo(getRandID() ,tool::getUUID().c_str() ,tool::getUUID().c_str());
            item->setID(getRandID());
            char buffer[128] = "";
            ///让上层做一次过滤
            if(!mpServer->onLogin(item,buffer)) {
                result.error = enFrameLogonErrorFromUser;
                sprintf(result.reqUrl,"%s",buffer);
            } else {
            	printf("login succ session = %d , %s, %s\n", item->getSession().sessionID,mstrUrl.c_str(),mpubStrUrl.c_str());
				result.id = item->getID();
                result.sessionID = item->getSession().sessionID;
                strcpy(result.ticket,item->getSession().ticket.c_str());
                strcpy(result.reqUrl,mstrUrl.c_str());
                strcpy(result.pubUrl,mpubStrUrl.c_str());
                strcpy(result.topic,item->getSession().topic.c_str());
                result.hbTimeOut = HB_TIME_OUT;
            }
            {   ///增加客户端
                tLockHelper helper(&mClientItemLocker);
                mClientItemmMap[item->getID()] = item;
				item->updateReqTime(tool::getStartMsec());
            }  
        } 
        return sndFrameMsg(enSubmsgLogin,(unsigned char *)&result,sizeof(result)) ;
    }
    ///发送框架消息
    bool msgServerSink::sndFrameMsg(unsigned int subCmd,unsigned char * buf, int bufLen) {
    	printf("sndFrameMsg start \n");
        unsigned char * pbuf = mHBRep.getmsg(sizeof(cmdHeader) + bufLen);
        if(pbuf==NULL) return false ;
        cmdHeader * pHeader = (cmdHeader *)pbuf;
        pHeader->mainMsg = enFrameMsg;
        pHeader->datLen = bufLen;
        if(bufLen)
            memcpy(pHeader->data,buf,bufLen);
        unsigned char * pbbuf = (unsigned char *)&pbuf;
        int rc = mHBRep.snd(pbbuf,NN_MSG,NN_DONTWAIT);
        printf("sndFrameMsgdfg fin %d , %d \n",rc,sizeof(cmdHeader) + bufLen);
        return (rc > 0);
    }

    ///登出
    bool msgServerSink::onLogOut(cmdHeader * pHeader) {
        clientItem * pItem = NULL ;
        {
        	tLockHelper helper(&mClientItemLocker);
        	std::map<unsigned int, clientItem *>::iterator iter = mClientItemmMap.find(pHeader->fromID);
			if(iter == mClientItemmMap.end()) {
				return false ;
			}
			pItem = iter->second ;
        }
        if(!pItem) {
        	return false ;
        }

		//上层进行处理
		mpServer->onLogout(pItem, true);
		//response;
		sndFrameMsg(enAutoResp, NULL, 0);
        /// pub msg
    	cmdDataLogout  logoutData;
		logoutData.cliID = pItem->getID();
		strcpy(logoutData.appName,pItem->getAppName());
		strcpy(logoutData.appID,pItem->getAppID());
        pubMsg(0,0,enFrameMsg,enSubmsgLogout,(unsigned char *)&logoutData,sizeof(logoutData));
        /// store item
        storeItem(pItem);
        return true ;
    }

    ///心跳
    bool msgServerSink::onHb(cmdHeader * pHeader) {
        if(pHeader->datLen < sizeof(cmdDataHb)) {
        	return false ;
        }
        clientItem * pItem =  getClientItem(pHeader->fromID);
        if(!pItem) {
        	return false ;
        }
        /// update last active time
        pItem->updateReqTime(tool::getStartMsec());
		sndFrameMsg(enAutoResp, NULL, 0);
		mpServer->onHb(pItem);
        return true ;
    }

    bool msgServerSink::onClientMsg(cmdHeader * pHeader,stSocket * pSkt) {
        ///找到客户端指针
    	printf("msgServerSink::onClientMsg strart \n");
        clientItem * pItem =  getClientItem(pHeader->fromID);
        if(pItem) {
        	pItem->updateReqTime(tool::getStartMsec());
        }
        printf("msgServerSink::onClientMsg fin %d\n",pItem);
        if(pItem == NULL){
            ///不是框架消息，返回错误
            if(pHeader->mainMsg != enFrameMsg) {
                 return false ;      
            } else {
                ///不是登录消息返回错误
                if(pHeader->subMsg != enSubmsgLogin) {   
                    return false ;
                }
            }
        }
        ///框架消息
        if(pHeader->mainMsg == enFrameMsg) {
            switch(pHeader->subMsg) {
                case enSubmsgLogin: {
                    return onLogin(pHeader,pItem) ;
                }
                case enSubHb: {
                    return onHb(pHeader) ;
                }
                case enSubmsgLogout: {
                    return onLogOut(pHeader) ;
                }
            }
            return false ;
        } else if(pHeader->mainMsg == enCustomMsg){ ///自定义消息
            repSocket * pReqSKT = dynamic_cast<repSocket *>(pSkt);
            reqMgr::reqData * pReq =  mreqMgr.setReq(pItem,pReqSKT,pHeader);
            pReqSKT->setReqid(pHeader->idx);
            bool ret = mpServer->onClientMsg(dynamic_cast<IClientItem *>(pItem),
                                             pHeader->subMsg,pHeader->data,
                                             pHeader->datLen);
            ///判断是否回复,如果没有回复，自动回复                              
            if(!pReq->bReponse) {
                cmdHeader data ;
                memset(&data,0,sizeof(cmdHeader));
                data.idx = pReq->pHeader->idx ;
                data.mainMsg = enFrameMsg ;
                data.subMsg  = enAutoResp ;
                pReq->pSKT->snd((const unsigned char *)&data,sizeof(cmdHeader),NN_DONTWAIT);
            }
            pReq->reset();
        } else {
            printf("unkown mainMsg: %d\n",pHeader->mainMsg);
            return false ;
        }
        return true;
    }

    clientItem * msgServerSink::getClientItem(unsigned int id) {
        tLockHelper helper(&mClientItemLocker);
        std::map<unsigned int, clientItem *>::iterator iter = mClientItemmMap.find(id);
        if(iter == mClientItemmMap.end()) {
            return NULL ;
        }
        clientItem * pItem = iter->second ;
        return pItem ;
    }

    clientItem *  msgServerSink::getFreeItem() {
    	tLockHelper helper(&mfreeClientItemLocker);
    	if(mFreeClientItemList.size() > 0) {
    		std::list<clientItem *>::iterator iter = mFreeClientItemList.begin();
    		clientItem * pItem = *iter ;
    		mFreeClientItemList.erase(iter);
    		return pItem ;
    	}
    	return new clientItem() ;
    }
    void         msgServerSink::storeItem(clientItem * pItem) {
    	tLockHelper helper(&mfreeClientItemLocker);
		std::list<clientItem *>::iterator iter = std::find(mFreeClientItemList.begin(), mFreeClientItemList.end(), pItem);
		if (iter == mFreeClientItemList.end())
			mFreeClientItemList.push_back(pItem);
    }
    ///========================clientItem=======================================
}

using namespace koal ;

///心跳线程 1个线程
THRED_RETURN __stdcall msgServerSinkHbThread(void * pParam)  {
	tThread::tParam * _pParam = (tThread::tParam *) pParam;
    msgServerSink * pSink = (msgServerSink * )_pParam->pParam;
    pSink->hbReqProcess();
    return 0 ;
}

THRED_RETURN __stdcall repSocketThread(void * pParam) {
	tThread::tParam * _pParam = (tThread::tParam *) pParam;
    repSocket * psocket = (repSocket *)_pParam->pParam;
    psocket->process();
    return 0 ;
}

THRED_RETURN __stdcall checkTMThread(void * pParam) {
	tThread::tParam * _pParam = (tThread::tParam *) pParam;
	msgServerSink * pSink = (msgServerSink * )_pParam->pParam;
	pSink->checkTimeout();
	return 0 ;
}
