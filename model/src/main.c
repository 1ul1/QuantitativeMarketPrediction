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

    ALPHA /= 2.5;
    BETA = 0.01;
    LAMBDA = 0.0000001;
    EPOCHS = 1;

    double*** features = (double***)malloc(sizeof(double**));
    *features = (double**)malloc(sizeof(double*) * MARKET->count);
    for (int time = 20; time < MARKET->count; time += 1) {
        (*features)[time] = (double*)malloc(sizeof(double) * NR_FEATURES);
    }
    calculate_features(*features);

    SINCE_TIMESTAMP = (
        COMPANY->samples[COMPANY->count - 1].t
        -
        1.0 * 
        (HORIZONS[NR_HORIZONS - 1] + 30)
        *
        24 * 3600 * 1000
    );

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
