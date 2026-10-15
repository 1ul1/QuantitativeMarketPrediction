

# [![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.22927824.svg)](https://doi.org/10.5281/zenodo.22927824) <br> Multi-Horizon Stock Log Return Forecasting<br> in Raw C

Built and self-hosted a website around the ML model | nginx, Python, Flutter.<br>
[![Website](https://img.shields.io/badge/Website-%E2%80%8B%20ByeBility.com-222?labelColor=ddd)](https://byebility.com)

<p align="center">
  <img src="plots/website/output.gif" alt="Demo" width="600">
  <br>
  <a href="https://byebility.com">ByeBility.com</a>
</p>

### C (prediction model) & Python (I/O)

- #### Ridge regression written directly in raw C, no external ML framework or numerical library used anywhere in the loop.

- #### Python side of repo handles only stock data scraping, parsing and preprocessing, and uses *ctypes* to pass it to the C model.

## Overview

- Data is collected and validated by a self-written scraper
- *openmp* is used to parallelize feature computation & training
- All features are centered, scaled and clipped
- Configurable independent prediction horizons
- Universe and forecasting are restricted to companies with a complete history since 2016
- Weights are pretrained across a company universe, then fine-tuned and calibrated per ticker before each forecast
- Certain time windows are excluded from all training and kept for calibration and skill measurement

## Output

For a given ticker and date, each configured horizon returns:

- **Starting price**: same for all horizons
- **Expected return**: the fine-tuned prediction, bias corrected
- **Expected price**: starting price × exp(expected return)
- **Residual bias**: the ticker's average historical miss for this horizon, converted to dollars
- **Residual standard deviation**: the spread of residuals after the bias correction, converted to dollars
- **Forecast strength**: How strong this forecast is relative to the model’s typical error for this ticker, expressed as a percentage of residual sd.

*Note: the standard deviation is computed only on the timeframes excluded from training & finetuning.*

## Results <br> RMSE + IC

### Chronological holdout Evaluation

<p align="center">
  <img src="./plots/model_results.png" alt="Model Results" width="600">
  <br>
  <sub>Each series is divided by its own largest absolute value.</sub>
  <br>
  <sub>RMSE improvement relative to a 0 log return baseline</sub>
  <br>
  <sub><em>TS</em> = time-series IC &nbsp;|&nbsp; <em>CS</em> = cross-sectional IC</sub>
</p>

***Cross-Sectional IC peaks at day 15 with +0.0287***

The model is trained only on data from 2016 through 2024, inclusive, on ~900 companies, then scored from 2025 till today on ~100 companies that were held-out during training.

Therefore, all metrics are computed on companies and time windows never seen during training with no lookahead, simulating real live forecasting conditions.

*Note*: Each horizon is an independent model with its own weights, they only share the same features for a given company at a given time.

***Observations***: 
- Features keep fine detail only for the last 20 days, and the cross-sectional IC results, that ignore the market part of the forecast, follow that window exactly. Close future is mostly noise and past that window the features carry too little relevant information, in between is where it fits.
- The early peak is the stable result, everything past roughly day 25 is window dependent.

<br>

*Notations*: $\hat r_{i,t}$ is the predicted log return for company $i$ at day $t$, and $r_{i,t}=\ln\big(C_{i,t+h}/C_{i,t}\big)$ is the realized one.

### RMSE Improvement

Performance is measured against a **zero log return baseline** (predicting no change):

$$
\text{RMSE Improvement}=\left(1-\frac{\text{RMSE}_{\text{model}}}{\text{RMSE}_{\text{baseline}}}\right)\times 100
$$

### IC - Information Coefficient

1) #### ***TS - time-series IC*** 

Per company, across time: does this company's realized return move with what the model predicted for it? 

*Pearson correlation* over all valid days of one company, then averaged across companies:

$$
TS_i=\frac{\text{cov}\big(\hat r_{i,t},r_{i,t}\big)}{\sigma_{\hat r}\sigma_r}
$$

2) #### ***CS - cross-sectional IC***

Per day, across companies: on a given day, does the model rank companies correctly against each other?

*Pearson correlation* over all companies on one day, then averaged across days:

$$
CS_t=\frac{\text{cov}\big(\hat r_{i,t},r_{i,t}\big)}{\sigma_{\hat r}\sigma_r}
$$

| Horizon |    RMSE Improvement |   Time-Series IC |   Cross-Sectional IC |
| ------: | ------: | ------: | ------: |
|   1 day | -0.054% | +0.0253 | +0.0091 |
|  2 days | -0.007% | +0.0529 | +0.0104 |
|  3 days | +0.023% | +0.0605 | +0.0137 |
|  5 days | +0.096% | +0.0774 | +0.0211 |
| 10 days | +0.211% | +0.1070 | +0.0272 |
| 15 days | +0.150% | +0.1142 | +0.0287 |
| 20 days | +0.292% | +0.1325 | +0.0238 |
| 25 days | +0.321% | +0.1400 | +0.0165 |
| 30 days | +0.481% | +0.1579 | +0.0106 |
| 35 days | +0.497% | +0.1549 | +0.0089 |
| 50 days | +0.829% | +0.1706 | +0.0163 |
| 75 days | +0.366% | +0.1436 | -0.0052 |

