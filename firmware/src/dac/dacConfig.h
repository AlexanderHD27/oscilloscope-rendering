#define PIO_CLK_DIV(freq) (SYS_CLK_MHZ*1000000)/(freq)

/** Configuration Stuff */

#define BUFFER_SIZE 0x1000

// How many buffers are in circulation (BUFFER_SIZE*2 * GENERAL_QUEUE_SIZE bytes are required)
#define GENERAL_QUEUE_SIZE 4

// How many bytes of instruction can be stored inside one instruction buffer
#define INSTRUCTION_BUF_SIZE 128

#define DEFAULT_QUEUE_WAIT_DURATION 32

// Frequency can be from 2 kHz to 133 MHz 
#define PIO_FREQ_KHZ 20000

// Every frame sent to the PIO cores is repeated FRAME_REPEAT times
#define FRAME_REPEAT 8
