#include "timer.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "block.h"
#include "util.h"

static unsigned int compute_tick(block_arr blocks) {
    unsigned int tick = 0;
    for (unsigned short i = 0; i < blocks.length; ++i) {
        const block *const block = &blocks.values[i];
        tick = gcd(block->interval, tick);
    }
    return tick;
}

static unsigned int compute_reset_value(block_arr blocks) {
    unsigned int reset_value = 1;
    for (unsigned short i = 0; i < blocks.length; ++i) {
        const block *const block = &blocks.values[i];
        reset_value = MAX(block->interval, reset_value);
    }
    return reset_value;
}

timer timer_new(block_arr blocks) {
    const unsigned int reset_value = compute_reset_value(blocks);
    timer timer = {
        .time = reset_value,  // Initial value to execute all blocks.
        .tick = compute_tick(blocks),
        .reset_value = reset_value,
    };
    return timer;
}

int timer_arm(timer *const timer) {
    errno = 0;
    (void)alarm(timer->tick);

    if (errno != 0) {
        (void)fprintf(stderr, "error: could not arm timer\n");
        return 1;
    }
    // Wrap `time` to the interval [1, reset_value].
    timer->time = (timer->time + timer->tick) % timer->reset_value;
    return 0;
}

bool timer_must_run_block(const timer *const timer, const block *const block) {
    if (timer == NULL || timer->time == timer->reset_value) {
        return true;
    }
    if (block->interval == 0) {
        return false;
    }
    return timer->time % block->interval == 0;
}
