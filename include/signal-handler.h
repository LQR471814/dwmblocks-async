#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <bits/types/sigset_t.h>

#include "block.h"

typedef sigset_t signal_set;

typedef struct {
    int fd;
    block_arr blocks;
} signal_handler;

signal_handler signal_handler_new(block_arr blocks);
int signal_handler_init(signal_handler* const handler);
int signal_handler_deinit(signal_handler* const handler);
int signal_handler_read(signal_handler* const handler);

#endif  // SIGNAL_HANDLER_H
