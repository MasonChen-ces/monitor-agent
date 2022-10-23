#ifndef RECV_MESSAGE_H
#define RECV_MESSAGE_H

enum CMD{
    CMD_SHELL = 1,
    CMD_SQL
};

enum TYPE{
    TYP_ORACLE = 1,
    TYP_MYSQL,
    TYP_POSTGRESQL
};
typedef struct recvMessage
{
    unsigned short type;	// 类型消息
    unsigned short cmd;		// 命令
    unsigned short index;	// 标识
    unsigned short total;	// 总数
    unsigned int length;	// 数据长度
    unsigned char payload[];// 数据区
} RecvMessage;

RecvMessage* RecvMessage_New(unsigned short type,
                     unsigned short cmd,
                     unsigned short index,
                     unsigned short total,
                     const char* payload,
                     unsigned int length);

#endif