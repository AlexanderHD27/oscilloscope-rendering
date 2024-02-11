#ifndef SIGNAL_GENERATION_HEADER4
#define SIGNAL_GENERATION_HEADER

#define FLAG_VALUE 0xaaaaaaaa

enum instruction_Type {
    POINT = 1,
    LINE = 2,
    CUBIC = 3,
    QUADRATIC = 4,
    SIN = 5
};

typedef struct {
    void * instruction_list;
    size_t instruction_list_size;
} instructions;

typedef struct {
    uint16_t (*buffer)[];
    size_t buffer_size;
    instructions ins;
    bool ready_for_dealloc; 
} generation_job;

// Core 1 functions

void gen_mainloop_entry1();
void process_generation_job(generation_job * job);

// Core0 functions

int send_job_to_processing(generation_job * job);
generation_job * recv_finished_job();

/**
 * Inits the required buffers. Should be run before the gen_mainloop_entry1 is started
*/
void gen_init();

#endif 