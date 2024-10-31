#ifndef __EG_HTTP_SERVER_H
#define __EG_HTTP_SERVER_H

/*服务器回调消息，自定义修改*/
enum eg_srv_msg_type {
    EG_DEFAULT_SRV_MSG = 0,
};
/*命令示例，自定义修改*/
enum eg_http_cmd {
    EG_DEFAULT_CMD = 0,
};

/*event 参数*/
struct eg_event_arg {
    void *cli;
    int cmd;
    char *content;
};
int eg_protocol_init(void);
int eg_cmd_analysis(int cmd, const char *content, void *cli);
#endif // __EG_HTTP_SERVER_H

