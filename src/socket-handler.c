#include "socket-handler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "block.h"

socket_handler socket_handler_new(block_arr blocks) {
    socket_handler handler = {
        .blocks = blocks,
    };
    return handler;
}

#define SOCKET_PATH "/tmp/dwmblocks"

int socket_handler_init(socket_handler *const handler) {
    int sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock_fd == -1) {
        (void)fprintf(stderr, "error: could not create socket\n");
        return 1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    int option = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    // name socket
    int status =
        bind(sock_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un));
    if (status != 0) {
        (void)fprintf(stderr, "error: could not bind socket\n");
        close(sock_fd);
        return 1;
    }

    handler->fd = sock_fd;
    handler->current_event = malloc(sizeof(char));

    return 0;
}

int socket_handler_deinit(socket_handler *const handler) {
    free(handler->current_event);
    if (close(handler->fd) != 0) {
        (void)fprintf(stderr, "error: could not close socket\n");
        return 1;
    }
    if (unlink(SOCKET_PATH) != 0) {
        (void)fprintf(stderr, "error: could not unbind socket\n");
        return 1;
    }
    return 0;
}

int socket_handler_read(socket_handler *const handler) {
    ssize_t n = read(handler->fd, handler->current_event, 1);
    if (n == 0) {
        (void)fprintf(stderr, "error: got EOF from socket while reading\n");
        return 1;
    }
    return 0;
}
