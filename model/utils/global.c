#include "utils.h"

Company* MARKET = NULL;
Company* COMPANY = NULL;
Weights* WEIGHTS = NULL;
Prediction* PREDICTION = NULL;
Companies* COMPANIES = NULL;
Companies* UNTRAINED_COMPANIES = NULL;

int NR_FEATURES = 72;

int NR_HORIZONS = 4;
int HORIZONS[4] = {1, 5, 10, 20};

int NR_COMPANIES = 0;
int NR_THREADS = 12;

// Only Finetuning overrides them
double ALPHA = 0.001;
double BETA = 0.2;
double LAMBDA = 0.00001;
int EPOCHS = 10;

double CLIP = 10;

int TODAY = 0;
int LAST = 0;

int get_nr_features() {
    return NR_FEATURES;
}

int get_nr_horizons() {
    return NR_HORIZONS;
}

