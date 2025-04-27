#ifndef CONFIG_H
#define CONFIG_H

// String used to delimit block outputs in the status.
#define DELIMITER "  "

// Maximum number of Unicode characters that a block can output.
#define MAX_BLOCK_OUTPUT_LENGTH 45

// Control whether blocks are clickable.
#define CLICKABLE_BLOCKS 0

// Control whether a leading delimiter should be prepended to the status.
#define LEADING_DELIMITER 0

// Control whether a trailing delimiter should be appended to the status.
#define TRAILING_DELIMITER 0

// Define blocks for the status feed as X(icon, cmd, interval, event_id).
// Note: An event_id=0 would indicate that the block should not receive any
// events. Similarly an interval=0, indicates that the block should never be
// refreshed by the timer.
#define BLOCKS(X)                  \
    X("", "bar-network", 0,    2)  \
    X("", "bar-sound",   0,    1)  \
    X("", "bar-battery", 0,    3)  \
    X("", "bar-date",    1,    0)

#endif  // CONFIG_H
