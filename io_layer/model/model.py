"""
Gets one ticker, calls upon the C code to get the <<Prediction>>
"""

from io_layer.data_types import *
import os
import sys
import ssl
import json
import requests
from datetime import date, timedelta
from urllib.request import urlopen

API_KEY = os.environ.get("MASSIVE_KEY")
ALPACA_KEY = os.environ.get("ALPACA_KEY")
ALPACA_SECRET = os.environ.get("ALPACA_SECRET")

RESET_WEIGHTS = 1

def request(ticker) -> Prediction:

    ticker = ticker.upper()

    if RESET_WEIGHTS == 0:
        weights = Weights(lib.get_nr_features() * lib.get_nr_horizons(), lib.get_nr_horizons())
    else:
        file = open("./model/model_weights", "r")
        weights_data, bias_data = file.readline(), file.readline()
        means_data, standard_deviations_data = file.readline(), file.readline()

        weights_data = [float(value) for value in weights_data.split("_")[:-1:]]
        bias_data = [float(value) for value in bias_data.split("_")[:-1:]]
        means_data = [float(value) for value in means_data.split("_")[:-1:]]
        standard_deviations_data = [float(value) for value in standard_deviations_data.split("_")[:-1:]]

        weights = Weights(
            len(weights_data), len(bias_data), weights_data, bias_data, means_data, standard_deviations_data
        )

    YESTERDAY = str((datetime.now() - timedelta(hours=7, days=1)).date())
    
    url = (
        f"https://data.alpaca.markets/v2/stocks/bars?symbols={ticker}&timeframe=1D&start="
        +
        f"2016-01-01&end={YESTERDAY}&limit=10000&adjustment=all&feed=sip&sort=asc"
    )
    headers = {
        "accept": "application/json",
        "APCA-API-KEY-ID": ALPACA_KEY,
        "APCA-API-SECRET-KEY": ALPACA_SECRET
    }

    url_market = (
        f"https://data.alpaca.markets/v2/stocks/bars?symbols=SPY&timeframe=1D&start="
        +
        f"2016-01-01&end={YESTERDAY}&limit=10000&adjustment=all&feed=sip&sort=asc"
    )
    headers_market = {
        "accept": "application/json",
        "APCA-API-KEY-ID": ALPACA_KEY,
        "APCA-API-SECRET-KEY": ALPACA_SECRET
    }
    
    response = requests.get(url, headers=headers)
    response_market = requests.get(url_market, headers=headers_market)
    print("response:", response.status_code)
    print("response_market:", response_market.status_code)
    assert response.status_code == 200 and response_market.status_code == 200
    
    data = response.json()
    data_market = response_market.json()

    assert len(data["bars"][ticker]) == len(data_market["bars"]["SPY"])
    assert data["bars"][ticker][0]["t"] == data_market["bars"]["SPY"][0]["t"]

    company = Company(ticker, len(data["bars"][ticker]), data["bars"][ticker])
    companies = Companies(1, None, company)
    market = Company("SPY", len(data_market["bars"]["SPY"]), data_market["bars"]["SPY"])

    prediction: Prediction = Prediction(lib.get_nr_horizons())
    lib.model(companies, market, weights, ctypes.byref(prediction))
    prediction.print()

    return prediction

