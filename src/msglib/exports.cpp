/**
 * libipcs库导出方法定义
 */

#include "./msglibs.hpp"
using namespace koal ;
/**
 * @brief 创建服务器接口
 * @param
 * @param pUrl 连接的字符串
 */
KOAL_EXPORT_MSGLIB ImsgServerSink * createMSGServer(ImsgServer * pSvr, const char * pUrl) {
	msgServerSink * pSink = new  (std::nothrow) msgServerSink(pSvr,pUrl);
	printf("start createMSGServer\n");
	if(pSink->initServerSink()) {
		printf(" createMSGServer  succssed\n");
		return pSink ;
	}
	delete pSink;
	return NULL ;
}

/**
 * @brief 释放服务器接口
 */
KOAL_EXPORT_MSGLIB void freeMSGServer(ImsgServerSink * pSink) {
	msgServerSink * _pSink = dynamic_cast<msgServerSink *>(pSink);
	if(_pSink) {
		_pSink->uninitServerSink();
		delete _pSink;
	}
}
