/*
 * Copyright (c) Branko Premzel.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * @file    rte_com_STM32_driver.h
 * @author  Branko Premzel
 * @brief   Send data over a serial channel using the DMA unit of the STM32 processor.
 * @note    The code has been tested on the STM32C0 only. Check if the DMA and
 *          USART of your STM32 device is compatible with this implementation.
 *******************************************************************************/

#ifndef RTE_COM_STM32_DRIVER_H_
#define RTE_COM_STM32_DRIVER_H_

#include "main.h"

/***
 * @brief Send data over USART using DMA.
 *
 * @param  p_buffer  Pointer to data buffer
 * @param  size      Size of data in the buffer
 *
 * @note The 'size' parameter must not be greater than 0xFFFF, as this is the maximum number
 *       of bytes that can be sent with STM32 DMA in a packet.
 *
 * The following macros have to be defined in a header file - example for the STM32C0:
 *    #define STM32_DMA_UNIT          DMA1
 *    #define STM32_LL_DMA_CHANNEL    LL_DMA_CHANNEL_1
 *    #define STM32_USART             USART2
 *
 */

__STATIC_FORCEINLINE void rte_com_send_data(const uint8_t *p_buffer, uint32_t size)
{
    // Disable DMA1 Channel to reconfigure it
    LL_DMA_DisableChannel(STM32_DMA_UNIT, STM32_LL_DMA_CHANNEL);
    LL_DMA_ConfigAddresses(STM32_DMA_UNIT, STM32_LL_DMA_CHANNEL,
                           (uint32_t)p_buffer,
                           LL_USART_DMA_GetRegAddr(STM32_USART, LL_USART_DMA_REG_DATA_TRANSMIT),
                           LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetDataLength(STM32_DMA_UNIT, STM32_LL_DMA_CHANNEL, size);

    // Enable DMA Channel
    LL_DMA_EnableChannel(STM32_DMA_UNIT, STM32_LL_DMA_CHANNEL);

    // Enable USART DMA transmit request (start transmission)
    LL_USART_EnableDMAReq_TX(STM32_USART);
}

#endif /* RTE_COM_STM32_DRIVER_H_ */

/*==== End of file ====*/
