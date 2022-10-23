#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sndMessage.h"
#include "log.h"
SndMessage *SndMessage_new(unsigned int length, unsigned char *payload)
{
    if (payload == NULL)
    {
        debug("payload is NULL\n");
        return NULL;
    }

    if (strlen(payload) < 0)
    {
        debug("payload is empty.\n");
        return NULL;
    }

    SndMessage *message;
    message = malloc(sizeof(SndMessage) + length);
    message->length = htonl(length);
    if (payload && length)
    {
       memcpy(message+1, payload, length);
    }
    return message;
}