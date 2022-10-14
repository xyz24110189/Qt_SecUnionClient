/**
 *  @file tThread.hpp
 *  @brief 线程类
 *  @date 2019年9月20日
 *  @author sharp
 *  @email  yangcp@koal.com
 */
#ifndef THIS_IS_THREAD_HPP_FROM_SHAP_11243431321
#define THIS_IS_THREAD_HPP_FROM_SHAP_11243431321

#ifdef  _WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif

#include <windows.h>
#include <process.h>
#include <string>
	
#define  THRED_RETURN unsigned int
#else
#include <string> 
#include <string.h>
	#define __stdcall
	#define  THRED_RETURN  void *
#endif

typedef THRED_RETURN   (__stdcall *threadProchelper)(void * pParam) ;


class tThread
{
public:
    struct tParam {
        std::string name  ; ///线程名
        void  *     pParam;
    };
    struct thread_info {
        ///线程ID
    #ifndef _WIN32
        pthread_t tid ;
    #else
        unsigned int     tid ;
        HANDLE    handle ;
    #endif
    };
private:
   tParam   mTParam ; 
   thread_info  info;
public:
    tThread(tParam & param) {
        mTParam = param;
        memset(&info,0,sizeof(thread_info));
    }
    ~tThread() {
        if(info.tid) {
            stop();
        }
    }
public:
    ///启动线程
    bool  start(threadProchelper  pfunc) {
#ifndef _WIN32
	    int rc = pthread_create(&info.tid,NULL,pfunc,(void *)(&this->mTParam));
	    if(rc) {
		    return false ;
	    }
#else
	    info.handle = (HANDLE)::_beginthreadex(NULL,0,pfunc,(void *)(&this->mTParam),0,&(info.tid));
#endif
        return true ;
    }
    ///停止线程
    void  stop(int  waitMsec = 0) {
#ifndef _WIN32
        if(info.tid) {
            void * status = 0 ;
            pthread_join(info.tid,&status);
            info.tid = 0 ;
        }     
#else
        if(info.handle) {
            ::WaitForSingleObject(info.handle,INFINITE);
			CloseHandle(info.handle);
            info.handle = 0;
        }			
#endif
    }
/**
 * _linux上可以在线程方法外面使用，苹果平台必须在线程方法里面使用
 */ 
    void  setName(const std::string & name) {
#ifdef __linux__
	pthread_setname_np(info.tid,mTParam.name.c_str());
#endif

#ifdef __APPLE__
    pthread_setname_np(mTParam.name.c_str());
#endif
    }
};

#endif
