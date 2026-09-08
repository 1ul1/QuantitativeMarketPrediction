"""
Bridge to raw C code.
All such logic lives here, the other files are for parsin, reading, outputing the data.
"""

import json
import ctypes

class Sample(ctypes.Structure):
    _fields_ = [
        ("v", ctypes.c_double),
        ("vw", ctypes.c_double),
        ("o", ctypes.c_double),
        ("c", ctypes.c_double),
        ("h", ctypes.c_double),
        ("l", ctypes.c_double),
        ("t", ctypes.c_double),
        ("n", ctypes.c_double),
    ]

    def __init__(self, v = 0, vw = 0, o = 0, c = 0, h = 0, l = 0, t = 0, n = 0):
        super().__init__()
        try:
            self.v = v
            self.vw = vw
            self.o = o
            self.c = c
            self.h = h
            self.l = l
            self.t = int(1)
            self.n = n
            assert not any(field <= 0 for field in (v, vw, o, c, h, l, self.t, n))
            
        except Exception as e:
            print(e)
            print(f"BAD Samples \n{v} {vw} {o} {c} {h} {l} {t} {n}")
        


class Company(ctypes.Structure):
    _fields_ = [
        ("count", ctypes.c_int),
        ("samples", ctypes.POINTER(Sample)),
    ]

    def __init__(self, ticker: str = "@", count: int = 0, results: list[dict[str, float]] = None):
        super().__init__()
        
        self.ticker = ticker
        self.count = count
        self.samples = None if count == 0 else (Sample * count)()

        for i, row in enumerate(results):
            self.samples[i] = Sample(row["v"], row["vw"], row["o"], row["c"], row["h"], row["l"], row["t"], row["n"])


            
class Companies(ctypes.Structure):
    _fields_ = [
        ("len_companies", ctypes.c_int),
        ("companies", ctypes.POINTER(Company))
    ]

    def __init__(self, len_companies: int, files: list[str], company: Company = None, untrained: bool = False):
        super().__init__()
        
        self.len_companies = len_companies
        self.companies = None if len_companies == 0 else (Company * len_companies)()

        if company and not files:
            self.companies[0] = company
            return

        dir = "stocks" if not untrained else "untrained_stocks"

        for i, f in enumerate(files):
            with open(f"./io_layer/training/{dir}/{f}", "r") as file:
                
                f = f.split(".")[0]
                
                data = json.load(file)
                try:
                    #self.companies[i] = Company(data["ticker"], data["count"], data["results"])
                    data = data["bars"]
                    self.companies[i] = Company(f, len(data[f]), data[f])
                except Exception as e:
                    print(e)
                    # print(data)
                    # print(f)



class Weights(ctypes.Structure):
    _fields_ = [
        ("len_weights", ctypes.c_int),
        ("weights", ctypes.POINTER(ctypes.c_double)),
        ("len_bias", ctypes.c_int),
        ("bias", ctypes.POINTER(ctypes.c_double)),
        ("means", ctypes.POINTER(ctypes.c_double)),
        ("standard_deviations", ctypes.POINTER(ctypes.c_double)),
    ]

    def __init__(self,
        len_weights = 0,
        len_bias: int = 0, 
        weights: list[float] = None,
        bias: list[float] = None,
        means: list[float] = None,
        standard_deviations: list[float] = None
    ):
        super().__init__()
        
        assert (len_weights > 0 and len_bias > 0)
        self.len_weights = len_weights
        self.weights = (ctypes.c_double * len_weights)()
        self.len_bias = len_bias
        self.bias = (ctypes.c_double * len_bias)()
        self.means = (ctypes.c_double * (len_weights // len_bias))()
        self.standard_deviations = (ctypes.c_double * (len_weights // len_bias))()

        for i, r in enumerate(weights):
            self.weights[i] = r
        for i, r in enumerate(bias):
            self.bias[i] = r

        if bias and means:
            
            for i, r in enumerate(means):
                self.means[i] = r
            for i, r in enumerate(standard_deviations):
                self.standard_deviations[i] = r

        else:
            
            for i in range(len_weights // len_bias):
                self.means[i] = 0
            for i in range(len_weights // len_bias):
                self.standard_deviations[i] = 0

    def print(self):
        for i in range(self.len_weights):
            print(GREEN + f"W {i} - " + str(self.weights[i]) + RESET)
        for i in range(self.len_bias):
            print(YELLOW +  f"B {i} - " + str(self.bias[i]) + RESET)

    def static_save(self):
        with open("./io_layer/training/model_weights", "w") as file:
            file.write("_".join(format(self.weights[i], ".17g") for i in range(self.len_weights)))
            file.write("_\n")
            file.write("_".join(format(self.bias[i], ".17g") for i in range(self.len_bias)))
            file.write("_\n")
            file.write("_".join(format(self.means[i], ".17g") for i in range(self.len_weights // self.len_bias)))
            file.write("_\n")
            file.write("_".join(format(self.standard_deviations[i], ".17g") for i in range(self.len_weights // self.len_bias)))
            file.write("_\n")
            print("Weights Saved!\n")

            for i in range(self.len_weights // self.len_bias):
                self.means[i] = 0
                self.standard_deviations[i] = 0


            
class Day(ctypes.Structure):
    _fields_ = [
        ("bias", ctypes.c_double),
        ("sd", ctypes.c_double),
        ("expected_return", ctypes.c_double),
        ("expected_price", ctypes.c_double),
        ("starting_price", ctypes.c_double),
        ("forecast_strength", ctypes.c_double),
    ]

    def __init__(self, bias = 0, sd = 0, expected_return = 0, expected_price = 0, starting_price = 0, forecast_strength = 0):
        super().__init__()

        self.bias = bias
        self.sd = sd
        self.expected_return = expected_return
        self.expected_price = expected_price
        self.starting_price = starting_price
        self.forecast_strength = forecast_strength



class Prediction(ctypes.Structure):
    _fields_ = [
        ("today", ctypes.c_int),
        ("len_days", ctypes.c_int),
        ("days", ctypes.POINTER(Day)),
    ]

    def __init__(self, len_days):
        super().__init__()

        self.today = 0
        self.len_days = len_days
        self.days = (Day * len_days)()

    def print(self):
        print(f"Today: {self.today}\n")

        for i in range(self.len_days):
            print(f"Prediction Layer {i + 1}\n")
            print(
                f"bias {self.days[i].bias}\nsd {self.days[i].sd}\nexpected_return {self.days[i].expected_return}"
                +
                f"\nexpected_price {self.days[i].expected_price}\nstarting_price {self.days[i].starting_price}\nforecast_strength {self.days[i].forecast_strength}\n"
            )

lib = ctypes.CDLL("./model/build/libmodel.dylib")

lib.get_nr_features.argtypes = []
lib.get_nr_features.restype = ctypes.c_int

lib.get_nr_models.argtypes = []
lib.get_nr_models.restype = ctypes.c_int

lib.model.argtypes = [Companies, Company, Weights, ctypes.POINTER(Prediction)]
lib.model.restype = None

lib.training.argtypes = [Companies, Companies, Company, Weights]
lib.training.restype = None
