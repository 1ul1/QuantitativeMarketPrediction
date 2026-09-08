#include "utils.h"

void calculate_means_sds(double*** features) {
    for (int time = 20; time < MARKET->count; time += 1) {
        for (int i = 0; i < NR_COMPANIES; i += 1) {
            calculate_raw_features(features[time][i], &(COMPANIES->companies[i]), time);
            // Means
            for (int f = 0; f < NR_FEATURES; f += 1) {
                WEIGHTS->means[f] += features[time][i][f];
            }
        }
    }
    for (int f = 0; f < NR_FEATURES; f += 1) {
        WEIGHTS->means[f] /= ((MARKET->count - 20) * NR_COMPANIES);
    }
    // Standard_deviations
    for (int time = 20; time < MARKET->count; time += 1) {   
        for (int i = 0; i < NR_COMPANIES; i += 1) {
            for (int f = 0; f < NR_FEATURES; f += 1) {
                WEIGHTS->standard_deviations[f] += pow(
                    features[time][i][f] - WEIGHTS->means[f]
                    ,
                    2
                );
            }
        }
    }
    for (int f = 0; f < NR_FEATURES; f += 1) {
        WEIGHTS->standard_deviations[f] /= ((MARKET->count - 20) * NR_COMPANIES);
        WEIGHTS->standard_deviations[f] = sqrt(WEIGHTS->standard_deviations[f]);
    }

    // Center and Scale || FIX all features among the universe
    for (int time = 20; time < MARKET->count; time += 1) {   
        for (int i = 0; i < NR_COMPANIES; i += 1) {
            for (int f = 0; f < NR_FEATURES; f += 1) {
                features[time][i][f] = (WEIGHTS->standard_deviations[f] < 1e-12) ? 0.0
                    : (features[time][i][f] - WEIGHTS->means[f]) / WEIGHTS->standard_deviations[f];
            }
        }
    }
}

void calculate_untrained_features(double*** features) {
    for (int time = TRAINING_LOWER_BOUND; time < TRAINING_UPPER_BOUND; time += 1) {   
        for (int i = 0; i < UNTRAINED_COMPANIES->len_companies; i += 1) {
            calculate_raw_features(features[time][i], &(UNTRAINED_COMPANIES->companies[i]), time);
            for (int f = 0; f < NR_FEATURES; f += 1) {
                features[time][i][f] = (WEIGHTS->standard_deviations[f] < 1e-12) ? 0.0
                    : (features[time][i][f] - WEIGHTS->means[f]) / WEIGHTS->standard_deviations[f];
            }
        }
    }
}
