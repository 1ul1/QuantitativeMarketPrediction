#include "utils.h"

void train(
    const Companies companies,
    const Company market,
    Weights weights
) {

    int iter = 0;

    double*** features = (double***)malloc(sizeof(double**) * (MARKET->count));
    double*** untrained_features = (double***)malloc(sizeof(double**) * (MARKET->count));
                
    for (int time = 20; time < MARKET->count; time += 1) {
        features[time] = (double**)malloc(sizeof(double*) * NR_COMPANIES);
        for (int i = 0; i < NR_COMPANIES; i += 1) {
            features[time][i] = (double*)malloc(sizeof(double) * NR_FEATURES);
            calculate_raw_features(features[time][i], &(COMPANIES->companies[i]), time);
        }
    }
        
    for (int time = TRAINING_LOWER_BOUND; time < TRAINING_UPPER_BOUND; time += 1) {
        untrained_features[time] = (double**)malloc(sizeof(double*) * UNTRAINED_COMPANIES->len_companies);
        for (int i = 0; i < UNTRAINED_COMPANIES->len_companies; i += 1) {
            untrained_features[time][i] = (double*)malloc(sizeof(double) * NR_FEATURES);
        }
    }
    calculate_means_sds(features); // Calculates Means and SDS too
    calculate_untrained_features(untrained_features);

    repeat:

    // Check Error
    double* ans1 = calloc(4, sizeof(double));
    error(ans1, features, 1);
    printf("\n");
    for (int i = 0; i < 4; i += 1) {
        printf("RMSE %.16f for Layer %d\n", ans1[i],i);
    }
        
    for (int time = 20; time < MARKET->count; time += 1) {

        if (TRAINING_LOWER_BOUND <= time && time < TRAINING_UPPER_BOUND) {continue;}
        
        for (int k = 0; k < EPOCHS; k += 1) {
            double* updates = (double*)calloc(WEIGHTS->len_weights + WEIGHTS->len_bias, sizeof(double));

            double alpha = ALPHA * exp((-1) * BETA * k);

            // calculate gi for each wi and save it in updates
            for (int i = 0; i < NR_COMPANIES; i += 1) {
                process_one_company(&(companies.companies[i]), updates, time, features[time][i]);
            }

            // update wi using -= ALPHA * gi
            for (int i = 0; i < WEIGHTS->len_weights; i += 1) {
                WEIGHTS->weights[i] -= alpha * updates[i];
            }
            for (int i = 0; i < WEIGHTS->len_bias; i += 1) {
                WEIGHTS->bias[i] -= alpha * updates[WEIGHTS->len_weights + i];
            }
            
            free(updates);
        }
    }

    double* ans2 = calloc(4, sizeof(double));
    error(ans2, features, 1);

    int nr = 0;
    
    for (int i = 0; i < 4; i += 1) {
        if (ans2[i] > ans1[i]) {
            nr += 1;
        }
    }

    // FREEING
    free(ans1);
    free(ans2);
    
    if (nr == 4 || iter == 20) {
        // True Skill on Trully unseen Companies over a unseed timeframe
        print_skill(untrained_features);
        
        if (nr == 4) {ALPHA /= 2;}
        for (int time = 20; time < MARKET->count; time += 1) {
            for (int i = 0; i < NR_COMPANIES; i += 1) {
                free(features[time][i]);
            }
            free(features[time]);
        }
        for (int time = TRAINING_LOWER_BOUND; time < TRAINING_UPPER_BOUND; time += 1) {
            for (int i = 0; i < UNTRAINED_COMPANIES->len_companies; i += 1) {
                free(untrained_features[time][i]);
            }
            free(untrained_features[time]);
        }
        free(features);
        free(untrained_features);
        
        return;
    }

    iter += 1;
    
    goto repeat; 
}
