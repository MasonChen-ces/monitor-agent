#ifndef MSG_PARSER_H
#define MSG_PARSER_H

#include "recvMessage.h"

typedef void  MParser;

typedef struct msg_parser
{
    RecvMessage cache;   // 缓存已解析的消息头
    int header;      // 标识消息头是否解析成功
    int need;        // 标识还需要多少字节才能完成解析
    RecvMessage* msg;    // 解析中的协议消息（半成品）
} MsgParser;


MParser* MParser_New();
RecvMessage* MParser_ReadMem(MParser* parser, unsigned char* mem, unsigned int length);
RecvMessage* MParser_ReadFd(MParser* parser, int fd);
void MParser_Reset(MParser* parser);
void MParser_Del(MParser* parser);

#endif