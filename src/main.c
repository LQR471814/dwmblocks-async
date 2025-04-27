#include "main.h"

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>

#include "block.h"
#include "cli.h"
#include "config.h"
#include "signal-handler.h"
#include "socket-handler.h"
#include "status.h"
#include "timer.h"
#include "util.h"
#include "watcher.h"
#include "x11.h"

static int refresh_and_advance_timer(block_arr blocks, timer *const timer) {
    for (unsigned short i = 0; i < blocks.length; ++i) {
        block *const block = &blocks.values[i];
        if (!timer_must_run_block(timer, block)) {
            continue;
        }
        if (block_execute(block, 0) != 0) {
            return 1;
        }
    }
    if (timer_arm(timer) != 0) {
        return 1;
    }
    return 0;
}

typedef struct {
    block_arr blocks;
    const bool is_debug_mode;
    x11_connection *const connection;
    signal_handler *const signal_handler;
    socket_handler *const socket_handler;
    timer timer;
    status status;
} event_loop;

event_loop event_loop_new(block_arr blocks, const bool is_debug_mode,
                          x11_connection *const connection,
                          signal_handler *const signal_handler,
                          socket_handler *const socket_handler) {
    event_loop loop = {.blocks = blocks,
                       .is_debug_mode = is_debug_mode,
                       .connection = connection,
                       .signal_handler = signal_handler,
                       .socket_handler = socket_handler,
                       .timer = timer_new(blocks),
                       .status = status_new(blocks)};
    return loop;
}

static int event_loop_handle_signal(event_loop *const loop) {
    int signal = signal_handler_read(loop->signal_handler);
    switch (signal) {
        case TIMER_SIGNAL:
            return refresh_and_advance_timer(loop->blocks, &loop->timer);
        case SIGTERM:
        case SIGINT:
            return 1;
        default:
            return 0;
    }
}

static int event_loop_handle_socket(event_loop *const loop) {
    int status = socket_handler_read(loop->socket_handler);
    if (status == 1) {
        return 1;
    }
    char event_id = *(loop->socket_handler->current_event);
    for (unsigned short i = 0; i < loop->blocks.length; ++i) {
        block *const b = &loop->blocks.values[i];
        if (b->event_id != event_id) {
            continue;
        }
        if (block_execute(b, 0) != 0) {
            return 1;
        }
    }
    return 0;
}

int event_loop_run(event_loop *const loop) {
    // kickstart the event loop with an initial execution.
    if (refresh_and_advance_timer(loop->blocks, &loop->timer) != 0) {
        return 1;
    }

    watcher watcher;
    if (watcher_init(&watcher, loop->blocks, loop->signal_handler->fd,
                     loop->socket_handler->fd) != 0) {
        return 1;
    }

    while (true) {
        if (watcher_poll(&watcher, -1) != 0) {
            return 1;
        }

        if (watcher.got_signal) {
            int will_exit = event_loop_handle_signal(loop);
            if (will_exit == 1) {
                return 0;
            }
        }

        if (watcher.got_socket) {
            int status = event_loop_handle_socket(loop);
            if (status == 1) {
                return 1;
            }
        }

        for (unsigned short i = 0; i < watcher.active_block_count; ++i) {
            (void)block_update(&loop->blocks.values[watcher.active_blocks[i]]);
        }

        const bool has_status_changed = status_update(&loop->status);
        if (has_status_changed &&
            status_write(&loop->status, loop->is_debug_mode,
                         loop->connection) != 0) {
            return 1;
        }
    }

    return 0;
}

int main(const int argc, const char *const argv[]) {
    const cli_arguments cli_args = cli_parse_arguments(argv, argc);
    if (errno != 0) {
        return 1;
    }

    x11_connection *const connection = x11_connection_open();
    if (connection == NULL) {
        return 1;
    }

#define BLOCK(icon, command, interval, signal) \
    block_new(icon, command, interval, signal),
    block block_values[BLOCK_COUNT] = {BLOCKS(BLOCK)};
#undef BLOCK

    block_arr blocks = {.values = block_values, .length = LEN(block_values)};
    int status = 0;
    if (block_arr_init(blocks) != 0) {
        status = 1;
        goto cleanup_x11;
    }

    signal_handler signal_handler = signal_handler_new(blocks);
    if (signal_handler_init(&signal_handler) != 0) {
        status = 1;
        goto cleanup_blocks;
    }

    socket_handler socket_handler = socket_handler_new(blocks);
    if (socket_handler_init(&socket_handler) != 0) {
        status = 1;
        goto cleanup_signal_handler;
    }

    event_loop loop =
        event_loop_new(blocks, cli_args.is_debug_mode, connection,
                       &signal_handler, &socket_handler);

    if (event_loop_run(&loop) != 0) {
        status = 1;
    }

    if (socket_handler_deinit(&socket_handler) != 0) {
        status = 1;
    }

cleanup_signal_handler:
    if (signal_handler_deinit(&signal_handler) != 0) {
        status = 1;
    }

cleanup_blocks:
    if (block_arr_deinit(blocks) != 0) {
        status = 1;
    }

cleanup_x11:
    x11_connection_close(connection);

    return status;
}
