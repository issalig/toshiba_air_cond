#ifndef MYSIMPLETIMER_H
#define MYSIMPLETIMER_H

typedef struct {
    unsigned long _start;
    int _interval;
    int _enabled;// = false;
    int _unit;// = 60 * 1000; // default unit is 60 seconds
    int _repeat;// = false;
} MySimpleTimer;

void my_timer_init(MySimpleTimer* timer);
int my_timer_is_time(MySimpleTimer* timer);
int my_timer_pending_time(MySimpleTimer* timer);
void my_timer_set_interval(MySimpleTimer* timer, int interval);
int my_timer_get_interval(MySimpleTimer* timer);
void my_timer_set_unit(MySimpleTimer* timer, int unit);
void my_timer_start(MySimpleTimer* timer);
void my_timer_enable(MySimpleTimer* timer);
void my_timer_disable(MySimpleTimer* timer);
int my_timer_is_enabled(MySimpleTimer* timer);
void my_timer_repeat(MySimpleTimer* timer);
void my_timer_dont_repeat(MySimpleTimer* timer);

#endif