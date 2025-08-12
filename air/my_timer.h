#ifndef MYTIMER_H
#define MYTIMER_H

typedef struct {
    unsigned long start;
    int interval;
    int enabled;// = false;
    int unit;// = 60 * 1000; // default unit is 60 seconds
    int repeat;// = false;
} my_timer;

void my_timer_init(my_timer* timer);
int my_timer_is_time(my_timer* timer);
int my_timer_pending_time(my_timer* timer);
void my_timer_set_interval(my_timer* timer, int interval);
int my_timer_get_interval(my_timer* timer);
void my_timer_set_unit(my_timer* timer, int unit);
void my_timer_start(my_timer* timer);
void my_timer_enable(my_timer* timer);
void my_timer_disable(my_timer* timer);
int my_timer_is_enabled(my_timer* timer);
void my_timer_repeat(my_timer* timer);
void my_timer_dont_repeat(my_timer* timer);

#endif