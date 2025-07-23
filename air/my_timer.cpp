#include "my_timer.h"
#include <Arduino.h>

void my_timer_init(MySimpleTimer* timer) {
    timer->_interval = 0;
    timer->_unit = 60 * 1000; // Default unit is 60 seconds
    timer->_enabled = 0;//false;
    timer->_repeat = 0;//false;
    timer->_start = 0;
}

int my_timer_is_time(MySimpleTimer* timer) {
    if (!timer || !timer->_enabled) return 0;  // null check

    int finished = (timer->_start + timer->_interval * timer->_unit <= millis()) && timer->_enabled;
    if (finished) {
        if (timer->_repeat) {
            my_timer_start(timer);  // Restart if repeat is enabled
        } else {
            my_timer_disable(timer);  // Disable if no repeat
        }
    }
    return finished;
}

int my_timer_pending_time(MySimpleTimer* timer) {
    int val = (timer->_start + timer->_interval * timer->_unit - millis());
    if (!timer->_enabled)
        val = 0;
    return val / timer->_unit;
}

void my_timer_set_interval(MySimpleTimer* timer, int interval) {
    timer->_interval = interval;
}

int my_timer_get_interval(MySimpleTimer* timer) {
    int val = 0;
    if (timer->_enabled) 
        val = timer->_interval;
    return val;
}

void my_timer_set_unit(MySimpleTimer* timer, int unit) {
    timer->_unit = unit;
}

void my_timer_start(MySimpleTimer* timer) {
    timer->_enabled = 1;
    timer->_start = millis();
}

void my_timer_enable(MySimpleTimer* timer) {
    timer->_enabled = 1;
}

void my_timer_disable(MySimpleTimer* timer) {
    timer->_enabled = 0;
}

int my_timer_is_enabled(MySimpleTimer* timer) {
    return timer->_enabled;
}

void my_timer_repeat(MySimpleTimer* timer) {
    timer->_repeat = 1;
}

void my_timer_dont_repeat(MySimpleTimer* timer) {
    timer->_repeat = 0;
}
