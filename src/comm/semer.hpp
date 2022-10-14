/**
 *  @file semer.hpp
 *  @brief 
 *  @date 2019年9月6日
 *  @author sharp
 *  @email  yangcp@koal.com
 */
#ifndef  THIS_IS_SEMER_HEADER_FROM_11231312321
#define  THIS_IS_SEMER_HEADER_FROM_11231312321

#ifdef  _WIN32
    #include <windows.h>
#else
    #include <pthread.h>
	#include <semaphore.h>
#endif

namespace   koal {
    class   tSemer {
public:
       int  getindex() {
			static int volatile index = 0 ;
			return index++;
		}   

        tSemer(const char * pName = NULL) {
#ifndef _WIN32
#ifndef __APPLE__  //linux
			int ret = sem_init(&Sem,0,0);
			if(ret != 0) {
				//printf("CSemer sem_init failed errno = %d\n", errno);
			}
#else
			char szName[128]="";
			if(!pName) {
				///加上时间，防止意外崩溃没有关闭
				sprintf(szName,"%d_%lld",this,tools::get_Startmsec());
           	 	sName = szName;
			} else {
				sprintf(szName,"%s_%d",pName,getindex());
			}
			Sem = sem_open(szName,O_CREAT, 0644, 0);
			
			if(Sem == SEM_FAILED) {
				sem_unlink(sName.c_str());
			}
			int val = -1;
			int nret = sem_getvalue(Sem,&val);
			while(val > 0) {
				sem_wait(Sem);
				sem_getvalue(Sem,&val);
			}
#endif

#else
			Sem = ::CreateEvent(NULL,TRUE,FALSE,NULL);
#endif
		}

        virtual	~tSemer() {
#ifndef  _WIN32
#ifndef  __APPLE__  //linux
		sem_destroy(&Sem);
#else
		if(Sem)
        {
			sem_close(Sem);
            sem_unlink(sName.c_str());
        }
#endif
#else
		::SetEvent(Sem);
		::CloseHandle(Sem);
		Sem = NULL;
#endif
		}

        virtual inline void post() {
#ifndef  _WIN32	
#ifdef  __APPLE__
				sem_post(Sem);
#else
				sem_post(&Sem);
#endif
#else
				::SetEvent(Sem);
#endif
		}
		inline  int  prewait() {
#ifdef _WIN32
			return ::ResetEvent(Sem);
#endif
			return 0 ;
		}
		virtual inline int wait() {
#ifndef  _WIN32

#ifdef  __APPLE__
			return	sem_wait(Sem);
#else
			return	sem_wait(&Sem);
#endif
#else
			unsigned int ret = ::WaitForSingleObject(Sem,INFINITE);
			if(WAIT_FAILED == ret){
				ret = -1;
			}
			return ret;
#endif
		}

        private:
#ifndef _WIN32
#ifndef __APPLE__  //linux
		sem_t     Sem;
#else
		sem_t  *  Sem;
        std::string sName;
#endif
#else
		HANDLE    Sem;
#endif
    };
}


#endif