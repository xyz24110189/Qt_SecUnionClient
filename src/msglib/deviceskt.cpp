#include "deviceskt.h"
#include <3rd/nano/include/nn.h>

THRED_RETURN   __stdcall  deviceThread(void * pParam) ;

namespace koal {
    deviceskt::deviceskt() {
        skt1 = 0 ;
        skt2 = 0 ;
        mPdevTrd = NULL ;
    }

    deviceskt::~deviceskt() {
        
    }

    ///绑定前端地址
    bool  deviceskt::bind1(int type,const char * pUrl)  {
        if(skt1) {
            return true ;
        }
        skt1 = nn_socket (AF_SP_RAW, type);
        if (skt1 < 0) {
            fprintf (stderr, "nn_socket: %s\n", nn_strerror (nn_errno ()));
            return false;
        }
        if (nn_bind (skt1, pUrl) < 0) {
            fprintf (stderr, "nn_bind1(%s): %s\n", pUrl, nn_strerror (nn_errno ()));
            return false;
        }
        return true ;
    }
    ///绑定后端地址
    bool  deviceskt::bind2(int type,const char * pUrl)  {
        if(skt2) {
            return true ;
        }
        skt2 = nn_socket (AF_SP_RAW, type);
        if (skt2 < 0) {
            fprintf (stderr, "nn_socket: %s\n", nn_strerror (nn_errno ()));
            return false;
        }
        if (nn_bind (skt2, pUrl) < 0) {
            fprintf (stderr, "nn_bind2(%s): %s\n", pUrl, nn_strerror (nn_errno ()));
            return false;
        }
        return true ;
    }
    ///连接前端，后端
    bool  deviceskt::startDevice()  {
        if(mPdevTrd == NULL) {
            tThread::tParam param;
            param.name = "deviceSkt";
            param.pParam=this;
            mPdevTrd = new tThread(param);
        }
        return mPdevTrd->start(&deviceThread);
    }

    void  deviceskt::stopDevice() {
        if(mPdevTrd) {
            mPdevTrd->stop();
            delete mPdevTrd;
            mPdevTrd = NULL ;
        }
    }

    void  deviceskt::device() {
        if(skt2 && skt1) {
            if(nn_device (skt1, skt2) != 0) {
                fprintf (stderr, "nn_device: %s\n", nn_strerror (nn_errno()));
            }
        }
        return ;
    }
}

using namespace koal;

THRED_RETURN   __stdcall  deviceThread(void * pParam) {
    deviceskt * pDevice ;
    pDevice->device();
    return 0 ;
}
