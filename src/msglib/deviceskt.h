/**
 * @brief SOKCET封装
 * sharp.young
 * yangcp@koal.com
 * 20200110
 */ 
#ifndef THIS_IS_DEVICE_SKT_HEADER_FROM_SHARP_YOUNG_20200110_1917
#define THIS_IS_DEVICE_SKT_HEADER_FROM_SHARP_YOUNG_20200110_1917


#include "./skt.hpp"
#include <tThread.hpp>

namespace koal {
    class deviceskt
    {
    private:
        int skt1 ;
        int skt2 ;
        tThread *  mPdevTrd ;
        

    public:
        deviceskt();
        ~deviceskt();
        void device();
        ///绑定前端地址
        bool bind1(int type,const char * pUrl) ;
        ///绑定后端地址
        bool bind2(int type,const char * pUrl) ;
        ///连接前端，后端
        bool startDevice() ;
        ///停止前端，后端
        void stopDevice() ;
    };
       
}

#endif
