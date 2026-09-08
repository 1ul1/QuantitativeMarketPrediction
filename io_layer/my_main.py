import sys
from io_layer.model.model import *
from io_layer.training.train import * 
from io_layer.training.scrape_data import * 

if __name__ == "__main__":

    if len(sys.argv) == 1:
        reset_weights()
        request(sys.argv[1])
    
    match sys.argv[1]:
        case "train":
            train()
        case "reset":
            reset_weights()
        case "get_data":
            scrape()
        case _:
            pass
