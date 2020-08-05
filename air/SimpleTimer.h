
#include <Arduino.h>

class SimpleTimer {
    uint64_t _start;
    int _interval;
    bool _enabled;
    int _unit=60*1000;
    
public:

    /// Constructor, that initialize timer
    /// \param interval An interval in msec
    explicit SimpleTimer(int interval = 0);
      
    /// Check if timer is ready
    /// \return True if is timer is ready
    bool isTime();

    /// Return pending time
    int pendingTime();
    
    /// Set the time interval
    /// \param interval An interval in msec    
    void setInterval(int interval);

    /// Get interval
    int getInterval();

    /// Set unit
    void setUnit(int unit);
    
    /// Enable timer    
    void enable();

    /// Disable timer    
    void disable();
    
    /// Reset a timer
    void start();
};
