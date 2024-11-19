import pandas as pd
import psycopg2
from io import StringIO

from sklearn.model_selection import train_test_split
from db import db_engine 

create_table_sql = """
CREATE TABLE IF NOT EXISTS inventory (
    id serial PRIMARY KEY,
    startOfWeek DATE NOT NULL UNIQUE,
    chickenOrder INTEGER DEFAULT 0,
    fishOrder INTEGER DEFAULT 0,
    saladOrder INTEGER DEFAULT 0
);
"""

df = pd.read_csv("data/pastInventory.csv", index_col="id")
test_size= 0.20 # 20% for testing
df_train, df_test = train_test_split(df, test_size=test_size, random_state=1234)
df_train.to_csv("data/train.csv", index=False)
df_test.to_csv("data/test.csv", index=False)
df_test = df_test.copy()

df = pd.concat([df_train, df_test])

try:
    conn = psycopg2.connect(db_engine())
    cur = conn.cursor()
    print("Create inventory table")
    cur.execute(create_table_sql)
    conn.commit()
    print("Insert train and test data into table ...")
    buffer = StringIO()
    df.to_csv(buffer, index_label="id", header=False)
    buffer.seek(0)
    cur.copy_from(buffer, "inventory", sep=",")
    conn.commit()
    cur.close()
except Exception as e:
    print("Problems:", str(e))