/*
 * Copyright (C) 2016 Toon Stegen
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @author  Toon Stegen <toon.stegen@altran.com>
 */

#include <assert.h>
#include <sys/uio.h>
#include <inttypes.h>

#include "net/ieee802154.h"
#include "net/netdev.h"
#include "net/netopt.h"
#include "utlist.h"
#include "thread.h"
#include "kernel_defines.h"

#include "picotcp/netdev.h"

#include "pico_stack.h"

#define ENABLE_DEBUG                (1)
#include "debug.h"

#define PICOTCP_NETDEV_NAME           "picotcp_netdev_mux"
#define PICOTCP_NETDEV_PRIO           (THREAD_PRIORITY_MAIN - 4)
#define PICOTCP_NETDEV_STACKSIZE      (THREAD_STACKSIZE_DEFAULT)
#define PICOTCP_NETDEV_QUEUE_LEN      (8)
#define PICOTCP_NETDEV_MSG_TYPE_EVENT 0x1235

static kernel_pid_t _pid = KERNEL_PID_UNDEF;
static char _stack[PICOTCP_NETDEV_STACKSIZE];
static msg_t _queue[PICOTCP_NETDEV_QUEUE_LEN];
static uint8_t _tmp_buf[PICOTCP_NETDEV_BUFLEN];

static void _event_cb(netdev_t *dev, netdev_event_t event);
static void *_event_loop(void *arg);

static int _link_state;
static netdev_t *_global_netdev;

static netdev_t *_get_netdev(struct pico_device *pico_dev)
{
    //TODO use clean method of retreiving netdev
    //return container_of((void *)pico_dev, netdev_t, context);
    (void) pico_dev;
    return _global_netdev;
}

/* Send function. Return 0 if busy */
static int _netdev_send(struct pico_device *pico_dev, void *buf, int len)
{
    struct iovec vector;
    unsigned int count = 1; //TODO right amount?
    netdev_t *dev = _get_netdev(pico_dev);

    vector.iov_base = buf;
    vector.iov_len = (size_t)len;

    len = dev->driver->send(dev, &vector, count);
    return 1;
}

static int _netdev_dsr(struct pico_device *pico_dev, int loop_score)
{
    netdev_t *dev = _get_netdev(pico_dev);
    (void) loop_score;
    int len;

    // TODO: lock
    pico_dev->__serving_interrupt = 0;

    // TODO: take loop score into account
    len = dev->driver->recv(dev, _tmp_buf, sizeof(_tmp_buf), NULL);
    pico_stack_recv(pico_dev, _tmp_buf, len);
    return 0;
}

static int _netdev_link_state(struct pico_device *pico_dev)
{
    netdev_t *dev = _get_netdev(pico_dev);
    (void) dev;
    return _link_state;
}

static void _netdev_destroy(struct pico_device *pico_dev)
{
    netdev_t *dev = _get_netdev(pico_dev);
    (void) dev;
    //TODO: clean up
}

static void _event_cb(netdev_t *dev, netdev_event_t event)
{
    struct pico_device * pico_dev = (struct pico_device *) dev->context;

    if (event == NETDEV_EVENT_ISR) {
        /* driver needs it's ISR handled */
        msg_t msg;

        msg.type = PICOTCP_NETDEV_MSG_TYPE_EVENT;
        msg.content.ptr = (char *)dev;

        if (msg_send(&msg, _pid) <= 0) {
            DEBUG("picotcp_netdev: possibly lost interrupt.\n");
        }
    }
    switch(event) {
        case NETDEV_EVENT_RX_STARTED:
            /* started to receive a packet */
        break;
        case NETDEV_EVENT_RX_COMPLETE:
            /* finished receiving a packet */
            // TODO: lock
            pico_dev->__serving_interrupt = 1;

        break;
        case NETDEV_EVENT_TX_STARTED:
            /* started to transfer a packet */
        break;
        case NETDEV_EVENT_TX_COMPLETE:
            /* finished transferring packet */
        break;
        case NETDEV_EVENT_TX_NOACK:
            /* ACK requested but not received */
        break;
        case NETDEV_EVENT_TX_MEDIUM_BUSY:
            /* couldn't transfer packet */
        break;
        case NETDEV_EVENT_LINK_UP:
            /* link established */
            _link_state = 1;
        break;
        case NETDEV_EVENT_LINK_DOWN:
            /* link gone */
            _link_state = 0;
        break;
        default:
        break;
    }
}

static void *_event_loop(void *arg)
{
    (void)arg;
    msg_init_queue(_queue, PICOTCP_NETDEV_QUEUE_LEN);
    while (1) {
        msg_t msg;
        msg_receive(&msg);
        if (msg.type == PICOTCP_NETDEV_MSG_TYPE_EVENT) {
            netdev_t *dev = (netdev_t *)msg.content.ptr;
            dev->driver->isr(dev);
        }
    }
    return NULL;
}

int picotcp_netdev_init(netdev_t *netdev, struct pico_device *pico_dev)
{
    uint16_t dev_type;
    uint8_t mac[ETHERNET_ADDR_LEN] = {0};

    /* start multiplexing thread (only one needed) */
    if (_pid <= KERNEL_PID_UNDEF) {
        _pid = thread_create(_stack, PICOTCP_NETDEV_STACKSIZE, PICOTCP_NETDEV_PRIO,
                             THREAD_CREATE_STACKTEST, _event_loop, NULL,
                             PICOTCP_NETDEV_NAME);
        if (_pid <= 0) {
            dbg("Thread creation failed\n");
            return -1;
        }
    }
    //TODO: remove
    _global_netdev = netdev;

    /* initialize netdev */
    netdev->driver->init(netdev);
    netdev->context = pico_dev;
    netdev->event_callback = _event_cb;

    if (netdev->driver->get(netdev, NETOPT_DEVICE_TYPE, &dev_type,
                            sizeof(dev_type)) < 0) {
        dbg("Get devicetype failed\n");
        return -1;
    }

    switch (dev_type) {
#ifdef MODULE_NETDEV_ETH
        case NETDEV_TYPE_ETHERNET:
            /* retreive mac address */
            if (netdev->driver->get(netdev, NETOPT_ADDRESS, &mac,
                        ETHERNET_ADDR_LEN) < 0) {
                dbg("Mac addr retreival failed.\n");
                return -1;
            }
            break;
#endif
        case NETDEV_TYPE_UNKNOWN:
        case NETDEV_TYPE_RAW:
        case NETDEV_TYPE_IEEE802154:
        case NETDEV_TYPE_CC110X:
        default:
            dbg("device type not supported yet\n");
            return -1;
    }

    /* initialize pico_device */
    if( 0 != pico_device_init(pico_dev, "pico_netdev", mac)) {
        dbg("Device init failed.\n");
        //PICO_FREE(eth_dev);
        return -1;
    }

    pico_dev->send = _netdev_send;
    pico_dev->dsr = _netdev_dsr;
    pico_dev->destroy = _netdev_destroy;
    pico_dev->link_state = _netdev_link_state;

    return 0;
}

/** @} */
