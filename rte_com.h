/*
 * Copyright (c) Branko Premzel.
 *
 * SPDX-License-Identifier: MIT
 */

/*********************************************************************************
 * @file    rte_com.h
 * @author  Branko Premzel
 * @brief   Functions to transmit log and other data over a serial channel.
 *          For additional documentation, see the Readme file.
 *******************************************************************************/

#ifndef RTE_COM_H
#define RTE_COM_H

typedef struct
{
    volatile uint16_t no_received;
        // no_received = 0 ... 9 => Index in the buffer starting with the &command
        // >10 => Number of characters to skip in the single-wire mode
    uint8_t command;        // Command - this is the first byte received from the host
    uint8_t checksum;       // The checksum is the XOR of 0xFF and the last eight bytes of
                            // the message (address and data word)
    uint32_t address;       // Address of data buffer to be transferred or variable to be set
    uint32_t data;          // Size of data (length) or data written to the address
} rtecom_recv_data_t;

typedef enum
{
    // The first two commands are mandatory.
    RTECOM_WRITE_RTEDBG,    // Write 32-bit data to g_rtedbg (e.g. set message filter or index)
                            // Address = index of 32-bit word
                            // Returns ACK if index is within g_rtedbg header, NACK otherwise
    RTECOM_READ_RTEDBG,     // Read data from g_rtedbg data structure
                            // Address is relative to the start of the g_rtgdb data structure.
                            // Returns: NN bytes (NN = data parameter – number of bytes requested)
                            //          or NACK if requested data not inside of g_rtedbg
    // Optional commands
    RTECOM_READ,            // Get data from the specified address ('data' = number of bytes)
                            // Returns: requested amount data or nothing if address not OK
    RTECOM_WRITE32,         // Write 32-bit data to the specified address
                            // Returns: ACK
    RTECOM_WRITE16,         // Write 16-bit data to the specified address
                            // Returns: ACK
    RTECOM_WRITE8,          // Write 8-bit data to the specified address
                            // Returns: ACK
    RTECOM_LAST_COMMAND
} rte_com_command_t;

#define RTECOM_CHECKSUM  0x0FU  // Initial checksum value

// Legend: ACK - acknowledge - sends RTECOM_CHECKSUM
//         NACK - sends the command value
// Note: NACK is returned if the command (e.g. RTECOM_READ) is not implemented or address not OK.

// Host always sends 10 bytes: command (8b), checksum (8b), address (32b), data (32b)
#define RTECOM_RECV_PACKET_LEN  10U

// Only if a timeout is implemented for receiving messages from the host, the following macro must be defined.
#if !defined RTECOM_LOG_TIME_LAST_DATA_RECEIVED
#define RTECOM_LOG_TIME_LAST_DATA_RECEIVED()
#endif

extern rtecom_recv_data_t g_rtecom;  // Working variable for rte_com_byte_received()


/**************************
 *  FUNCTION DEFINITIONS
 **************************/
void rte_com_byte_received(uint32_t data, uint32_t errors);
    // Callback function for processing of received data


#if (RTECOM_WRITE_ENABLED == 1) && (RTECOM_READ_ENABLED == 0)
#error "The RTECOM_READ_ENABLED must also be enabled if the RTECOM_WRITE_ENABLED is enabled."
#endif
#if (RTECOM_READ_FROM_PERIPHERALS == 1) && (RTECOM_READ_ENABLED == 0)
#error "The RTECOM_READ_FROM_PERIPHERALS can not be enabled without the RTECOM_READ_ENABLED."
#endif

#endif // RTE_COM_H

/*==== End of file ====*/
