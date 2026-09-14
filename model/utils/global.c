#include "utils.h"

Company* MARKET = NULL;
Company* COMPANY = NULL;
Weights* WEIGHTS = NULL;
Prediction* PREDICTION = NULL;
Companies* COMPANIES = NULL;
Companies* UNTRAINED_COMPANIES = NULL;

int NR_FEATURES = 72;

int NR_COMPANIES = 0;
int NR_THREADS = 12;

// These 2 must be UNIX TIME in milliseconds
// Data is from 2016 till present
// https://www.epochconverter.com/
double TRAINING_LOWER_BOUND_TIMESTAMP = 1546300801000; // 2019-01-01
double TRAINING_UPPER_BOUND_TIMESTAMP = 1672531201000; // 2023-01-01
// From them i calculate the indexes
int TRAINING_LOWER_BOUND = 0;
int TRAINING_UPPER_BOUND = 0;

// Only Finetuning overrides them
double ALPHA = 0.01;
double BETA = 0.01;
double LAMBDA = 0.000005;
int EPOCHS = 100;

int TODAY = 0;
int LAST = 0;

int get_nr_features() {
    return NR_FEATURES;
}

int get_nr_models() {
    return 4;
}

