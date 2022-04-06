#include "MySimpleTimer.h"

MySimpleTimer::MySimpleTimer(int interval) : _interval(interval) {  // interval is seconds
  _enabled = 0;
}



bool MySimpleTimer::isTime() {
  bool finished = (_start + _interval * _unit <= millis()) & _enabled;
  if (finished)
    if (_repeat)
      this->start();
    else
      this->disable();


  return finished;
}

int MySimpleTimer::pendingTime() {
  int val = (_start + _interval * _unit - millis());
  if (!this->_enabled)
    val = 0;
  return val / this->_unit;
}


void MySimpleTimer::setInterval(int interval) {
  _interval = interval;
}

int MySimpleTimer::getInterval() {
  int val = 0;

  if (this->_enabled) val = _interval;
  return val;
}


void MySimpleTimer::setUnit(int unit) {
  _unit = unit;
}

void MySimpleTimer::start() {
  _enabled = true;
  _start = millis();
}

void MySimpleTimer::enable() {
  _enabled = true;
}

void MySimpleTimer::disable() {
  _enabled = false;
}


bool MySimpleTimer::isEnabled() {
  return (_enabled);
}


void MySimpleTimer::repeat() {
  _repeat = true;
}

void MySimpleTimer::dontRepeat() {
  _repeat = false;
}
