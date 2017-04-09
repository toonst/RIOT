/*
 * Copyright (C) Toon Stegen
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @author Toon Stegen <toonstegen@hotmail.com>
 */

#include "thread.h"
#include "xtimer.h"

#include "picotcp.h"
#include "picotcp/netdev2.h"

#include "pico_stack.h"
#include "pico_ipv4.h"

#ifdef MODULE_NETDEV2_TAP
#include "netdev2_tap.h"
#endif

#define ENABLE_DEBUG    (1)
#include "debug.h"

#define PICOTCP_STACKSIZE  (THREAD_STACKSIZE_DEFAULT)
#define PICOTCP_PRIO       (THREAD_PRIORITY_MAIN - 3)
#define PICOTCP_DELAY      (500)

#ifdef MODULE_NETDEV2_TAP
static struct pico_device pico_dev;
#endif

static char picotcp_stack[PICOTCP_STACKSIZE];

static void *_picotcp_thread(void *args)
{
    (void) args;

    /* process data and events */
    while (1) {
        pico_stack_tick();
        xtimer_usleep(1000);
    }

    /* should be never reached */
    return NULL;
}

void picotcp_bootstrap(void)
{
    int ret;
    struct pico_ip4 ipaddr, netmask;

    dbg("Starting picotcp_bootstrap\n");
    /* initialize the stack */
    pico_stack_init();

#ifdef MODULE_NETDEV2_TAP
    ret = picotcp_netdev2_init(&netdev2_tap.netdev, &pico_dev);
    if(ret) {
        dbg("Error initializing picotcp_netdev\n");
        return;
    }
#else
#error "No netdev included"
#endif

    /* assign the IP address to the tap interface */
    pico_string_to_ipv4("192.168.5.4", &ipaddr.addr);
    pico_string_to_ipv4("255.255.255.0", &netmask.addr);
    ret = pico_ipv4_link_add(&pico_dev, ipaddr, netmask);

    if(ret) {
        dbg("Error adding link\n");
        return;
    }

    thread_create(picotcp_stack, sizeof(picotcp_stack), PICOTCP_PRIO,
                  THREAD_CREATE_STACKTEST, _picotcp_thread, NULL, "picotcp");
}

/** @} */
