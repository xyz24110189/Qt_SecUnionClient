/**
 *  @file msglib.h
 *  @brief msglib的接口头文件
 *  @date 2019年12月31日
 *  @author sharp
 *  @email  yangcp@koal.com
 */ 
#ifndef THIS_IS_MSG_LIB_HEADER_FROM_SHARP_201912311801_fjdksaklf
#define THIS_IS_MSG_LIB_HEADER_FROM_SHARP_201912311801_fjdksaklf

#ifndef Interface
#define KoalInterface  struct
#endif

namespace koal {
    KoalInterface ImsgClient;
    KoalInterface ImsgClientSink;
    KoalInterface ImsgServer;
    KoalInterface ImsgServerSink;
}

///错误定义
#define ERR_PARAMTER  -1

#ifdef _WIN32
    #ifdef KOAL_MSGLIB_EXPORT
        #define  KOAL_EXPORT_MSGLIB __declspec(dllexport)
    #else
        #define  KOAL_EXPORT_MSGLIB //__declspec(dllimport)
    #endif
#else
    #define  KOAL_EXPORT_MSGLIB  __attribute__((visibility("default")))
#endif

/**
 * @brief 动态内存管理
 */ 
KOAL_EXPORT_MSGLIB void * msgLibMalloc(unsigned int size);
KOAL_EXPORT_MSGLIB void   msgLibFree(void *);

#endif