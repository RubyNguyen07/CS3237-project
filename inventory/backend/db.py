""" db.py file """
""" Database API """
import json
import numpy as np
import pandas as pd
import psycopg2
from io import StringIO

def db_engine():
    config = json.load(open("config.json"))
    host = config["connection"]["host"]
    port = config["connection"]["port"]
    user = config["connection"]["user"]
    # password should be hidden in production setting
    # do not store it in config.json
    password = config["connection"]["password"]
    db = config["connection"]["db"]

    return "user='{}' password='{}' host='{}' port='{}' dbname='{}'".format(
        user, password, host, port, db
    )

def sql_to_df(sql_query):
    try:
        conn = psycopg2.connect(db_engine())
        cur = conn.cursor()
        cur.execute(sql_query)
        df = pd.DataFrame(cur.fetchall(), columns=[elt[0] for elt in cur.description])
        cur.close()
        return df
    except Exception as e:
        print("Problems:", str(e))

    return None


# def get_train_data():
#     config = json.load(open("config.json"))
#     table = config["automl"]["table"]
#     features = config["automl"]["features"]
#     target = config["automl"]["target"]
#     get_data_sql = f"select {','.join(features+[target])} from {table} where {target} != ''"
#     df = sql_to_df(get_data_sql)
#     if df is None:
#         return None, None
#     return df[features], df[target]
   
   
# def get_live_data():
#     config = json.load(open("config.json"))
#     table = config["automl"]["table"]
#     features = config["automl"]["features"]
#     target = config["automl"]["target"]
#     predicted = config["automl"]["predicted"]
#     id_column = config["automl"]["id"]
#     get_data_sql = f"select {','.join(features + [id_column])} from {table} where {target} = '' and {predicted} = ''"
#     df = sql_to_df(get_data_sql)
#     if df is None:
#         return None, None
#     return df[features], df[id_column]


# def get_predictions():
#     config = json.load(open("config.json"))
#     table = config["automl"]["table"]
#     target = config["automl"]["target"]
#     predicted = config["automl"]["predicted"]
#     id_column = config["automl"]["id"]
#     get_data_sql = f"select {','.join([predicted] + [id_column])} from {table} where {target} = ''"
#     df = sql_to_df(get_data_sql)
#     if df is None:
#         return None
#     df.index = df[id_column]
#     return df
   
# def insert_predictions(predictions, ids):
#     config = json.load(open("config.json"))
#     table = config["automl"]["table"]
#     predicted = config["automl"]["predicted"]
#     id_column = config["automl"]["id"]

#     try:
#         conn = psycopg2.connect(db_engine())
#         cur = conn.cursor()
#         tuples = list(zip(predictions, ids))
#         sql_query = f"update {table} set {predicted} = %s where {id_column} = %s"
#         cur.executemany(sql_query, tuples)
#         conn.commit()
#     except Exception as e:
#         print("Problems:", str(e))

def get_data(startOfWeek, conn): 
    try: 
        with conn:
            with conn.cursor() as cur:
                prev_week_1_query = f"select chickenOrder, fishOrder, saladOrder from inventory where startOfWeek = CAST(%(startOfWeek)s as DATE) - 7;"
                cur.execute(prev_week_1_query, {
                    "startOfWeek": startOfWeek
                })
                prev_week_1_order = cur.fetchone()
                if not prev_week_1_order: 
                    prev_week_1_order = (0, 0, 0)
                prev_week_2_query = f"select chickenOrder, fishOrder, saladOrder from inventory where startOfWeek = CAST(%(startOfWeek)s as DATE) - 14;"
                cur.execute(prev_week_2_query, {
                    "startOfWeek": startOfWeek
                })
                prev_week_2_order = cur.fetchone()
                if not prev_week_2_order: 
                    prev_week_2_order = (0, 0, 0)

                return {
                    "prev_week_1_co": prev_week_1_order[0],
                    "prev_week_1_fo": prev_week_1_order[1],
                    "prev_week_1_so": prev_week_1_order[2],
                    "prev_week_2_co": prev_week_2_order[0],
                    "prev_week_2_fo": prev_week_2_order[1],
                    "prev_week_2_so": prev_week_2_order[2],
                }
                
    except Exception as e:
        print("Problems:", str(e))
        
def get_all_orders(conn): 
    try: 
        with conn:
            with conn.cursor() as cur:
                all_orders_query = f"select id, startOfWeek, chickenOrder, fishOrder, saladOrder from inventory ORDER BY startOfWeek DESC;"
                cur.execute(all_orders_query)
                all_orders = cur.fetchall()
                return all_orders
    
    except Exception as e: 
        print("Problems:", str(e))
        
       
def update_order(order, conn):
    # date format: 1999-01-08	
    try:
        with conn:
            with conn.cursor() as cur:
                find_query = "select * from inventory where startOfWeek = CAST(%(startOfWeek)s as DATE);"
                cur.execute(find_query, {
                    "startOfWeek": order['startOfWeek']
                })
                existingOrder = cur.fetchone()
                sql_query = ""
                if not existingOrder or len(existingOrder) == 0:
                    max_id_query = "select coalesce(max(id), -1) + 1 from inventory"
                    cur.execute(max_id_query)
                    max_id = cur.fetchone()
                    order["id"] = max_id[0]
                    sql_query = f"insert into inventory (id, startOfWeek, chickenOrder, fishOrder, saladOrder) values (%(id)s, %(startOfWeek)s, %(chickenOrder)s, %(fishOrder)s, %(saladOrder)s);"
                else: 
                    sql_query = f"update inventory set chickenOrder = %(chickenOrder)s + chickenOrder, fishOrder = %(fishOrder)s + fishOrder, saladOrder = %(saladOrder)s + saladOrder where startOfWeek = %(startOfWeek)s"
                cur.execute(sql_query, order)
    except Exception as e:
        print("Problems:", str(e))