*Positive RMSE values indicate lower RMSE than the baseline.*

***Observations***:
- All effects are small in absolute terms, but my baseline is already very hard to beat.
- The time-series IC runs larger than the cross-sectional IC through the first 20 days. The time-series keeps the market component, while cross-sectional IC removes it by default. The gap between the two is exactly that market component.
- Time-series IC and RMSE relative gain keep increasing far past the detailed window because both include the market part of the forecast.

***Additional Note***:

- A second holdout starting 2026-01 reaches a higher cross-sectional IC than the one reported above, +0.040 at day 14 against +0.029 at day 15 here. I do not headline it because that window has such a small holdout.
- Comparing the two runs horizon by horizon is more informative than either one alone:

    - The cross-sectional IC curves correlate 0.92 across the 75 horizons;
    - The time-series IC curves correlate -0.09 and the RMSE improvement curves correlate -0.64;

   The less market a metric contains, the better it reproduces on a different window.



## Features & Mathematics

Notations: daily log return $r_s=\ln(C_s/C_{s-1})$, lookback window $L\in\{5,10,20\}$, $S$ = stock, $M$ = market.

- **Momentum**: log return

  $$\ln\frac{C_t}{C_{t-L}}$$

- **Relative volume**: today's volume vs its average

  $$\ln\frac{V_t}{\bar V_L}$$

- **Volatility**: standard deviation of daily log returns

  $$\sqrt{\frac{1}{L}\sum\big(r_i-\bar r\big)^2}$$

- **Dispersion**: how spread out the price path is around its own mean, divided by that mean

  $$\frac{1}{\bar C}\sqrt{\frac{1}{L}\sum\big(C_i-\bar C\big)^2}$$

- **Stability**: how much returns fluctuate, the RMS of the change in returns

  $$\sqrt{\frac{1}{L-1}\sum\big(r_i-r_{i-1}\big)^2}$$

- **Persistence**: the lag-1 Pearson correlation

  $$\frac{\frac{1}{L-1}\sum\big(r_i-\bar r_X\big)\big(r_{i+1}-\bar r_Y\big)}{\sigma_X\,\sigma_Y}$$

  $X$ = first $L-1$ returns of the window, $Y$ = last $L-1$.

- **Gap, intraday move, range, closing strength**: same day price action

  $$\ln\frac{O_t}{C_{t-1}}\qquad \ln\frac{C_t}{O_t}\qquad \ln\frac{\mathrm{Hi}_t}{\mathrm{Lo}_t}\qquad \frac{C_t-\mathrm{Lo}_t}{\mathrm{Hi}_t-\mathrm{Lo}_t}$$

- **Relative VWAP, relative transaction count**: like relative volume

  $$\ln\frac{VW_t}{\overline{VW}_L}\qquad \ln\frac{N_t}{\bar N_L}$$

- **Average trade size slope**: today's dollar size per trade against the window's

  $$\ln\frac{VW_t\,V_t/N_t}{\sum VW\,V\big/\sum N}$$

- **Relative volatility** (stock vs market): stock returns dispersed around the *market's* mean return

  $$\sqrt{\frac{1}{L}\sum\big(r_S-\bar r_M\big)^2}$$

- **Market correlation** (stock vs. market): Pearson correlation between stock and market returns

  $$\frac{\frac{1}{L}\sum\big(r_S-\bar r_S\big)\big(r_M-\bar r_M\big)}{\sigma_S\,\sigma_M}$$

- **Market beta**: stock sensitivity to market returns, fit on the $L-1$ returns ending at $t-1$, so today stays out of the fit

  $$\frac{\frac{1}{L-1}\sum\big(r_S-\bar r_S\big)\big(r_M-\bar r_M\big)}{\sigma_M^2}$$

- **Residual return**: stock return unexplained by the market, over the same window as $\beta$

  $$r_{S,t}-\alpha-\beta\,r_{M,t}\qquad \text{with}\qquad \alpha=\bar r_S-\beta\,\bar r_M$$

- **Centering, scaling and clipping**: uses the training set mean and standard deviation:

$$z=\frac{x-\bar x}{\sigma_x}$$

, where x is clipped if needed

$$x=\max\!\Big(\bar x-c\sigma_x,\min\big(\bar x+c\sigma_x,x\big)\Big)$$

## Raw Features' Correlations

How the 72 raw features correlate with future stock log returns across 1-25 day horizons, computed over 10 years of daily data using 896 companies.

### Cross-sectional Pearson (per day, across companies)

<p align="center">
  <img src="plots/mean_cross_sectional_pearson.png" alt="Demo" width="600">
</p>

### Time-series Pearson (per company, across time)

<p align="center">
  <img src="plots/mean_time_series_pearson.png" alt="Demo" width="600">
</p>

<a href="plots/features.png"><u>Detailed results here.</u></a>

## Model

Each horizon is an independent ridge regression on log-returns. They are defined in the C part of the code:
```
int NR_HORIZONS = 4;
int HORIZONS[4] = {1, 5, 10, 20};
```
The Python side reads this definition and creates the appropriate *model_weights* file, while the C side trains every horizon listed.

