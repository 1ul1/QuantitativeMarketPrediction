"""
Handles the general training.
"""

from io_layer.data_types import *
import os
import sys
import json

MARKET: Company = None
COMPANIES: Companies = None
UNTRAINED_COMPANIES: Companies = None
WEIGHTS: Weights = None

def reset_weights():
    nr_features = lib.get_nr_features()
    nr_weights: int = nr_features * 4

    with open("./model_weights", "w") as file:
        for _ in range(nr_weights):
            file.write("0_")
        file.write("\n")
        for _ in range(4):
            file.write("0_")
        file.write("\n")
        for _ in range(nr_features):
            file.write("0_")
        file.write("\n")
        for _ in range(nr_features):
            file.write("0_")
        file.write("\n")

def train():
    global MARKET, COMPANIES, WEIGHTS
    
    files = os.listdir(".io_layer/training/stocks")
    
    COMPANIES = Companies(len(files), files)

    files = os.listdir(".io_layer/training/untrained_stocks")

    UNTRAINED_COMPANIES = Companies(len(files), files)

    with open(".io_layer/training/market/SPY.json", "r") as file:
        data = json.load(file)
        MARKET = Company(data["ticker"], data["count"], data["results"])

    with open(".io_layer/training/model_weights", "r") as file:
        weights, bias = file.readline(), file.readline()

        weights = [float(value) for value in weights.split("_")[:-1:]]
        bias = [float(value) for value in bias.split("_")[:-1:]]
        
        WEIGHTS = Weights(len(weights), weights, len(bias), bias)

    if not (COMPANIES and MARKET and WEIGHTS):
        print("IO failure")
        
    try:
        while True:
            lib.train(COMPANIES, UNTRAINED_COMPANIES, MARKET, WEIGHTS)
            WEIGHTS.static_save()
    except Exception as e:
        print(e)

