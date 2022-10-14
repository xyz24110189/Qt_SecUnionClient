/**
 *  @file reqThreadPool.hpp 
 *  @brief 请求的线程池模板类
 *  @date 2019年12月5日
 *  @author sharp
 *  @email  yangcp@koal.com
 */
#ifndef  THIS_IS_REQTREADPOOL_HEADER_FROM_201912052203
#define  THIS_IS_REQTREADPOOL_HEADER_FROM_201912052203

#include "locker.hpp"
#include "tThread.hpp"
#include "semer.hpp"
using namespace koal;
///TREQ 类中不能含有需要深拷贝的类
template<class TREQ>
class reqThreadPool {
public:
    struct msg {
        int  svc ;
        TREQ * pReq ;
        void * pData ;
    };
    struct trdChannel {
        ///是否运行
        volatile  bool  bruning ;
        ///是否繁忙
        volatile  bool  bbusy ;
        reqThreadPool * pPool ;
        threadProchelper  func ;
        tThread  *  reqThread ;
        tLocker     reqLocker ;
        tSemer      reqSemer;
        void     *  pUserData ;
        std::list<msg> reqList ;
        trdChannel() {
            func = NULL;
            reqThread = NULL ;
            pPool = NULL ;
            bruning = false ;
            bbusy = false ;
            pUserData = NULL;
        }

        bool push(int svc , TREQ * pReq , void * pData) {
            {
                tLockHelper  helper(&reqLocker);
                msg _msg ;
                _msg.svc  = svc;
                _msg.pReq = pReq ;
                _msg.pData = pData ;
                reqList.push_back(_msg);
            }
            reqSemer.post();
            return true ;
        }

        bool pop(msg & _msg) {
            bool bGet = getMsg(_msg);
            while(!bGet) {
                reqSemer.prewait();
                if(reqSemer.wait() < 0) {
                    break ;
                }
                bGet = getMsg(_msg);
            }       
            return bGet ;
        }

        void recover(TREQ * pReq) {
            pPool->freeReq(pReq);
        }
        
        bool start() {
             tThread::tParam param ;
             char buffer[64];
             sprintf(buffer,"poolTrd %d",this);
             param.name = buffer;
             param.pParam = this ;
             reqThread = new (std::nothrow)tThread(param);
             if(reqThread==NULL) {
                 return false ;
             }
             bruning = true ;
             return reqThread->start(func);
        }

        void stop() {
            ///发送一个空的消息，让线程主动退出
            push(0,NULL,NULL);
            if(reqThread){
                reqThread->stop();
                delete reqThread;
                reqThread = NULL ;
            }
            {   ///释放请求数据
                tLockHelper  helper(&reqLocker);
                typename std::list<msg>::iterator iter = reqList.begin();
                while(iter != reqList.end()) {
                    if(iter->pReq) {
                        delete iter->pReq;
                    }    
                    iter++;
                }
                reqList.clear();
            } 
        }

        int getReqCount() {
            tLockHelper  helper(&reqLocker);
            return reqList.size();
        }

        private:
            bool getMsg(msg & _msg) {
                tLockHelper  helper(&reqLocker);
                if(reqList.empty()) {
                    return false ;
                }
                _msg = *(reqList.begin());
                reqList.erase(reqList.begin());
                return true ;
            }
    };
public:
    reqThreadPool() {
        bruning = false ;
    }
    virtual ~reqThreadPool() {
        uinit();
    }
public:
    bool  init(threadProchelper func,int trdCnt = 1) {
        for(int i = 0 ; i < trdCnt ; i++) {
            trdChannel * pChannel = new (std::nothrow)trdChannel;
            if(!pChannel) return false ;
            pChannel->pPool = this ;
            pChannel->func = func;
            if(!pChannel->start()) {
                delete pChannel;
                return false ;
            } 
            channelList.push_back(pChannel);
        }
        bruning = (channelList.size() > 0) ;
        return bruning ;
    }

    bool  sendReq(int svc,TREQ & req, void * pUserData) {
        if(!bruning) return false ;
        if(channelList.size() == 0) {
            return false ;
        }
        trdChannel * pChannel = NULL ;
        int count = 0xFFFFFF ; ///设置一个极大值，默认是达不到的
        int index = 0 ;
        bool bBusy = true ;
        /**
         * 查找请求数最少的channel 
         * 请求数相同的情况下，不繁忙的channel优先
         */ 
        for(int i = 0 ; i < channelList.size() ; i++) {
            pChannel = channelList[i];
            if(pChannel->bruning) {
                int _cnt = pChannel->getReqCount();
                /// 寻找数量最小的
                if(count > _cnt) {
                    count = _cnt;
                    index = i ;
                    bBusy = pChannel->bbusy;
                } else if(count == _cnt) {
                    if(bBusy && !pChannel->bbusy) {
                        index = i ;
                        bBusy = pChannel->bbusy ;
                    }    
                }
            }
        }
        pChannel = channelList[index];
        TREQ * pReq = getReq();
        if(!pReq) {
            return false ;
        }
        memcpy(pReq,&req,sizeof(TREQ));
        return pChannel->push(svc,pReq,pUserData);
    }

    void  uinit() {
        bruning = false ;
        ///让每个channel都退出
        for(int i = 0 ; i < channelList.size() ; i++) {
            channelList[i]->stop();
        }
        {///释放
            tLockHelper helper(&freeLock);
            typename std::list<TREQ *>::iterator iter = freeReqList.begin();
            while (iter != freeReqList.end()) {
                delete *iter ;    
                iter++;
            }   
            freeReqList.clear();
        }
    }
protected:
    ///线程数组
    std::vector<trdChannel *> channelList ;
    volatile  bool  bruning ;
    TREQ * getReq() {
        TREQ * pReq = NULL ;
        {
            tLockHelper helper(&freeLock);
            if(freeReqList.size()) {
                pReq = *(freeReqList.begin());
                freeReqList.erase(freeReqList.begin());
            }
        }
        if(pReq==NULL) {
            pReq = new (std::nothrow)TREQ;
        }
        return pReq;
    }
    void  freeReq(TREQ * pReq) {
        {
            tLockHelper helper(&freeLock);
            freeReqList.push_back(pReq);
        }
    }
    tLocker   freeLock ;
    std::list<TREQ *>  freeReqList ;
};

#endif