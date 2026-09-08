#include "utils.h"

// ---------------------------------------------------------------------------
//                                                               AUX Functions
// ---------------------------------------------------------------------------

void print_weights() {
    for (int i = 0; i < WEIGHTS->len_weights; i += 1) {
        printf("W %d - %.16f \n", i, WEIGHTS->weights[i]);  
    }

    for (int i = 0; i < WEIGHTS->len_bias; i += 1) {
        printf("B %d - %.16f \n", i, WEIGHTS->bias[i]);
    }
}


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


// ---------------------------------------------------------------------------
//                                                             Verify Training
// ---------------------------------------------------------------------------

void predict(double* res, double* features) {

    res[0] = 0;
    res[1] = 0;
    res[2] = 0;
    res[3] = 0;

    for (int idx = 0; idx < 4; idx += 1) {
        
        for (int i = 0; i < NR_FEATURES; i += 1) {
            res[idx] += WEIGHTS->weights[i + idx * NR_FEATURES] * features[i];
        }
        res[idx] += WEIGHTS->bias[idx];
    }
}

void expect(double* res, int time, Company* company) {

    res[0] = 0;
    res[1] = 0;
    res[2] = 0;
    res[3] = 0;

    if (time + 1 <= LAST) {
        res[0] = log(company->samples[time + 1].c  / company->samples[time].c);
    }
    if (time + 5 <= LAST) {
        res[1] = log(company->samples[time + 5].c  / company->samples[time].c); 
    }
    if (time + 10 <= LAST) {
        res[2] = log(company->samples[time + 10].c / company->samples[time].c);
    }
    if (time + 20 <= LAST) {
        res[3] = log(company->samples[time + 20].c / company->samples[time].c);
    }
}

void error(double* ans, double*** features, int toggle) {
    double* computed = (double*)malloc(sizeof(double) * 4);
    double* expected = (double*)malloc(sizeof(double) * 4);

    int start = 20, end = MARKET->count - 20;
    Companies* c = COMPANIES;

    if (toggle == 0) {
        start = TRAINING_LOWER_BOUND;
        end = TRAINING_UPPER_BOUND;
        c = UNTRAINED_COMPANIES;
    }

    int count = 0;

    for (int time = start; time < end; time += 1) {

        if (toggle == 1 && 
            TRAINING_LOWER_BOUND <= time && time < TRAINING_UPPER_BOUND
        ) {continue;}
        
        for (int i = 0; i < c->len_companies; i += 1) {
            predict(computed, features[time][i]);
            expect(expected, time, &(c->companies[i]));

            for (int idx = 0; idx < 4; idx += 1) {
                ans[idx] += pow(computed[idx] - expected[idx], 2);
            }
            count += 1;
        }
    }

    for (int idx = 0; idx < 4; idx += 1) {
        ans[idx] /= count;
        ans[idx] = sqrt(ans[idx]);
    }

    free(computed);
    free(expected);
}

void baseline_error(double* ans, int toggle) {
    double* expected = (double*)malloc(sizeof(double) * 4);

    int start = 20, end = MARKET->count - 20;
    Companies* c = COMPANIES;

    if (toggle == 0) {
        start = TRAINING_LOWER_BOUND;
        end = TRAINING_UPPER_BOUND;
        c = UNTRAINED_COMPANIES;
    }
    
    int count = 0;

    for (int time = start; time < end; time += 1) {
        
        if (toggle == 1 && 
            TRAINING_LOWER_BOUND <= time && time < TRAINING_UPPER_BOUND
        ) {continue;}
        
        for (int i = 0; i < c->len_companies; i += 1) {
            expect(expected, time, &(c->companies[i]));

            for (int idx = 0; idx < 4; idx += 1) {
                ans[idx] += pow(0 - expected[idx], 2);
            }
            count += 1;
        }
    }

    for (int idx = 0; idx < 4; idx += 1) {
        ans[idx] /= count;
        ans[idx] = sqrt(ans[idx]);
    }

    free(expected);
}

void print_skill(double*** features) {
    double* ans1 = calloc(4, sizeof(double));
    double* ans2 = calloc(4, sizeof(double));
    double skill = 0;
    
    error(ans1, features, 0);
    baseline_error(ans2, 0);

    printf("\n");
    
    for (int i = 0; i < 4; i += 1) {
        printf("Delta RMSE %.16f for Layer %d\n", (1 -  ans1[i] / ans2[i]) * 100,i);
        skill += ans1[i] / ans2[i];
    }
    
    free(ans1);
    free(ans2);
}
