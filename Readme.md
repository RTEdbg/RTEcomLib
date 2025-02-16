## Functions for transferring recorded data to the host via a serial channel

The files in this folder contain an implementation of data transfer over the serial channel. It is used to transfer data logged by the RTEdbg library to a host. See the implementation of the *rte_com_byte_received()* function in [rte_com.c](./rte_com.c). A DMA data transfer driver for the STM32 family is also included. The programmer must initialize the serial channel peripherals, the interrupt handler, and the DMA unit if it is used for data transfers. This is a minimalist implementation suitable for embedded systems with limited resources.
An initialization example can be found in the demo project.

The demo code assumes that the microcontroller has a DMA device that can send blocks of memory over the UART (see Note 3). Large memory blocks can be sent to the host without overloading the CPU. Receiving data is handled by the interrupt handler. The rte_com_byte_received() data processing function is optimized for very fast execution to minimize CPU load.

Functionality for timeout of data reception (sample implementation) is also provided, in case of noisy serial line and incomplete messages (commands) received from the host. See the *main.c* file.

The code is optimized to use as little Flash and RAM as possible. In this demo project (STM32C071 with Cortex M0+ core), the size of the function that handles receiving and sending data to/from the host is only about 200 bytes. The problem for resource-constrained microcontrollers with small flash memory is the code generators. Taking the STM32 family as an example, the code required to initialize the serial channel and the DMA unit is several times larger, even with the option of using LL (low level) drivers. With classic HAL drivers, the initialization code is much larger. Microcontrollers with limited resources require hand-optimized versions of the serial peripheral and DMA initialization code.

See also the **[Readme](https://github.com/RTEdbg/RTEcomLib_NUCLEO_C071RB_Demo/blob/master/README.md)** file in the demo folder. It contains the complete demo for the NUCLEO-C071RB board.

Data transfer to the host is possible with the **[RTEcomData](https://github.com/RTEdbg/RTEcomData)** utility.

### Communication Implementation
All commands sent from the host side are ten bytes long. They contain a command, checksum, address, and data values. See rte_com.c and rte_com.h for implementation details.

Only two commands are required to transfer logged data to the host, to change the message filter, and to reset/restart logging in post-mortem or single shot mode. The additional (optional) commands allow reading from embedded system memory or peripherals and writing 32-bit data to the embedded system. They can be used for additional diagnostic purposes (e.g., reading variables or complete buffers, setting triggers, influencing the embedded system code to start a specific procedure or test its robustness, etc.).

The maximum data block size that can be transferred with one command in this example is 65535 (0xFFFF), which is the maximum data block size for DMA units in the STM32. The limit is 65525 (65535-10) when using single wire communication. The size of the *g_rtedbg* data structure in which the data is logged can be larger, because it is possible to select which part of the data structure is transferred with a single command from the host (address parameter).

#### Notes
1. All messages sent from the host side contain a checksum. It is important for the data sent to the embedded system because we can manipulate it with commands.
2. Sending data to the host without a checksum is implemented to reduce the impact of executing the data send code on the embedded system. Checksum calculation can consume a lot of CPU time when transmitting a large message size, and the embedded system software may also modify the message content in the meantime. Almost all 32-bit microcontrollers have a DMA unit that can be enabled to send the entire message without CPU intervention (without affecting code execution when using two-wire communication). Data transfer with DMA and simple (and fast) implementation of checksum calculation is not possible. If some data is very important, read it twice from the host side and compare the results. Also enable serial port parity checking if necessary.
3. In single-wire communication, the microcontroller receives every byte it sends to the host. At high transfer rates, receive data interrupts may occur frequently during transmission. Therefore, the receive function must be coded as optimally as possible. It is very important how the interrupt handler is implemented to receive from the serial channel where the communication with the host is interrupted. Some solutions with HAL functions and callbacks have a lot of overload.
4. The functionality to reset the circular buffer (set it to 0xFFFFFFFFFF) in the g_rtedbg structure has been deliberately not implemented because resetting it takes too much time and could have too much impact on code execution in the embedded system.
5. The code is documented in Doxygen style. However, the project is not yet ready to automatically generate documentation with Doxygen.

## How to contribute or get help
Follow the [Contributing Guidelines](https://github.com/RTEdbg/RTEdbg/blob/master/docs/CONTRIBUTING.md) for bug reports and feature requests regarding the RTEcomLib repository. 
Please use [RTEdbg.freeforums.net](https://rtedbg.freeforums.net/) for general discussions about the RTEdbg toolkit.

When asking a support question, be clear and take the time to explain your problem properly. If your problem is not strictly related to this project, we recommend that you use [Stack Overflow](https://stackoverflow.com/), [r/Embedded](https://www.reddit.com/r/embedded/) or similar question-and-answer website instead. First, check if the [RTEdbg manual](https://github.com/RTEdbg/RTEdbg/releases/download/Documentation/RTEdbg.library.and.tools.manual.pdf) already contains an answer to your question or a solution to your problem.

## Repository Structure
This repository contains the RTEcomLib - functions for logged data transfer to the host. The subfolder **Portable** contains header files with the CPU specific specific drivers.

See the list of **[RTEdbg toolkit Repositories](https://github.com/RTEdbg/RTEdbg/blob/master/README.md#Repository-Structure)**.
