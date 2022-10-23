#include <stdlib.h>
#include <string.h>

#include "recvMessage.h"

/**********************************************************************
 * Function:        Message_New()
 * Description:     构造新消息(客户端)
 * Table Accessed:
 * Table Updated:
 * Input:           type				类型消息
 *					cmd					命令
 *					index				标识
 *					total				总数
 *					payload				数据区
 *					length				数据长度
 * Output:
 * Return:          ERROR				NULL
 *					SUCCESS				消息地址(Message*)
 * Others:
 **********************************************************************/
RecvMessage* RecvMessage_New(unsigned short type,
                     unsigned short cmd,
                     unsigned short index,
                     unsigned short total,
                     const char* payload,
                     unsigned int length){
    RecvMessage *ret = malloc(sizeof(RecvMessage) + length);

    if (ret)
    {
        ret->type = type;
        ret->cmd = cmd;
        ret->index = index;
        ret->total = total;
        ret->length = length;

        if (payload && length)
        {
            memcpy(ret + 1, payload, length);
        }
    }

    return ret;
}
