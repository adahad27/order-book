import random
import numpy as np
import os

ITERATIONS = 10
TICKER = "APPL"
DIR = "data"
os.makedirs(DIR, exist_ok=True)

PATH = f"{DIR}/synthetic_data.csv"


PRICE_MEAN = 300
PRICE_STDDEV = 50


QUANTITY_LOW = 1
QUANTITY_HIGH = 250 

prices = np.random.normal(PRICE_MEAN, PRICE_STDDEV, ITERATIONS)
quantities = np.random.uniform(QUANTITY_LOW, QUANTITY_HIGH, ITERATIONS)



open(PATH, 'w').close()

for i in range(ITERATIONS):
    order_type_number = random.randint(0, 2)
    order_type = ""
    match order_type_number:
        case 0:
            order_type = "add"
        case 1:
            order_type = "cancel"
        case 2:
            order_type = "modify"
    
    side = "BID" if random.randint(0, 1) == 0 else "ASK"
    order_subtype = "MARKET" if random.randint(0, 1) else "LIMIT"

    price = round(prices[i], 2)
    quantity = int(quantities[i])

    
    with open(PATH, 'a', encoding="utf-8") as file:
        file.write(f"{order_type}, {TICKER}, user{i}, {side}, {order_subtype}, {price}, {quantity}\n")