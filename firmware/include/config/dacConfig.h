/**
 * @file dacConfig.h
 * @author AlexanderHD27
 * @brief Holds preprocess defines for Config
 * @version 0.1
 * @date 2024-02-15
 * 
 * @copyright Copyright (c) 2024
 * 
 * This contains configuration variable, that can be change by the users 
 * 
 */


/**
 * @brief Size of frame buffer
 * 
 * The Larger the frame buffer, the longer a frame is -> more memory is used
 * Warning: @ref BUFFER_SIZE * ( @ref GENERAL_QUEUE_SIZE + 2) * sizeof(uint16_t) bytes of memory must be available. The RP2040 has 260 KB of SRAM (Instruction memory is separate)
 */
#define BUFFER_SIZE 0x800

/**
 * @brief Number of Buffers in circulation + Queue Sizes
 * 
 * All for queue are allocated with spaces for @ref GENERAL_QUEUE_SIZE pointers to queue
 * (execept for the unused_Frame_buffer_queue, this holds @ref GENERAL_QUEUE_SIZE + 1 pointers for safety)
 */
#define GENERAL_QUEUE_SIZE 6

// How many bytes of instruction can be stored inside one instruction buffer
/**
 * @brief Size of each Instruction Buffer
 * 
 * One instruction contains at least 3 bytes up 11 bytes.
 * Warning: @ref INSTRUCTION_BUF_SIZE * @ref GENERAL_QUEUE_SIZE + 2* sizeof(uint8_t) bytes of memory must be available
 */
#define INSTRUCTION_BUF_SIZE 128

/**
 * @brief How many FreeRTOS-Ticks a recv should take befor restarting
 */
#define DEFAULT_QUEUE_WAIT_DURATION 32

/**
 * @brief Base Frequency of the PIO Cores in kHz
 * can be from 2 kHz to 133 MHz 
 */
#define PIO_FREQ_KHZ 10000

/**
 * @brief How many times a single frame is repeat before being swapped out
 * 
 * More frame repeats more time for processing, but lower refresh rate
 */
#define FRAME_REPEAT 16
