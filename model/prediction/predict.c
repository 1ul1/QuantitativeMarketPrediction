#include "utils.h"

void populate(double* features) {
    if (PREDICTION->days[0].bias == 0 && PREDICTION->days[0].sd == 0) {exit(1);}
    
    double* computed = (double*)malloc(sizeof(double) * PREDICTION->len_days);
    int last = LAST - 1;
    
    predict(computed, features);

    PREDICTION->today= TODAY;

    for (int i = 0; i < PREDICTION->len_days; i += 1) {
        PREDICTION->days[i].day = HORIZONS[i];
        PREDICTION->days[i].starting_price = COMPANY->samples[last].c;
        PREDICTION->days[i].expected_return = computed[i] + PREDICTION->days[i].bias;
        PREDICTION->days[i].expected_price = COMPANY->samples[last].c * exp(computed[i] + PREDICTION->days[i].bias);
        PREDICTION->days[i].forecast_strength = (
            (computed[i] + PREDICTION->days[i].bias) /  PREDICTION->days[i].sd
        );
        // Conversion to $
        PREDICTION->days[i].sd = COMPANY->samples[last].c / 2 * (
                exp(PREDICTION->days[i].expected_return + PREDICTION->days[i].sd)
                -
                exp(PREDICTION->days[i].expected_return - PREDICTION->days[i].sd)
        );
        PREDICTION->days[i].bias = COMPANY->samples[last].c * (exp(PREDICTION->days[i].bias) - 1);
    }
    
    free(computed);
}

void populate_error_metrics(double** features) {
    double* computed = (double*)malloc(sizeof(double) * PREDICTION->len_days);
    double* expected = (double*)malloc(sizeof(double) * PREDICTION->len_days);
    
    int* range = (int*)calloc(PREDICTION->len_days, sizeof(int));
    
    // -------------------------------------------------------------------------------------------
    // BIAS
    for (int time = 20; time < MARKET->count - 20; time += 1) {

        if (train_rule(time) != 1) {continue;}
        
        predict(computed, features[time]);
        expect(expected, time, COMPANY, 1);

        for (int i = 0; i < NR_HORIZONS; i += 1) {
            if (horizon_check(time, HORIZONS[i], 1)) {
                PREDICTION->days[i].bias += expected[i] - computed[i];
                range[i] += 1;
            }
        }
    }

    // BIAS average
    for (int i = 0; i < NR_HORIZONS; i += 1) {
        PREDICTION->days[i].bias /= (range[i]);
    }

    // Standard Deviation
    for (int time = 20; time < MARKET->count - 20; time += 1) {

        if (train_rule(time) != 1) {continue;}
        
        predict(computed, features[time]);
        expect(expected, time, COMPANY, 1);

        for (int i = 0; i < NR_HORIZONS; i += 1) {
            if (horizon_check(time, HORIZONS[i], 1)) {
                PREDICTION->days[i].sd += pow(expected[i] - computed[i] - PREDICTION->days[i].bias, 2);
            }
        }
    }

    for (int i = 0; i < NR_HORIZONS; i += 1) {
        PREDICTION->days[i].sd = sqrt(PREDICTION->days[i].sd / range[i]);
    }

    free(range);
    free(computed);
    free(expected);
}
