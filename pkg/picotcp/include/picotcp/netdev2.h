/*
 * Copyright (C) 2017 Toon Stegen <toonstegen@hotmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    pkg_picotcp_netdev2    picoTCP netdev2 adapter
 * @ingroup     pkg_picotcp
 * @brief       netdev2 adapter for picoTCP
 * @{
 *
 * @file
 * @brief   picoTCP netdev2 adapter definitions
 *
 * @author  Toon Stegen <toonstegen@hotmail.com>
 */
#ifndef NETDEV2_H_
#define NETDEV2_H_

#include "net/ethernet.h"
#include "net/netdev2.h"

#include "pico_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Length of the temporary copying buffer for receival.
 * @note    It should be as long as the maximum packet length of all the netdev you use.
 */
#ifndef PICOTCP_NETDEV2_BUFLEN
#define PICOTCP_NETDEV2_BUFLEN     (ETHERNET_MAX_LEN)
#endif

/**
 * @brief   Initializes the netdev2 adapter.
 *
 * @param[in] netdev The netdev device for initialisation
 * @param[in] pico_dev The pico_device for initialisation
 *
 * @return  0 on success.
 * @return  -1 on error.
 */
int picotcp_netdev2_init(netdev2_t *netdev, struct pico_device *pico_dev);

#ifdef __cplusplus
}
#endif

#endif /* NETDEV2_H_ */
/** @} */
