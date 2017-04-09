/*
 * Copyright (C) 2015 Martine Lenders <mlenders@inf.fu-berlin.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    pkg_picotcp    picoTCP
 * @ingroup     pkg
 * @brief       A lightweight modular TCP/IP stack
 * @see         http://www.picotcp.com
 *
 * picoTCP is a lightweight TCP/IP stack
 *
 * @{
 *
 * @file
 * @brief   picoTCP bootstrap definitions
 *
 * @author  Toon Stegen <toonstegen@hotmail.com>
 */
#ifndef PICOTCP_H_
#define PICOTCP_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Initializes picoTCP stack.
 *
 * This initializes picoTCP, i.e. all netdevs are added to as interfaces to the
 * stack and the stack's thread is started.
 */
void picotcp_bootstrap(void);

#ifdef __cplusplus
}
#endif

#endif /* PICOTCP_H_ */
/** @} */
