#include "utils.h"

Company* MARKET = NULL;
Company* COMPANY = NULL;
Weights* WEIGHTS = NULL;
Prediction* PREDICTION = NULL;
Companies* COMPANIES = NULL;
Companies* UNTRAINED_COMPANIES = NULL;

int NR_FEATURES = 168;
int LOOKBACK = 500;

int NR_HORIZONS = 4;
int HORIZONS[4] = {5, 10, 15, 20};

int NR_COMPANIES = 0;
int NR_THREADS = 12;

// Only Finetuning overrides them
double ALPHA = 0.0001;
double BETA = 0.001;
double LAMBDA = 0.00001;
int EPOCHS = 10;

double CLIP = 100.0;

int TODAY = 0;
int LAST = 0;

double get_ALPHA() {
    return ALPHA;
}

int get_nr_features() {
    return NR_FEATURES;
}

int get_nr_horizons() {
    return NR_HORIZONS;
}