by default, for 72 detailed short horizon features per sample:

| Horizon | Look-ahead | Weight slice | Bias index |
|---|---|---|---|
| 1 day  | `t + 1`  | `weights[0:72]`    | `bias[0]` |
| 5 day  | `t + 5`  | `weights[72:144]`  | `bias[1]` |
| 10 day | `t + 10` | `weights[144:216]` | `bias[2]` |
| 20 day | `t + 20` | `weights[216:288]` | `bias[3]` |

All parameters go through both training stages defined [here](./README_TRAINING.md): first fit across the whole universe, then finetune to one company at a time.

## How to run

### Requirements: 

- Python I/O side uses *requests* to pull live stock data
    
      pip install requests
  
- MacOS uses Clang instead of standard GCC, therefore the Makefile requires OpenMP support for building the parallelized version on MacOS, otherwise juse use *make all_serial*. I copied the OpenMP binaries from a Conda env, therefore I made the Makefile to expect actual binaries, using *homebrew* or *macports* will not work without modifying it.

- The Python side needs [Alpaca](https://docs.alpaca.markets/us/reference/stockbars) credentials to pull data, accesible through the environment as ALPACA_KEY & ALPACA_SECRET.
      
      ALPACA_KEY = os.environ.get("ALPACA_KEY")
      ALPACA_SECRET = os.environ.get("ALPACA_SECRET")

### Full Setup

      git clone https://github.com/1ul1/QuantitativeMarketPrediction
      cd QuantitativeMarketPrediction
      mkdir ./io_layer/training/market; mkdir ./io_layer/training/stocks; mkdir ./io_layer/training/untrained_stocks
      rm -rf ./io_layer/training/market/*; rm -rf ./io_layer/training/stocks/*; rm -rf ./io_layer/training/untrained_stocks/*
      python3 -m io_layer.my_main get_data (download training universe)
      cd model; make; cd ..; python3 -m io_layer.my_main train
      cp io_layer/training/model_weights ./model/
      python3 -m io_layer.my_main AAPL

If everything is set up correctly, a forecast for Apple's stock should be displayed.

*Note*: change make to make all_linux or make all_serial if needed. Implicit *all* rule is for Macos, linked against local OpenMP binaries.

### Predict a ticker
      python3 -m io_layer.my_main {ticker} 
                                  e.g python3 -m io_layer.my_main AAPL
                                  (you can use any US equity ticker)
### Train

      python3 -m io_layer.my_main train

,when done

      cp io_layer/training/model_weights ./model/

- To get a training universe to train on:

      mkdir ./io_layer/training/market; mkdir ./io_layer/training/stocks; mkdir ./io_layer/training/untrained_stocks
      rm -rf ./io_layer/training/market/*; rm -rf ./io_layer/training/stocks/*; rm -rf ./io_layer/training/untrained_stocks/*
      python3 -m io_layer.my_main get_data

### Replicate my results (RMSE + IC)

The Python side *get_data* module randomly selects 1 in 10 pulled stocks to be held-out during training and used for verification only. Therefore, to replicate my results you need to force these 3 dirs at *./io_layer/training/* to match exactly this structure:
[ls ./io_layer/training/market](./plots/replicate_results/market) & [ls ./io_layer/training/stocks](./plots/replicate_results/stocks) & [ls ./io_layer/training/untrained_stocks](./plots/replicate_results/untrained_stocks).

Additionally, the python scraper at [./io_layer/training/scrape_data.py](./io_layer/training/scrape_data.py), must only pull data from 2016-01-01 to 2026-09-14, so hardcode *YESTERDAY* to be 2026-09-14.

### Project Layout

```
QuantitativeMarketPrediction
_
├── LICENSE
├── README.md
├── io_layer -------------------- Python side: gathers and processes stock data
│   ├── __init__.py
│   ├── data_types.py
│   ├── my_main.py
│   ├── model
│   │   ├── __init__.py
│   │   └── model.py
│   └── training
│       ├── __init__.py
│       ├── model_weights
│       ├── scrape_data.py
│       ├── train.py
│       ├── market -------------- Training universes directories
│       │   └── empty
│       ├── stocks
│       │   └── empty
│       └── untrained_stocks
│           └── empty
├── model ----------------------- Raw C implementation of the actual model
│   ├── Makefile
│   ├── model_weights
│   ├── build
│   │   └── empty
│   ├── features ---------------- Turns raw stock data into numerical signals
│   │   ├── features.c
│   │   └── features_helper.c
│   ├── prediction
│   │   └── predict.c
│   ├── src
│   │   └── main.c
│   ├── training ---------------- Contains the math for fitting the model
│   │   ├── calculate_gradients.c
│   │   ├── center_and_scale.c
│   │   ├── finetune.c
│   │   └── train.c
│   └── utils ------------------- Shared code for computing predictions and errors
│       ├── utils.h
│       ├── utils.c
│       ├── global.c
│       ├── train_rules.c
│       └── time.c
├── plots
│   ├── *.png
│   └── stock_universe_plots
│       └─ *.png
│
```
