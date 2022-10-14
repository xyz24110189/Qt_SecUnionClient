/**
 *  @file config.h
 *  @brief 提供基础类型的定义头文件
 *  @date 2019年9月3日
 *  @author sharp
 *  @email  yangcp@koal.com
 */
#ifndef CONFIG_H_
#define CONFIG_H_

#ifdef  _WIN32
typedef   char   int8;
typedef   unsigned char  uint8;
typedef   short  int16 ;
typedef   unsigned short uint16 ;
typedef   int    int32 ;
typedef   unsigned int   uint32 ;
typedef   __int64   int64 ;
typedef   unsigned  __int64  uint64 ;

#elif  defined(__linux__)
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>
#include <stdarg.h>

typedef   int8_t     int8   ;
typedef   uint8_t    uint8  ;
typedef   int16_t    int16  ;
typedef   uint16_t   uint16 ;
typedef   int32_t    int32  ;
typedef   uint32_t   uint32 ;
typedef   int64_t    int64  ;
typedef   uint64_t   uint64 ;

#elif  defined(__APPLE__)
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>
#include <stdarg.h>

typedef   int8_t     int8   ;
typedef   uint8_t    uint8  ;
typedef   int16_t    int16  ;
typedef   uint16_t   uint16 ;
typedef   int32_t    int32  ;
typedef   uint32_t   uint32 ;
typedef   int64_t    int64  ;
typedef   uint64_t   uint64 ;

#endif

///表示输入参数
#ifndef _IN    
#define _IN
#endif  
///表示输出参数
#ifndef _OUT   
#define _OUT
#endif
///表示即是输入也是输出参数
#ifndef _INOUT 
#define _INOUT
#endif

#ifndef Interface
#define KoalInterface  struct
#endif

#ifndef nullPtr
#define nullPtr  0
#endif 

#ifndef _WIN32
#define VOLATILE_INT	volatile int     
#else
#define VOLATILE_INT    volatile  long  
#endif


#endif