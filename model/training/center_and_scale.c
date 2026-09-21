#include "utils.h"

void calculate_means_sds(double*** features) {
    int range = 0;
    // Means
    for (int time = 20; time < MARKET->count; time += 1) {
        if (train_rule(time)) {continue;}
        for (int i = 0; i < NR_COMPANIES; i += 1) {
            for (int f = 0; f < NR_FEATURES; f += 1) {
                WEIGHTS->means[f] += features[time][i][f];
            }
        }
        range += 1;
    }
    for (int f = 0; f < NR_FEATURES; f += 1) {
        WEIGHTS->means[f] /= (range * NR_COMPANIES);
    }
    // Standard_deviations
    for (int time = 20; time < MARKET->count; time += 1) {
        if (train_rule(time)) {continue;} 
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
        WEIGHTS->standard_deviations[f] /= (range * NR_COMPANIES);
        WEIGHTS->standard_deviations[f] = sqrt(WEIGHTS->standard_deviations[f]);
    }

    // Center and Scale || FIX all features among the universe
    #pragma omp parallel for num_threads(NR_THREADS)
    
    for (int time = 20; time < MARKET->count; time += 1) {   
        for (int i = 0; i < NR_COMPANIES; i += 1) {
            for (int f = 0; f < NR_FEATURES; f += 1) {
                features[time][i][f] = max(
                    WEIGHTS->means[f] - WEIGHTS->standard_deviations[f] * CLIP,
                    min(WEIGHTS->means[f] + WEIGHTS->standard_deviations[f] * CLIP, features[time][i][f])
                );
                features[time][i][f] = (WEIGHTS->standard_deviations[f] < 1e-12) ? 0.0
                    : (features[time][i][f] - WEIGHTS->means[f]) / WEIGHTS->standard_deviations[f];
            }
        }
    }
}

void calculate_untrained_features(double*** features) {
    #pragma omp parallel for num_threads(NR_THREADS)
    
    for (int time = 20; time < MARKET->count; time += 1) {
        if (!train_rule(time)) {continue;}
        for (int i = 0; i < UNTRAINED_COMPANIES->len_companies; i += 1) {
            calculate_raw_features(features[time][i], &(UNTRAINED_COMPANIES->companies[i]), time);
            for (int f = 0; f < NR_FEATURES; f += 1) {
                features[time][i][f] = max(
                    WEIGHTS->means[f] - WEIGHTS->standard_deviations[f] * CLIP,
                    min(WEIGHTS->means[f] + WEIGHTS->standard_deviations[f] * CLIP, features[time][i][f])
                );
                features[time][i][f] = (WEIGHTS->standard_deviations[f] < 1e-12) ? 0.0
                    : (features[time][i][f] - WEIGHTS->means[f]) / WEIGHTS->standard_deviations[f];
            }
        }
    }
}
