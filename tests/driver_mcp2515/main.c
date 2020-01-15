/*
 * Copyright (C) 2016 OTA keys
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Test application for the MCP2515 can driver
 *
 * @author      Toon Stegen <toon.stegen@altran.com>
 *
 * @}
 */

#ifndef TEST_MCP2515_SPI
#error "TEST_MCP2515_SPI not defined"
#endif
#ifndef TEST_MCP2515_CS
#error "TEST_MCP2515_CS not defined"
#endif
#ifndef TEST_MCP2515_RESET
#error "TEST_MCP2515_RESET not defined"
#endif
#ifndef TEST_MCP2515_MODE
#error "TEST_MCP2515_MODE not defined"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "shell.h"
#include "candev_mcp2515.h"
#include "../../drivers/mcp2515/mcp2515.h"
#include <errno.h>
#include <debug.h>

static candev_mcp2515_t dev;
struct can_bittiming timing;

#define CS_PIN GPIO_PIN(PORT_B, 4)
#define RST_PIN GPIO_PIN(PORT_B, 2)
#define INT_PIN GPIO_PIN(PORT_B, 3)

#define CANDEV_MCP2515_NUMOF 1

#define CANDEV_MCP2515_CLOCK 80000000U

static const candev_mcp2515_conf_t conf = {
    .spi = SPI_DEV(1),
    .cs_pin = CS_PIN,
    .rst_pin = RST_PIN,
    .int_pin = INT_PIN,
};

static int _send(int argc, char **argv) 
{
    (void) argc;
    (void) argv;
    int ret = 0;

    struct can_frame frame = {
        .can_id = 1,
        .can_dlc = 2,
        .data[0] = 255,
        .data[1] = 50
    };
    
    ret = mcp2515_send(&dev, &frame, 0);
    printf("mailbox: %d\n", ret);




    return 0;
}

static int _send2(int argc, char** argv)
//static int _send(candev_t *candev, const struct can_frame *frame)
{
    (void) argc;
    (void) argv;
    //candev_mcp2515_t *dev = (candev_mcp2515_t *)candev;
    int box;
    enum mcp2515_mode mode;

    struct can_frame frame = {
        .can_id = 1,
        .can_dlc = 2,
        .data[0] = 50,
        .data[1] = 51
    };

    mode = mcp2515_get_mode(&dev);
    if (mode != MODE_NORMAL && mode != MODE_LOOPBACK) {
       return -EINVAL; 
    }

    DEBUG("Inside mcp2515 send\n");

    for (box = 0; box < MCP2515_TX_MAILBOXES; box++) {
        if (dev.tx_mailbox[box] == NULL) {
            break;
        }
    }

    if (box == MCP2515_TX_MAILBOXES) {
        return -EBUSY;
    }

    dev.tx_mailbox[box] = &frame;

    mcp2515_send(&dev, &frame, box);

    return box;
}


static const shell_command_t shell_commands[] = {
    { "send", "send some data", _send },
    { "send2", "send some data", _send2 },
    //{ "receive", "receive some data", _receive },
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("MCP2515 can driver test application\n");
    printf("Initializing MCP2515 at SPI_%i... ", TEST_MCP2515_SPI);
    candev_mcp2515_init(&dev, &conf);

    /* run shell */
    puts("All OK, running shell now");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
