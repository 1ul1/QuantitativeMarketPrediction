#include "utils.h"

void model(
    Companies companies,
    Company market,
    Weights weights,
    Prediction* prediction
) {
    NR_COMPANIES = companies.len_companies;
    COMPANIES = &companies;
    COMPANY = companies.companies;
    MARKET = &market;
    WEIGHTS = &weights;
    PREDICTION = prediction;
    TODAY = is_today(COMPANY);
    LAST = TODAY ? COMPANY->count - 2: COMPANY->count - 1;

    TRAINING_LOWER_BOUND = 1,
    TRAINING_UPPER_BOUND = -1;

    ALPHA = 0.00001;
    BETA = 0.01;
    LAMBDA = 0.000005;
    EPOCHS = 100;

    double** features = (double**)malloc(sizeof(double*) * MARKET->count);
    for (int time = 20; time < MARKET->count; time += 1) {
        features[time] = (double*)malloc(sizeof(double) * NR_FEATURES);
    }
    calculate_features(features);
    
    populate_error_metrics(features);

    finetune(*COMPANY, market, weights, features);

    populate(features[LAST]);

    for (int time = 20; time < MARKET->count; time += 1) {
        free(features[time]);
    }
    free(features);
}

void training(
    const Companies companies,
    const Companies untrained_companies,
    const Company market,
    Weights weights
) {
    MARKET = &market;
    WEIGHTS = &weights;
    NR_COMPANIES = companies.len_companies;
    COMPANIES = &companies;
    UNTRAINED_COMPANIES = &untrained_companies;
    
    TRAINING_LOWER_BOUND = MARKET->count / 6 * 2,
    TRAINING_UPPER_BOUND = MARKET->count / 6 * 3;

    LAST = MARKET->count - 1;
    
    train(companies, market, weights);
}
