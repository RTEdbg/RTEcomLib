/*
 * Copyright (c) Branko Premzel.
 *
 * SPDX-License-Identifier: MIT
 */

/********************************************************************************
 * @file    rte_com.c
 * @author  Branko Premzel
 * @brief   Functions to transmit log data structure over serial channel.
 *          For additional documentation, see the Readme file.
 ********************************************************************************/

#include "main.h"
#include "rtedbg_int.h"

#if RTE_ENABLED != 0
#include "memory.h"
#include "assert.h"
#include "rte_com.h"
#if defined RTECOM_SERIAL_DRIVER
#include RTECOM_SERIAL_DRIVER   // rte_com_send_data() function implementation
#endif

/* Global variable */
rtecom_recv_data_t g_rtecom;    // Working variable for rte_com_byte_received()

/***
 * @brief Processing of data received through the serial channel.
 *        This function is called, for example, from UART receive interrupt routine.
 *        After receiving the complete message, it sends a response, the length
 *        of which depends on the command. If it receives a bad command,
 *        or message with bad checksum it does not send a response.
 *
 * @param data   Data from the serial channel.
 * @param errors Non-zero value indicates a hardware reception error (framing error, parity error, etc.).
 */

void rte_com_byte_received(uint32_t data, uint32_t errors)
{
    uint32_t no_received = g_rtecom.no_received;

#if RTECOM_SINGLE_WIRE == 1
    if (no_received >= RTECOM_RECV_PACKET_LEN)
    {
        g_rtecom.no_received = no_received + 1U;    // Ignores the bytes it sends itself
        return;
    }
#endif

    if ((errors == 0U)                              // No error during reception?
        && (!((no_received == 0U) && (data >= RTECOM_LAST_COMMAND))) // Correct command?
#if RTECOM_SINGLE_WIRE != 1
        && (no_received < RTECOM_RECV_PACKET_LEN)   // Correct index?
#endif
       )
    {
        uint32_t checksum = g_rtecom.checksum ^ data;
        g_rtecom.checksum = (uint8_t)checksum;
        *(((uint8_t *)&g_rtecom.command) + no_received) = (uint8_t)data; // Assemble message
        ++no_received;
        g_rtecom.no_received = no_received;

        if (no_received < RTECOM_RECV_PACKET_LEN)
        {
             // If timeout is enabled, it stores the time of the data received.
            RTECOM_LOG_TIME_LAST_DATA_RECEIVED();   // Set time of the last received data
            return;
        }

        if (checksum == RTECOM_CHECKSUM)
        {
            // NACK is returned if the command (e.g. RTECOM_READ) is not implemented or address not OK.
            const uint8_t *p_data = &g_rtecom.command;  // NACK = command value

            uint32_t data_size = 1U;                    // Size of ACK or NACK
            uint32_t command = g_rtecom.command;

            if (command == RTECOM_WRITE_RTEDBG)
            {
                // Write 32-bit data to g_rtedbg (e.g. set message filter or index)
                // Returns ACK or NACK if address value is not OK
                if (g_rtecom.address < ((sizeof(g_rtedbg) - sizeof(g_rtedbg.buffer)) / 4U))
                {
                    *(((uint32_t *)&g_rtedbg) + g_rtecom.address) = g_rtecom.data;
                    p_data++;   // ACK (pointer to checksum = 0x0F)
                }
            }
            else if (command == RTECOM_READ_RTEDBG)
            {
                // Read data from g_rtedbg data structure
                // Returns: NN bytes (NN = data parameter – number of bytes requested)
                if ((data_size + g_rtecom.address) <= sizeof(g_rtedbg))
                {
                    p_data = ((const uint8_t *)&g_rtedbg) + g_rtecom.address;
                    data_size = g_rtecom.data;
                }
            }

            // Enable the following commands it if you also want to be able to read and write
            // to the embedded system's memory and peripherals for testing purposes.
            // Add check if address is aligned if the core does not support unaligned access.
            // Ensure that all data from the specified address to the end address (address + data)
            // is within the valid memory range (prevent exception in case of bad address).
#if RTECOM_READ_ENABLED == 1
            else if (command == RTECOM_READ)
            {
                // Read data from the specified address
                // Returns: NN=size data bytes
                uint32_t size = g_rtecom.data;
                uint32_t address = g_rtecom.address;
                data_size = size;
                p_data = (const uint8_t *)address;
#if RTECOM_READ_FROM_PERIPHERALS == 1
                if ((size == 2U) && ((address & 1U) == 0U))
                {
                    g_rtecom.data = *(uint16_t *)address;
                    p_data = (const uint8_t *)&g_rtecom.data;
                }
                else if ((size == 4U) && ((address & 3U) == 0U))
                {
                    g_rtecom.data = *(uint32_t *)address;
                    p_data = (const uint8_t *)&g_rtecom.data;

                }
#endif // RTECOM_READ_FROM_PERIPHERALS == 1
            }
#if RTECOM_WRITE_ENABLED == 1
            else if (command == RTECOM_WRITE32)
            {
                // Write 32-bit data to the specified address
                // Returns: ACK
                *(uint32_t *)g_rtecom.address = g_rtecom.data;
                p_data++;   // ACK (pointer to checksum = 0x0F)
            }
            else if (command == RTECOM_WRITE16)
            {
                // Write 16-bit data to the specified address
                // Returns: ACK
                *(uint16_t *)g_rtecom.address = (uint16_t)g_rtecom.data;
                p_data++;   // ACK (pointer to checksum = 0x0F)
            }
            else if (command == RTECOM_WRITE8)
            {
                // Write 8-bit data to the specified address
                // Returns: ACK
                    *(uint8_t *)g_rtecom.address = (uint8_t)g_rtecom.data;
                p_data++;   // ACK (pointer to checksum = 0x0F)
            }
#endif  // RTECOM_WRITE_ENABLED == 1
#endif  // RTECOM_READ_ENABLED == 1

            rte_com_send_data(p_data, data_size);   // Send the data to host

#if RTECOM_SINGLE_WIRE == 1
            // Set the number of bytes that have to be discarded before reception starts again.
            g_rtecom.no_received = (uint32_t)(-(int32_t)data_size);
            return;
#endif
        }
    }

    g_rtecom.no_received = 0U;
}

#endif // RTE_ENABLED != 0

/*==== End of file ====*/
