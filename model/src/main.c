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
    LAST = TODAY ? COMPANY->count - 1: COMPANY->count;

    ALPHA = 0.0005;
    BETA = 0.3;
    LAMBDA = 0.0001;
    EPOCHS = 50;

    double*** features = (double***)malloc(sizeof(double**));
    *features = (double**)malloc(sizeof(double*) * MARKET->count);
    for (int time = 20; time < MARKET->count; time += 1) {
        (*features)[time] = (double*)malloc(sizeof(double) * NR_FEATURES);
    }
    calculate_features(*features);

    finetune(*COMPANY, market, weights, *features);

    populate_error_metrics(*features);

    populate((*features)[LAST - 1]);

    for (int time = 20; time < MARKET->count; time += 1) {
        free((*features)[time]);
    }
    free(*features);
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
    
    LAST = MARKET->count;
    
    train(companies, market, weights);
}
