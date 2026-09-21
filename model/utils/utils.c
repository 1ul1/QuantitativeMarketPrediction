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

double min(double x, double y) {
    return x >= y ? y : x;
}

double max(double x, double y) {
    return x >= y ? x : y;
}

void predict(double* res, double* features) {

    for (int i = 0; i < NR_HORIZONS; i += 1) {
        res[i] = 0;
    }

    for (int idx = 0; idx < NR_HORIZONS; idx += 1) {
        
        for (int i = 0; i < NR_FEATURES; i += 1) {
            res[idx] += WEIGHTS->weights[i + idx * NR_FEATURES] * features[i];
        }
        res[idx] += WEIGHTS->bias[idx];
    }
}

void expect(double* res, int* count, int time, Company* company, int toggle) {

    for (int i = 0; i < NR_HORIZONS; i += 1) {
        res[i] = 0;
    }

    for (int i = 0; i < NR_HORIZONS; i += 1) {
        if (horizon_check(time, HORIZONS[i], toggle)) {
            res[i] = log(company->samples[time + HORIZONS[i]].c  / company->samples[time].c);
            count[i] += 1;
        }
    }
}

// toggle logic inverted here, to be fixed!
void error(double* ans, double*** features, int toggle) {
    double* computed = (double*)malloc(sizeof(double) * NR_HORIZONS);
    double* expected = (double*)malloc(sizeof(double) * NR_HORIZONS);
    int* count = (int*)calloc(sizeof(int), NR_HORIZONS);
    
    int start = 20, end = MARKET->count - 20;
    Companies* c = COMPANIES;

    if (toggle == 0) {c = UNTRAINED_COMPANIES;}

    for (int time = start; time < end; time += 1) {

        if (
            (toggle == 1 && (train_rule(time)))
            ||
            (toggle == 0 && (!train_rule(time)))
        ) {continue;}
        
        for (int i = 0; i < c->len_companies; i += 1) {
            predict(computed, features[time][i]);
            expect(expected, count, time, &(c->companies[i]), !toggle);

            for (int idx = 0; idx < NR_HORIZONS; idx += 1) {
                if (horizon_check(time, HORIZONS[idx], !toggle)) {
                    ans[idx] += pow(computed[idx] - expected[idx], 2);
                }
            }
        }
    }

    for (int idx = 0; idx < NR_HORIZONS; idx += 1) {
        ans[idx] = count[idx] ? sqrt(ans[idx] / count[idx]) : 1;
    }

    free(computed);
    free(expected);
    free(count);
}

// toggle logic inverted here, to be fixed!
void baseline_error(double* ans, int toggle) {
    double* expected = (double*)malloc(sizeof(double) * NR_HORIZONS);
    int* count = (int*)calloc(sizeof(int), NR_HORIZONS);
    
    int start = 20, end = MARKET->count - 20;
    Companies* c = COMPANIES;

    if (toggle == 0) {
        c = UNTRAINED_COMPANIES;
    }

    for (int time = start; time < end; time += 1) {
        
        if (
            (toggle == 1 && (train_rule(time)))
            ||
            (toggle == 0 && (!train_rule(time)))
        ) {continue;}
        
        for (int i = 0; i < c->len_companies; i += 1) {
            expect(expected, count, time, &(c->companies[i]), !toggle);

            for (int idx = 0; idx < NR_HORIZONS; idx += 1) {
                if (horizon_check(time, HORIZONS[idx], !toggle)) {
                    ans[idx] += pow(0 - expected[idx], 2);
                }
            }
        }
    }

    for (int idx = 0; idx < NR_HORIZONS; idx += 1) {
        ans[idx] = count[idx] ? sqrt(ans[idx] / count[idx]) : 1;
    }

    free(expected);
    free(count);
}

void print_skill(double*** features) {
    double* ans1 = calloc(NR_HORIZONS, sizeof(double));
    double* ans2 = calloc(NR_HORIZONS, sizeof(double));
    double skill = 0;
    
    error(ans1, features, 0);
    baseline_error(ans2, 0);

    printf("\n");
    
    for (int i = 0; i < NR_HORIZONS; i += 1) {
        printf("Delta RMSE %.16f for Layer %d\n", (1 -  ans1[i] / ans2[i]) * 100,i);
        skill += ans1[i] / ans2[i];
    }
    
    free(ans1);
    free(ans2);
}
