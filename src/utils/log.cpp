#include "log.hpp"

long    log_time(time_t input_time)
{
    time_t current_time;
    time(&current_time);
    long ret = current_time - input_time;
    return (ret);
}