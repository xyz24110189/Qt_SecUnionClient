/**
 * libipcc库导出方法定义
 */
#include "./msglibc.hpp"


using namespace koal ;
/**
 * @brief 创建客户端接口
 * @param
 * @param pUrl 连接的字符串
 */ 
KOAL_EXPORT_MSGLIB ImsgClientSink * createMSGClient(ImsgClient * pClient, const char * pUrl) {
	printf("createMSGClient start \n");
	msgClientSink * pSink = new  (std::nothrow) msgClientSink(pClient,pUrl);
	if(pSink->initClient()) {
		printf("createMSGClient succ \n");
		return pSink ;
	}
	printf("createMSGClient failed \n");
	delete pSink;
	return NULL ;
}

/**
 * @brief 释放服务器接口
 */ 
KOAL_EXPORT_MSGLIB void freeMSGClient(ImsgClientSink * pSink) {
	msgClientSink * _pSink = dynamic_cast<msgClientSink *>(pSink);
	if(_pSink) {
		_pSink->uninitClient();
		delete _pSink ;
	}
}
