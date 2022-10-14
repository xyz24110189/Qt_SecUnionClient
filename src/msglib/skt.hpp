/**
 * @brief SOKCET封装
 * sharp.young
 * yangcp@koal.com
 * 20200108
 */ 
#ifndef THIS_IS_SKT_HPP_FROM_SHARP_20100110_1920
#define THIS_IS_SKT_HPP_FROM_SHARP_20100110_1920

#include <3rd/nano/include/nn.h>
//#include <3rd/nano/include/err.h>
#include <locker.hpp>
#include <stdio.h>


namespace koal {
    class stSocket {
        public:
        stSocket(){
            mSKT = -1 ; 
        }
        ~stSocket() {
            close();
        }
        virtual bool init(int type,int family = AF_SP) {
        	printf("bool init \n");
            mSKT = nn_socket (family, type);
            if(mSKT == -1) {
                return false;
            }
            printf("bool init123 \n");
            return true;
        }
        virtual bool setOpt(int lvl,int opt,const void *optval,int optvallen) {
            if(mSKT < 0) {
                return false ;
            }
            int ret = nn_setsockopt (mSKT, lvl, opt, optval, optvallen);
            if(ret < 0) {
                return false ;
            }
            return true ;
        }
        virtual bool bind(const char * purl) {
            int rc;
            rc = nn_bind (mSKT, purl);
            if (rc < 0) {
               //printf("bind error:  %s (rc=%d)\n",nn_err_strerror(errno),rc);
               return false ;
            }
            return true ;
        }
        virtual bool connect(const char * purl,int timeout = 5) {
            if(mSKT < 0) {
                return false ;
            }
            int ret,ret1;
            if(timeout) {
                int tm = timeout*1000;
                ret = nn_setsockopt (mSKT, NN_SOL_SOCKET, NN_RCVTIMEO, &tm, sizeof(tm));
                ret1 = nn_setsockopt (mSKT, NN_SOL_SOCKET, NN_SNDTIMEO, &tm, sizeof(tm));
            }
            if(ret < 0  || ret1 < 0) {
               return false ;
            }
            ret = nn_connect (mSKT, purl);
            if(ret < 0) {
               return false ;
            }
            return true ;
        }
        virtual int  snd(const unsigned char * buf, int len, int flags = 0) {
            if(mSKT < 0) {
                return -1 ;
            }
            int rc = nn_send (mSKT, buf, len, flags);
            if (rc < 0) {
                //printf("snd error:  %s (rc=%d)\n",nn_err_strerror(errno),rc);
                return rc ;
            }
            if (rc != len) {
                //printf("snd error:  %s (rc=%d)\n",nn_err_strerror(errno),rc);
                return rc;
            }
            return rc ;
        }
        virtual int  recv(unsigned char ** buf, int flags = 0) {
            if(mSKT < 0) {
                return 0 ;
            }
            int rc = nn_recv (mSKT, buf, NN_MSG, flags);
            return rc ;
        }
        virtual unsigned char * getmsg(int len) {
            return (unsigned char *)nn_allocmsg(len,0);
        }
        virtual int fremsg(unsigned char * buf) {
           return nn_freemsg (buf);
        }
        virtual void close() {
            if(mSKT < 0) {
                return  ;
            }
            int rc = nn_close (mSKT);
            if ((rc != 0) && (errno != EBADF && errno != ETERM)) {
            	//printf("snd error:  %s (rc=%d)\n",nn_err_strerror (errno),rc);
            }
            mSKT = -1;
            return ;
        }
        virtual int getSKT() { return mSKT ;}
    protected:    
        int  mSKT ;
    };
}
#endif
