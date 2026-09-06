import random
import numpy as np
import os

"""
NOTE: If you change the value of ITERATIONS, and you are using it in conjunction
with test_server.py, make sure to change the value of ITERATIONS in that file as
well. In the future all of this will be moved to a config file so that only one
location needs to be modifed.
"""
ITERATIONS = 100
TICKER = "APPL"
DIR = "data"
os.makedirs(DIR, exist_ok=True)

PATH = f"{DIR}/synthetic_data.csv"


PRICE_MEAN = 300
PRICE_STDDEV = 30

ADD_PERCENTAGE = 60
CANCEL_PERCENTAGE = 20
MODIFY_PERCENTAGE = 100 - ADD_PERCENTAGE - CANCEL_PERCENTAGE


QUANTITY_LOW = 1
QUANTITY_HIGH = 250 

prices = np.random.normal(PRICE_MEAN, PRICE_STDDEV, ITERATIONS)
quantities = np.random.uniform(QUANTITY_LOW, QUANTITY_HIGH, ITERATIONS)



open(PATH, 'w').close()

for i in range(ITERATIONS):
    order_type_number = random.randint(0, 99)

    
    side = "BID" if random.randint(0, 1) == 0 else "ASK"
    order_subtype = "MARKET" if random.randint(0, 1) else "LIMIT"

    price = round(prices[i], 2)
    quantity = int(quantities[i])

    
    with open(PATH, 'a', encoding="utf-8") as file:
        if (order_type_number < ADD_PERCENTAGE):
            file.write(f"add, {TICKER}, user{i}, {side}, {order_subtype}, {price}, {quantity}\n")
        elif (order_type_number < ADD_PERCENTAGE + CANCEL_PERCENTAGE):
            order_id = random.randint(0, i)
            file.write(f"cancel, {TICKER}, user{i}, {order_id}\n")
        else:
            order_id = random.randint(0, i)
            file.write(f"modify, {TICKER}, user{i}, {order_id}, {side}, {order_subtype}, {price}, {quantity}\n")