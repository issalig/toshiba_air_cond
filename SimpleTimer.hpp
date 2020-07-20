#include "SimpleTimer.h"

SimpleTimer::SimpleTimer(int interval) : _interval(interval) {
    _enabled = 0;
}



bool SimpleTimer::isTime() {  
    bool finished = (_start + _interval * _unit <= millis()) & _enabled;
    if (finished)
      this->disable();

    return finished;
}

int SimpleTimer::pendingTime() {
    int val = (_start + _interval * _unit - millis());
    if (!this->_enabled)
      val = 0;
    return val/this->_unit;
}


void SimpleTimer::setInterval(int interval) {
    _interval = interval;
}

int SimpleTimer::getInterval() {
    int val=0;
    
    if (this->_enabled) val=_interval;    
    return val;
}

void SimpleTimer::start() {
    _enabled = 1;
    _start = millis();
}

void SimpleTimer::enable() {
    _enabled = 1;
}

void SimpleTimer::disable() {
    _enabled = 0;    
}
