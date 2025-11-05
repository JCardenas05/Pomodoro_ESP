#ifndef POMOTASK_H
#define POMOTASK_H

#include <stdbool.h>

typedef void (*cb_counter_end_t)(void);

typedef struct 
{   
    int seconds;
    bool is_running;
    int timers_elapsed;
    cb_counter_end_t counter_callback;

} counter_time_t;

void pomotask_init(int seconds, cb_counter_end_t callback);
void pomotask_start();

#endif // POMOTASK_H