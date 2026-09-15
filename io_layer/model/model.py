"""
Gets one ticker, calls upon the C code to get the <<Prediction>>
"""

from io_layer.data_types import *
import os
import sys
import ssl
import json
from datetime import date, timedelta
from urllib.request import urlopen

API_KEY = os.environ.get("MASSIVE_KEY")
ALPACA_KEY = os.environ.get("ALPACA_KEY")
ALPACA_SECRET = os.environ.get("ALPACA_SECRET")

RESET_WEIGHTS = 1

def request(ticker) -> Prediction:

    ticker = ticker.upper()

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

    YESTERDAY = str(date.today() - timedelta(days=1))
    
    url = f"""
curl --request GET \
    --url 'https://data.alpaca.markets/v2/stocks/bars?symbols={ticker}&timeframe=1D&start=2016-01-01&end=\
{YESTERDAY}&limit=10000&adjustment=all&feed=sip&sort=asc' \
    --header 'APCA-API-KEY-ID: {ALPACA_KEY}' \
    --header 'APCA-API-SECRET-KEY: {ALPACA_SECRET}' \
    --header 'accept: application/json' \
"""
    url_market = f"""
curl --request GET \
    --url 'https://data.alpaca.markets/v2/stocks/bars?symbols=SPY&timeframe=1D&start=2016-01-01&end=\
{YESTERDAY}&limit=10000&adjustment=all&feed=sip&sort=asc' \
    --header 'APCA-API-KEY-ID: {ALPACA_KEY}' \
    --header 'APCA-API-SECRET-KEY: {ALPACA_SECRET}' \
    --header 'accept: application/json' \
    > {saved_file}
"""

    ctx = ssl.create_default_context()
    ctx.check_hostname = False
    ctx.verify_mode = ssl.CERT_NONE
    
    response = urlopen(url, context = ctx)
    response_market = urlopen(url_market, context = ctx)
    
    data = json.load(response)
    data_market = json.load(response_market)

    assert len(data["bars"][ticker]) == len(data_market["bars"]["SPY"])
    assert data["bars"][ticker][0]["t"] == data_market["bars"]["SPY"][0]["t"]

    company = Company(ticker, len(data["bars"][ticker]), data["bars"][ticker])
    companies = Companies(1, None, company)
    market = Company("SPY", len(data_market["bars"]["SPY"]), data_market["bars"]["SPY"])

    prediction: Prediction = Prediction(4)
    lib.model(companies, market, weights, ctypes.byref(prediction))
    prediction.print()

    return prediction

