#pragma once

#define MSG_RANGE 50
// 消息定义
enum MsgCode
{
	MSG_SEND_TO_TRAY = 0x00000000,      /* 发消息给托盘 [内部使用] */
	MSG_SEND_TO_APP,                                       /* 发消息给其他应用 [内部使用] */

	MSG_LOGOUT_CLEAN_DEAL,                                 /* 客户端登出处理 [内部使用] */
	MSG_PROCESS_MONITOR_OPEN,                              /* 开启程序监控 [内部使用] */
	MSG_PROCESS_MONITOR_CLOSE,                             /* 关闭程序监控 [内部使用] */

	MSG_SERVICE_CONTROL_START,                             /* 启动服务程序 [内部使用] */
	MSG_SERVICE_CONTROL_STOP,                              /* 停止服务程序 [内部使用] */

	MSG_INNER_OUTER_SPLITER             = 0x00000063,      /* ==========内外部消息定义分界 =========== */

	MSG_PROCESS_CONTROL_REGISTER        = 0x00000064,      /* 注册控制外部进程 */
	MSG_PROCESS_CONTROL_UNREGISTER      = 0x00000065,      /* 注销控制外部进程 */

	MSG_PROCESS_MONITOR_REGISTER        = 0x00000096,      /* 注册保活程序 */
	MSG_PROCESS_MONITOR_UNREGISTER      = 0x00000097,      /* 注销保活程序 */

	MSG_SERVICE_CONTROL_REGISTER        = 0x000000C8,      /* 注册控制服务程序 */
	MSG_SERVICE_CONTROL_UNREGISTER      = 0x000000C9,      /* 注销控制服务程序 */

	MSG_EXTERNAL_PROGRAM_OPER           = 0x0000FFFD,      /* 操作外部程序 */
	MSG_SHOW_TRAY_TIPS                  = 0x0000FFFE,      /* 显示托盘冒泡信息 */
	MSG_CHANGE_TRAYICONS_INFO           = 0x0000FFFF	   /* 修改托盘图标和鼠标悬浮提示信息 */            
};
