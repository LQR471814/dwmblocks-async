#ifndef WATCHER_H
#define WATCHER_H

#include <poll.h>
#include <stdbool.h>

#include "block.h"
#include "main.h"

#define SIGNAL_FD BLOCK_COUNT
#define SOCKET_FD (BLOCK_COUNT + 1)
#define WATCHER_FD_COUNT (SOCKET_FD + 1)

typedef struct pollfd watcher_fd;

typedef struct {
    watcher_fd fds[WATCHER_FD_COUNT];
    unsigned short active_blocks[BLOCK_COUNT];
    unsigned short active_block_count;
    bool got_signal;
    bool got_socket;
} watcher;

int watcher_init(watcher *const watcher, block_arr blocks, const int signal_fd, const int socket_fd);
int watcher_poll(watcher *const watcher, const int timeout_ms);

#endif  // WATCHER_H
