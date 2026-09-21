#include "utils.h"

int horizon_check(int time, int horizon, int toggle) {
    // toggle is 0 for training, 1 for untrained

    if (time + horizon >= LAST) {return 0;}
    
    int here = train_rule(time),
        there = train_rule(time + horizon);

    if (toggle != here) {return 0;}

    return toggle ? there : !there;
}

// Functions return 0 if trained data should access, 1 if untrained should

// Excluded timeframe_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-

// These 2 must be UNIX TIME in milliseconds
// Data is from 2016 till present
// https://www.epochconverter.com/
double TRAINING_LOWER_BOUND_TIMESTAMP = 1704067201000; //2024-01-01
double TRAINING_UPPER_BOUND_TIMESTAMP = 1735689601000; //2025-01-01
// From them calculate the indexes
int TRAINING_LOWER_BOUND = 0;
int TRAINING_UPPER_BOUND = 0;

int one_timeframe(int time) {
    if (!TRAINING_UPPER_BOUND || !TRAINING_LOWER_BOUND) {
        
        TRAINING_LOWER_BOUND = find_index(TRAINING_LOWER_BOUND_TIMESTAMP);
        TRAINING_UPPER_BOUND = find_index(TRAINING_UPPER_BOUND_TIMESTAMP);
    }
    return (TRAINING_LOWER_BOUND <= time && time < TRAINING_UPPER_BOUND) ? 1 : 0;
}

// Walk forward folds_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-
double TRAINING_TIMESTAMP = 1514764801000; //2018-01-01
int TRAINING_INDEX = 0;

int walk_forward_folds(int time) {
    if (!TRAINING_INDEX) {TRAINING_INDEX = find_index(TRAINING_TIMESTAMP);}

    int relative_time = time - TRAINING_INDEX;
    return relative_time < 100 ? 0 : relative_time % 170 < 40;
}

// Choose strategy from the above here
int train_rule(int time) {
    return one_timeframe(time);
}
