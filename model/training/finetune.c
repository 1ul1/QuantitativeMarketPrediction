#include "utils.h"

void finetune(
    const Company company,
    const Company market,
    Weights weights,
    double** features
) {
    for (int k = 0; k < EPOCHS; k += 1) {

        double alpha = ALPHA * exp((-1) * BETA * k);

        double* updates = (double*)calloc(WEIGHTS->len_weights + WEIGHTS->len_bias, sizeof(double));
            
        for (int time = 20; time < LAST; time += 1) {

            if (train_rule(time)) {continue;}

            // calculate gi for each wi and save it in updates
            memset(updates, 0, sizeof(double) * (WEIGHTS->len_weights + WEIGHTS->len_bias));
            process_one_company(&company, updates, time, features[time]);

            // update wi using -= ALPHA * gi
            for (int i = 0; i < WEIGHTS->len_weights; i += 1) {
                WEIGHTS->weights[i] -= alpha * updates[i];
            }
            for (int i = 0; i < WEIGHTS->len_bias; i += 1) {
                WEIGHTS->bias[i] -= alpha * updates[WEIGHTS->len_weights + i];
            }
        }
        free(updates);
    }
}
