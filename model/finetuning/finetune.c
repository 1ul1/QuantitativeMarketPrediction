#include "utils.h"

void finetune(
    const Company company,
    const Company market,
    Weights weights,
    double** features
) {
    for (int k = 0; k < EPOCHS; k += 1) {

        double alpha = ALPHA * exp((-1) * BETA * k);
            
        for (int time = 20; time < LAST + 1; time += 1) {
            
            double* updates = (double*)calloc(WEIGHTS->len_weights + WEIGHTS->len_bias, sizeof(double));

            // calculate gi for each wi and save it in updates
            process_one_company(&company, updates, time, features[time]);

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
}
