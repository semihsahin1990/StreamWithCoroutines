#ifndef STREAMC_EXPERIMENT_AUTOTIMER_H
#define STREAMC_EXPERIMENT_AUTOTIMER_H

#include <boost/chrono.hpp>

namespace streamc { namespace experiment
{
    class AutoTimer
    {
    public:
        AutoTimer();
        AutoTimer(bool start);
        void start();
        void stop();
        double getCPUTimeInSeconds();
        double getRealTimeInSeconds();
        double getCPUUtilization();
    private:
        boost::chrono::process_cpu_clock::time_point time_;
        boost::chrono::process_cpu_clock::time_point end_;
    };
}}

#endif /* STREAMC_EXPERIMENT_AUTOTIMER_H */
