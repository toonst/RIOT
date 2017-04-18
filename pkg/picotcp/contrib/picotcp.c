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
#include "picotcp/netdev.h"

#include "pico_stack.h"
#include "pico_ipv4.h"

#ifdef MODULE_NETDEV_TAP
#include "netdev_tap.h"
#include "netdev_tap_params.h"
#endif

#ifdef MODULE_AT86RF2XX
#include "at86rf2xx.h"
#include "at86rf2xx_params.h"
#endif

#ifdef MODULE_MRF24J40
#include "mrf24j40.h"
#include "mrf24j40_params.h"
#endif

#define ENABLE_DEBUG    (1)
#include "debug.h"

#ifdef MODULE_NETDEV_TAP
#define PICO_DEVICE_NUMOF        (NETDEV_TAP_MAX)
#endif

#ifdef MODULE_AT86RF2XX    /* is mutual exclusive with above ifdef */
#define PICO_DEVICE_NUMOF        (sizeof(at86rf2xx_params) / sizeof(at86rf2xx_params[0]))
#endif

#ifdef MODULE_MRF24J40     /* is mutual exclusive with above ifdef */
#define PICO_DEVICE_NUMOF        (sizeof(mrf24j40_params) / sizeof(mrf24j40_params[0]))
#endif

#define PICOTCP_STACKSIZE  (THREAD_STACKSIZE_DEFAULT)
#define PICOTCP_PRIO       (THREAD_PRIORITY_MAIN - 3)

#ifdef PICO_DEVICE_NUMOF
static struct pico_device pico_devs[PICO_DEVICE_NUMOF];
#endif

#ifdef MODULE_NETDEV_TAP
static netdev_tap_t netdev_taps[PICO_DEVICE_NUMOF];
#endif

#ifdef MODULE_AT86RF2XX
static at86rf2xx_t at86rf2xx_devs[PICO_DEVICE_NUMOF];
#endif

#ifdef MODULE_MRF24J40
static mrf24j40_t mrf24j40_devs[PICO_DEVICE_NUMOF];
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

    /* initialize the stack */
    pico_stack_init();

    /* TODO: do for every eligable netdev */
#ifdef MODULE_NETDEV_TAP
    for (int i = 0; i < PICO_DEVICE_NUMOF; i++) {
        netdev_tap_setup(&netdev_taps[i], &netdev_tap_params[i]);
        if(picotcp_netdev_init((netdev_t *)&netdev_taps[i], &pico_devs[i])){
            DEBUG("Could not add netdev_tap device\n");
            return;
        }
    }
#elif defined(MODULE_MRF24J40)
    for (int i = 0; i < PICO_DEVICE_NUMOF; i++) {
        mrf24j40_setup(&mrf24j40_devs[i], &mrf24j40_params[i]);
        if(picotcp_netdev_init((netdev_t *)&mrf24j40_devs[i], &pico_devs[i])){
            DEBUG("Could not add mrf24j40 device\n");
            return;
        }
    }
#elif defined(MODULE_AT86RF2XX)
    for (int i = 0; i < PICO_DEVICE_NUMOF; i++) {
        at86rf2xx_setup(&at86rf2xx_devs[i], &at86rf2xx_params[i]);
        if(picotcp_netdev_init((netdev_t *)&at86rf2xx_devs[i], &pico_devs[i])){
            DEBUG("Could not add at86rf2xx device\n");
            return;
        }
    }
#else
#error "No netdev included"
#endif

    //TODO: link creation should probably be done
    //      on application level, and for all pico_devs
    pico_string_to_ipv4("192.168.5.4", &ipaddr.addr);
    pico_string_to_ipv4("255.255.255.0", &netmask.addr);
    ret = pico_ipv4_link_add(&pico_devs[0], ipaddr, netmask);

    if(ret) {
        dbg("Error adding link\n");
        return;
    }

    thread_create(picotcp_stack, sizeof(picotcp_stack), PICOTCP_PRIO,
                  THREAD_CREATE_STACKTEST, _picotcp_thread, NULL, "picotcp");
}

/** @} */
