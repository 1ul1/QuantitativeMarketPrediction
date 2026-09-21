#include "utils.h"

int is_today(const Company* company) {
    time_t  current_time = time(NULL),
            given_time = (
                (time_t)(company->samples[company->count - 1].t / 1000.0)
            );
    struct tm given, current; 
    localtime_r(&given_time, &given);
    localtime_r(&current_time, &current);

    return (
        given.tm_yday == current.tm_yday && given.tm_year == current.tm_year
    );
}

int find_index(double t) {
    for (int i = 0; i < MARKET->count; i += 1) {
        if (MARKET->samples[i].t >= t) {
            return i;
        }
    }
    return -1;
}
