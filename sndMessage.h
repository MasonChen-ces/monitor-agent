#ifndef SND_MESSAGE_H
#define SND_MESSAGE_H
typedef struct sndMessage
{
    unsigned int length;	// 数据长度
    unsigned char payload[];// 数据区
} SndMessage;

SndMessage *SndMessage_new(unsigned int length,unsigned char *payload);
#endif