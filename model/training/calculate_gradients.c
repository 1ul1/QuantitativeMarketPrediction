#include "utils.h"

void adjust_updates(const Company* company,
    double* updates,
    const double* features,
    int time,
    int prediction_time,
    int start,
    int end
) {

    double prediction = 0, expected = log(company->samples[prediction_time].c / company->samples[time].c);
   
    for (int i = start; i < end; i += 1) {
        prediction += WEIGHTS->weights[i] * features[i - start];
    }
    prediction += WEIGHTS->bias[start / NR_FEATURES];

    double error = prediction - expected;

    // printf("ERROR %.16f \n", error);

    for (int i = start; i < end; i += 1) {
        updates[i] += (error * features[i - start] + LAMBDA * WEIGHTS->weights[i]) / NR_COMPANIES;
    }
    updates[WEIGHTS->len_weights + start / NR_FEATURES] += error / NR_COMPANIES;
}

void process_one_company(const Company* company, double* updates, int time, double* features) {

    for (int i = 0; i < NR_HORIZONS; i += 1) {
        if (horizon_check(time, HORIZONS[i], 0)) {
            int start = NR_FEATURES * i, end = start + NR_FEATURES;
            adjust_updates(company, updates, features, time, time + HORIZONS[i], start, end);
        }
    }
}

