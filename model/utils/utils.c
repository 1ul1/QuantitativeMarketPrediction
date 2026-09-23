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

    memset(res, 0, sizeof(double) * NR_HORIZONS);

    for (int idx = 0; idx < NR_HORIZONS; idx += 1) {
        
        for (int i = 0; i < NR_FEATURES; i += 1) {
            res[idx] += WEIGHTS->weights[i + idx * NR_FEATURES] * features[i];
        }
        res[idx] += WEIGHTS->bias[idx];
    }
}

void expect(double* res, int time, Company* company, int toggle) {

    memset(res, 0, sizeof(double) * NR_HORIZONS);

    for (int i = 0; i < NR_HORIZONS; i += 1) {
        if (horizon_check(time, HORIZONS[i], toggle)) {
            res[i] = log(company->samples[time + HORIZONS[i]].c  / company->samples[time].c);
        }
    }
}

// toggle logic inverted here, to be fixed!
void error(double* ans, double*** features, int toggle) {
    double* computed = (double*)malloc(sizeof(double) * NR_HORIZONS);
    double* expected = (double*)malloc(sizeof(double) * NR_HORIZONS);
    int* count = (int*)calloc(NR_HORIZONS, sizeof(int));
    
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
            expect(expected, time, &(c->companies[i]), !toggle);

            for (int idx = 0; idx < NR_HORIZONS; idx += 1) {
                if (horizon_check(time, HORIZONS[idx], !toggle)) {
                    ans[idx] += pow(computed[idx] - expected[idx], 2);
                    count[idx] += 1;
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
    int* count = (int*)calloc(NR_HORIZONS, sizeof(int));
    
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
            expect(expected, time, &(c->companies[i]), !toggle);

            for (int idx = 0; idx < NR_HORIZONS; idx += 1) {
                if (horizon_check(time, HORIZONS[idx], !toggle)) {
                    ans[idx] += pow(0 - expected[idx], 2);
                    count[idx] += 1;
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

// only for untrained
// time series & its transpose aka information coefficient
void ts_and_ic(double* ans1, double* ans2, double*** features) {
    double  ***x = (double***)malloc(sizeof(double**) * MARKET->count),
            ***y = (double***)malloc(sizeof(double**) * MARKET->count),
            *range  = (double*)calloc(NR_HORIZONS, sizeof(double));

    for (int time = 0; time < MARKET->count; time += 1) {
        x[time] = (double**)malloc(sizeof(double*) * UNTRAINED_COMPANIES->len_companies);
        y[time] = (double**)malloc(sizeof(double*) * UNTRAINED_COMPANIES->len_companies);
        for (int i = 0; i < UNTRAINED_COMPANIES->len_companies; i += 1) {
            x[time][i] = (double*)calloc(NR_HORIZONS, sizeof(double));
            y[time][i] = (double*)calloc(NR_HORIZONS, sizeof(double));
        }
    }

    #pragma omp parallel for num_threads(NR_THREADS)
    for (int time = 20; time < MARKET->count; time += 1) {
        for (int i = 0; i < UNTRAINED_COMPANIES->len_companies; i += 1) {
            predict(x[time][i], features[time][i]);
            expect(y[time][i], time, &(UNTRAINED_COMPANIES->companies[i]), 1);
        }
    }
    for (int time = 20; time < MARKET->count; time += 1) {
        for (int idx = 0; idx < NR_HORIZONS; idx += 1) {
            if (horizon_check(time, HORIZONS[idx], 1)) {
                range[idx] += 1;
            }
        }
    }
    for (int idx = 0; idx < NR_HORIZONS; idx += 1) {
        if (range[idx] == 0) {
            printf("wtf");
            exit(-2);
        }
    }

    // ts - Perason per company accross all days
    double  *mean_x = (double*)calloc(NR_HORIZONS, sizeof(double)),
            *mean_y = (double*)calloc(NR_HORIZONS, sizeof(double));

    double  *numerator = (double*)calloc(NR_HORIZONS, sizeof(double)),
            *sd_x = (double*)calloc(NR_HORIZONS, sizeof(double)),
            *sd_y = (double*)calloc(NR_HORIZONS, sizeof(double));
        
    for (int i = 0; i < UNTRAINED_COMPANIES->len_companies; i += 1) {

        memset(mean_x, 0, sizeof(double) * NR_HORIZONS);
        memset(mean_y, 0, sizeof(double) * NR_HORIZONS);
        
        for (int time = 20; time < MARKET->count; time += 1) {
            for (int idx = 0; idx < NR_HORIZONS; idx += 1) {
                if (horizon_check(time, HORIZONS[idx], 1)) {
                    mean_x[idx] += x[time][i][idx];
                    mean_y[idx] += y[time][i][idx];
                }
            }
        }
        for (int idx = 0; idx < NR_HORIZONS; idx += 1) {
            mean_x[idx] /= range[idx];
            mean_y[idx] /= range[idx];
        }

        memset(numerator, 0, sizeof(double) * NR_HORIZONS);
        memset(sd_x, 0, sizeof(double) * NR_HORIZONS);
        memset(sd_y, 0, sizeof(double) * NR_HORIZONS);
        
        for (int time = 20; time < MARKET->count; time += 1) {
            for (int idx = 0; idx < NR_HORIZONS; idx += 1) {
                if (horizon_check(time, HORIZONS[idx], 1)) {
                    double aux_x, aux_y;
                    aux_x = x[time][i][idx] - mean_x[idx];
                    aux_y = y[time][i][idx] - mean_y[idx];
                    numerator[idx] += aux_x * aux_y;
                    sd_x[idx] += pow(aux_x, 2);
                    sd_y[idx] += pow(aux_y, 2);
                }
            }
        }
        for (int idx = 0; idx < NR_HORIZONS; idx += 1) {
            double denominator = (sqrt(sd_x[idx]) * sqrt(sd_y[idx]));
            
            ans1[idx] += (denominator <= 1e-15) ? 0 : numerator[idx] / denominator;
        }
    }
    for (int idx = 0; idx < NR_HORIZONS; idx += 1) {
        ans1[idx] /= UNTRAINED_COMPANIES->len_companies;
    }
    
    // ic - Pearson per day between all companies
    memset(mean_x, 0, sizeof(double) * NR_HORIZONS);
    memset(mean_y, 0, sizeof(double) * NR_HORIZONS);

    for (int time = 20; time < MARKET->count; time += 1) {

        memset(mean_x, 0, sizeof(double) * NR_HORIZONS);
        memset(mean_y, 0, sizeof(double) * NR_HORIZONS);
        
        for (int i = 0; i < UNTRAINED_COMPANIES->len_companies; i += 1) {
            for (int idx = 0; idx < NR_HORIZONS; idx += 1) {
                if (horizon_check(time, HORIZONS[idx], 1)) {
                    mean_x[idx] += x[time][i][idx];
                    mean_y[idx] += y[time][i][idx];
                }
            }
        }
        for (int idx = 0; idx < NR_HORIZONS; idx += 1) {
            mean_x[idx] /= UNTRAINED_COMPANIES->len_companies;
            mean_y[idx] /= UNTRAINED_COMPANIES->len_companies;
        }
        
        memset(numerator, 0, sizeof(double) * NR_HORIZONS);
        memset(sd_x, 0, sizeof(double) * NR_HORIZONS);
        memset(sd_y, 0, sizeof(double) * NR_HORIZONS);
        
        for (int i = 0; i < UNTRAINED_COMPANIES->len_companies; i += 1) {
            for (int idx = 0; idx < NR_HORIZONS; idx += 1) {
                if (horizon_check(time, HORIZONS[idx], 1)) {
                    double aux_x, aux_y;
                    aux_x = x[time][i][idx] - mean_x[idx];
                    aux_y = y[time][i][idx] - mean_y[idx];
                    numerator[idx] += aux_x * aux_y;
                    sd_x[idx] += pow(aux_x, 2);
                    sd_y[idx] += pow(aux_y, 2);
                }
            }
        }
        for (int idx = 0; idx < NR_HORIZONS; idx += 1) {
            double denominator = (sqrt(sd_x[idx]) * sqrt(sd_y[idx]));
            
            ans2[idx] += (denominator <= 1e-15) ? 0 : numerator[idx] / denominator;
        }
    }
    for (int idx = 0; idx < NR_HORIZONS; idx += 1) {
        ans2[idx] /= range[idx];
    }

    // Freeing
    free(mean_x);
    free(mean_y);
    free(numerator);
    free(sd_x);
    free(sd_y);

    for (int time = 0; time < MARKET->count; time += 1) {
        for (int i = 0; i < UNTRAINED_COMPANIES->len_companies; i += 1) {
            free(x[time][i]);
            free(y[time][i]);
        }
        free(x[time]);
        free(y[time]);
    }
    free(x);
    free(y);
    free(range);
}

void print_skill(double*** features) {
    
    double* ans1 = calloc(NR_HORIZONS, sizeof(double));
    double* ans2 = calloc(NR_HORIZONS, sizeof(double));
    
    error(ans1, features, 0);
    baseline_error(ans2, 0);
    
    for (int i = 0; i < NR_HORIZONS; i += 1) {
        printf("Delta RMSE for %d days ahead: %.16f\n", HORIZONS[i], (1 -  ans1[i] / ans2[i]) * 100);
    }

    memset(ans1, 0, sizeof(double) * NR_HORIZONS);
    memset(ans2, 0, sizeof(double) * NR_HORIZONS);
    ts_and_ic(ans1, ans2, features);

    for (int i = 0; i < NR_HORIZONS; i += 1) {
        printf("TS for %d days ahead: %.16f\n", HORIZONS[i], ans1[i]);
        printf("IC for %d days ahead: %.16f\n", HORIZONS[i], ans2[i]);
    }
    
    free(ans1);
    free(ans2);
}
