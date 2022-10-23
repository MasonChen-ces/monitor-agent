#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "recvMSGParser.h"

/**********************************************************************
 * Function:        InitState()
 * Description:     状态初始化(服务端)
 * Table Accessed:
 * Table Updated:
 * Input:           	p					解析器的相关信息
 * Output:
 * Return:
 **********************************************************************/
static void InitState(MsgParser *p)
{
    p->header = 0;
    p->need = sizeof(p->cache);

    free(p->msg);

    p->msg = NULL;
}

/**********************************************************************
 * Function:        ToMidState()
 * Description:     中间态(服务端)
 * Table Accessed:
 * Table Updated:
 * Input:           	p					解析器的相关信息
 * Output:
 * Return:          	ERROR				0
 *					SUCCESS				1
 * Others:
 **********************************************************************/
static int ToMidState(MsgParser *p)
{
    p->header = 1;
    p->need = p->cache.length;

    p->msg = malloc(sizeof(p->cache) + p->need);

    if (p->msg)
    {
        *p->msg = p->cache;
    }

    return !!p->msg;
}

/**********************************************************************
 * Function:        ToLastState()
 * Description:     最终态(服务端)
 * Table Accessed:
 * Table Updated:
 * Input:           	p					解析器的相关信息
 * Output:
 * Return:          	FAILTRUE			NULL
 *					SUCCESS				消息地址(Message*)
 * Others:
 **********************************************************************/
static RecvMessage *ToLastState(MsgParser *p)
{
    RecvMessage *ret = NULL;

    if (p->header && !p->need)
    {
        ret = p->msg;

        p->msg = NULL;
    }

    return ret;
}

/**********************************************************************
 * Function:        ntoh()
 * Description:     网络字节序转为本机字节序(服务端)
 * Table Accessed:
 * Table Updated:
 * Input:           	p					解析器的相关信息
 * Output:
 * Return:
 * Others:
 **********************************************************************/
static void ntoh(RecvMessage *m)
{
    m->type = ntohs(m->type);
    m->cmd = ntohs(m->cmd);
    m->index = ntohs(m->index);
    m->total = ntohs(m->total);
    m->length = ntohl(m->length);
}

/**********************************************************************
 * Function:        ToRecv()
 * Description:     接收一定量的数据(服务端)
 * Table Accessed:
 * Table Updated:
 * Input:           	fd					文件描述符
 *					size				欲接受的数据量
 * Output:			buf					接收到的数据
 * Return:          	i					实际接收到的数据量
 * Others:
 **********************************************************************/
static int ToRecv(int fd, char *buf, int size)
{
    int retry = 0;
    int i = 0;

    while (i < size)
    {
        int len = read(fd, buf + i, size - i);

        if (len > 0)
        {
            i += len;
        }
        else if (len < 0)
        {
            break;
        }
        else
        {
            if (retry++ > 5)
            {
                break;
            }

            usleep(500 * 1000);
        }
    }

    return i;
}

/**********************************************************************
 * Function:        MParser_New()
 * Description:     创建新解析器(服务端)
 * Table Accessed:
 * Table Updated:
 * Input:
 * Output:
 * Return:			FAILTRUE			NULL
 *					SUCCESS				解析器参数地址(MParser*)
 * Others:
 **********************************************************************/
MParser *MParser_New()
{
    MParser *ret = calloc(1, sizeof(MsgParser));

    if (ret)
    {
        InitState(ret);
    }

    return ret;
}

/**********************************************************************
 * Function:        MParser_ReadMem()
 * Description:     从内存读取数据(服务端)
 * Table Accessed:
 * Table Updated:
 * Input:           	parser				解析器参数
 *					mem					数据的内存地址
 *					length				数据量
 * Output:
 * Return:
 * Others:
 **********************************************************************/
RecvMessage *MParser_ReadMem(MParser *parser, unsigned char *mem, unsigned int length)
{
    RecvMessage *ret = NULL;
    MsgParser *p = (MsgParser *)parser;

    if (p && mem && length)
    {
        if (!p->header)
        {
            int len = (p->need < length) ? p->need : length;
            int offset = sizeof(p->cache) - p->need;

            memcpy((char *)&p->cache + offset, mem, len);

            if (p->need == len)
            {
                ntoh(&p->cache);

                mem += p->need;
                length -= p->need;

                if (ToMidState(p))
                {
                    ret = MParser_ReadMem(p, mem, length);
                }
                else
                {
                    InitState(p);
                }
            }
            else
            {
                p->need -= len;
            }
        }
        else
        {
            if (p->msg)
            {
                int len = (p->need < length) ? p->need : length;
                int offset = p->msg->length - p->need;

                memcpy(p->msg->payload + offset, mem, len);

                p->need -= len;
            }

            if (ret = ToLastState(p))
            {
                InitState(p);
            }
        }
    }

    return ret;
}

/**********************************************************************
 * Function:        MParser_ReadFd()
 * Description:     从文件描述符读取数据(服务端)
 * Table Accessed:
 * Table Updated:
 * Input:           	fd					文件描述符
 *					parser				解析器参数
 * Output:
 * Return:          	Message 			(MParser*)
 * Others:
 **********************************************************************/
RecvMessage *MParser_ReadFd(MParser *parser, int fd)
{
    RecvMessage *ret = NULL;
    MsgParser *p = (MsgParser *)parser;
    if ((fd != -1) && p)
    {
        if (!p->header)
        {
            int offset = sizeof(p->cache) - p->need;
            int len = ToRecv(fd, (char *)&p->cache + offset, p->need);

            if (len == p->need)
            {
                ntoh(&p->cache);

                if (ToMidState(p))
                {
                    ret = MParser_ReadFd(p, fd);
                }
                else
                {
                    InitState(p);
                }
            }
            else
            {
                p->need -= len;
            }
        }
        else
        {
            if (p->msg)
            {
                int offset = p->msg->length - p->need;
                int len = ToRecv(fd, p->msg->payload + offset, p->need);

                p->need -= len;
            }

            if (ret = ToLastState(p))
            {
                InitState(p);
            }
        }
    }

    return ret;
}

/**********************************************************************
 * Function:        MParser_Reset()
 * Description:     复位解析器(服务端)
 * Table Accessed:
 * Table Updated:
 * Input:
 * Output:
 * Return:			FAILTRUE			NULL
 *					SUCCESS				解析器参数地址(MParser*)
 * Others:
 **********************************************************************/
void MParser_Reset(MParser *parser)
{
    MsgParser *p = (MsgParser *)parser;

    if (p)
    {
        InitState(p);
    }
}

/**********************************************************************
 * Function:        MParser_Del()
 * Description:     删除解析器(服务端)
 * Table Accessed:
 * Table Updated:
 * Input:
 * Output:
 * Return:			FAILTRUE			NULL
 *					SUCCESS				解析器参数地址(MParser*)
 * Others:
 **********************************************************************/
void MParser_Del(MParser *parser)
{
    MsgParser *p = (MsgParser *)parser;

    if (p)
    {
        free(p->msg);
        free(p);
    }
}
