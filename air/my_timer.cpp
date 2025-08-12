#include "my_timer.h"
#include <Arduino.h>

void my_timer_init(my_timer* timer) {
    timer->interval = 0;
    timer->unit = 60 * 1000; // Default unit is 60 seconds
    timer->enabled = 0;//false;
    timer->repeat = 0;//false;
    timer->start = 0;
}

int my_timer_is_time(my_timer* timer) {
    if (!timer || !timer->enabled) return 0;  // null check

    int finished = (millis() > timer->start + timer->interval * timer->unit) && timer->enabled;
    if (finished) {
        if (timer->repeat) {
            my_timer_start(timer);  // Restart if repeat is enabled
        } else {
            my_timer_disable(timer);  // Disable if no repeat
        }
    }
    return finished;
}

int my_timer_pending_time(my_timer* timer) {
    int val = (timer->start + timer->interval * timer->unit - millis());
    if (!timer->enabled)
        val = 0;
    return val / timer->unit;
}

void my_timer_set_interval(my_timer* timer, int interval) {
    timer->interval = interval;
}

int my_timer_get_interval(my_timer* timer) {
    int val = 0;
    if (timer->enabled) 
        val = timer->interval;
    return val;
}

void my_timer_set_unit(my_timer* timer, int unit) {
    timer->unit = unit;
}

void my_timer_start(my_timer* timer) {
    timer->enabled = 1;
    timer->start = millis();
}

void my_timer_enable(my_timer* timer) {
    timer->enabled = 1;
}

void my_timer_disable(my_timer* timer) {
    timer->enabled = 0;
}

int my_timer_is_enabled(my_timer* timer) {
    return timer->enabled;
}

void my_timer_repeat(my_timer* timer) {
    timer->repeat = 1;
}

void my_timer_dont_repeat(my_timer* timer) {
    timer->repeat = 0;
}
