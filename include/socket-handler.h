#ifndef SOCKET_HANDLER_H
#define SOCKET_HANDLER_H

#include "block.h"

typedef struct {
    int fd;
    block_arr blocks;
    char* current_event;
} socket_handler;

socket_handler socket_handler_new(block_arr blocks);
int socket_handler_init(socket_handler* const handler);
int socket_handler_deinit(socket_handler* const handler);
int socket_handler_read(socket_handler* const handler);

#endif  // SOCKET_HANDLER_H
