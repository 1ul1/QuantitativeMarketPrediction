"""
Gets one ticker, calls upon the C code to get the <<Prediction>>
"""

from io_layer.data_types import *
import os
import sys
import ssl
import json
from datetime import date
from urllib.request import urlopen

API_KEY = os.environ.get("MASSIVE_KEY")
ALPACA_KEY = os.environ.get("ALPACA_KEY")
ALPACA_SECRET = os.environ.get("ALPACA_SECRET")

RESET_WEIGHTS = 1

def request(ticker) -> Prediction:

    ticker = ticker.upper()
    
    until: str = str(sys.argv[2]) if len(sys.argv) == 3 else str(date.today())

    if RESET_WEIGHTS == 0:
        weights = Weights(lib.get_nr_features() * lib.get_nr_models(), lib.get_nr_models())
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
        
    url = (
        'https://api.massive.com/v2/aggs/ticker/'
        + ticker +
        '/range/1/day/2023-06-23/'
        + until +
        '?adjusted=true&sort=asc&limit=1000&apiKey='
        + API_KEY
    )
    url_market = (
        'https://api.massive.com/v2/aggs/ticker/SPY/range/1/day/2023-06-23/'
        + until +
        '?adjusted=true&sort=asc&limit=1000&apiKey='
        + API_KEY
    )

    ctx = ssl.create_default_context()
    ctx.check_hostname = False
    ctx.verify_mode = ssl.CERT_NONE
    
    response = urlopen(url, context = ctx)
    response_market = urlopen(url_market, context = ctx)
    
    data = json.load(response)
    data_market = json.load(response_market)

    assert data["resultsCount"] == data_market["resultsCount"]

    company = Company(data["ticker"], data["count"], data["results"])
    companies = Companies(1, None, company)
    market = Company(data_market["ticker"], data_market["count"], data_market["results"])

    prediction: Prediction = Prediction(4)
    lib.model(companies, market, weights, ctypes.byref(prediction))
    prediction.print()

    return prediction
