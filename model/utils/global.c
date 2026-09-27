#include "utils.h"

Company* MARKET = NULL;
Company* COMPANY = NULL;
Weights* WEIGHTS = NULL;
Prediction* PREDICTION = NULL;
Companies* COMPANIES = NULL;
Companies* UNTRAINED_COMPANIES = NULL;

int NR_FEATURES = 72;

int NR_HORIZONS = 15;
int HORIZONS[15] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, 30, 50, 75};

int NR_COMPANIES = 0;
int NR_THREADS = 12;

// Only Finetuning overrides them
double ALPHA = 0.0001;
double BETA = 0.5;
double LAMBDA = 0.00000000001;
int EPOCHS = 1;

double CLIP = 10;

int TODAY = 0;
int LAST = 0;

int get_nr_features() {
    return NR_FEATURES;
}

int get_nr_horizons() {
    return NR_HORIZONS;
}

