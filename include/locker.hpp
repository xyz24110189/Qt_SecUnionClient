/**
 *  @file locker.hpp
 *  @brief 锁头文件
 *  @date 2019年9月5日
 *  @author sharp
 *  @email  yangcp@koal.com
 */
#ifndef TLOCKER_H_
#define TLOCKER_H_

#ifdef  _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 
#endif
#include <windows.h>
#else
#include <pthread.h>
#endif

namespace koal
{
#ifndef  _WIN32
	/**
	* gcc 4.1.2后支持
	*/
	inline  int  lock_inc(volatile  int * val) {
		return __sync_add_and_fetch(val, 1);
	}

	inline  int  lock_dec(volatile   int * val) {
		return __sync_sub_and_fetch(val, 1);
	}
#else  
	inline  long  lock_inc(volatile  long * val) {
		return InterlockedIncrement(val);
	}

	inline  long  lock_dec(volatile   long * val) {
		return InterlockedDecrement(val);
	}
#endif

	/**
	* @brief 锁包装类
	*/

	class tLocker {
	public:
		inline  tLocker() {
#ifndef _WIN32
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
			pthread_mutex_init(&m_mutex,&attr);
#else
			::InitializeCriticalSection(&m_csLock);
#endif
		}
		virtual inline ~tLocker() {
#ifndef _WIN32
			pthread_mutex_destroy(&m_mutex);
#else
			::DeleteCriticalSection(&m_csLock);
#endif
		}
	public:
		virtual inline void lock() {
#ifndef _WIN32
			pthread_mutex_lock(&m_mutex);
#else
			::EnterCriticalSection(&m_csLock);
#endif
		}
		virtual inline void unlock() {
#ifndef _WIN32
			pthread_mutex_unlock(&m_mutex);
#else
			::LeaveCriticalSection(&m_csLock);
#endif
		}
	private:
#ifndef _WIN32
		mutable pthread_mutex_t m_mutex;
#else
		CRITICAL_SECTION	m_csLock;
#endif
	};

	/**
	* @brief 锁帮助类，自动开闭锁
	*/
	class tLockHelper {
#ifndef _WIN32
		volatile int      m_nlockCount;
#else
		volatile long     m_nlockCount;
#endif
		tLocker *         m_pLocker;
	public:
		tLockHelper(tLocker * pLocker, bool bAutoLock = true) {
			m_nlockCount = 0;
			m_pLocker = pLocker;
			if (bAutoLock) {
				lock();
			}
		}
		~tLockHelper() {
			while (m_nlockCount > 0) { unlock(); }
		}
	public:
		void  lock() {
			lock_inc(&m_nlockCount);
			m_pLocker->lock();
			return;
		}
		void  unlock() {
			lock_dec(&m_nlockCount);
			m_pLocker->unlock();
			return;
		}
		unsigned int  inline  getLockcount() {
			return m_nlockCount;
		}
	};
}

#endif /* TLOCKER_H_ */
