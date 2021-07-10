import os

from flask import Flask, render_template, request, Response
import sqlalchemy

import json

global db


def init_connection_engine():
    db_config = {
        # [START cloud_sql_mysql_sqlalchemy_limit]
        # Pool size is the maximum number of permanent connections to keep.
        "pool_size": 2,
        # Temporarily exceeds the set pool_size if no connections are available.
        "max_overflow": 1,
        # The total number of concurrent connections for your application will be
        # a total of pool_size and max_overflow.
        # [END cloud_sql_mysql_sqlalchemy_limit]

        # [START cloud_sql_mysql_sqlalchemy_backoff]
        # SQLAlchemy automatically uses delays between failed connection attempts,
        # but provides no arguments for configuration.
        # [END cloud_sql_mysql_sqlalchemy_backoff]

        # [START cloud_sql_mysql_sqlalchemy_timeout]
        # 'pool_timeout' is the maximum number of seconds to wait when retrieving a
        # new connection from the pool. After the specified amount of time, an
        # exception will be thrown.
        "pool_timeout": 30,  # 30 seconds
        # [END cloud_sql_mysql_sqlalchemy_timeout]

        # [START cloud_sql_mysql_sqlalchemy_lifetime]
        # 'pool_recycle' is the maximum number of seconds a connection can persist.
        # Connections that live longer than the specified amount of time will be
        # reestablished
        "pool_recycle": 1800,  # 30 minutes
        # [END cloud_sql_mysql_sqlalchemy_lifetime]

    }

    return init_unix_connection_engine(db_config)

def init_unix_connection_engine(db_config):
    # [START cloud_sql_mysql_sqlalchemy_create_tcp]
    # Remember - storing secrets in plaintext is potentially unsafe. Consider using
    # something like https://cloud.google.com/secret-manager/docs/overview to help keep
    # secrets secret.
    db_user = os.environ["DB_USER"]
    db_pass = os.environ["DB_PASS"]
    db_name = os.environ["DB_NAME"]

    # Extract host and port from db_host
    db_socket_dir = os.environ.get("DB_SOCKET_DIR", "/cloudsql")
    cloud_sql_connection_name = os.environ["CLOUD_SQL_CONNECTION_NAME"]

    pool = sqlalchemy.create_engine(
    # Equivalent URL:
    # mysql+pymysql://<db_user>:<db_pass>@/<db_name>?unix_socket=<socket_path>/<cloud_sql_instance_name>
    sqlalchemy.engine.url.URL.create(
        drivername="mysql+pymysql",
        username=db_user,  # e.g. "my-database-user"
        password=db_pass,  # e.g. "my-database-password"
        database=db_name,  # e.g. "my-database-name"
        query={
            "unix_socket": "{}/{}".format(
                db_socket_dir,  # e.g. "/cloudsql"
                cloud_sql_connection_name)  # i.e "<PROJECT-NAME>:<INSTANCE-REGION>:<INSTANCE-NAME>"
        }
    ),
    **db_config
    )

    return pool

def put_data_to_sql(request):
    
    db = init_connection_engine()
    data = request.get_data(as_text=True)
    data_list = data.splitlines()

    sql_insert_lte = "INSERT INTO LTE_M (Technology, Latitude, Longitude, RSRQ, RSRP, RSSI, Network_Operator) VALUES (%s, %s, %s, %s, %s, %s, %s)"
    sql_insert_nbiot = "INSERT INTO NB_IOT (Technology, Latitude, Longitude, RSRQ, RSRP, RSSI, Network_Operator) VALUES (%s, %s, %s, %s, %s, %s, %s)"
    sql_insert_lora = "INSERT INTO Lora (Technology, Latitude, Longitude, Margin, NOfGateways, RSSI, SNR) VALUES (%s, %s, %s, %s, %s, %s, %s)"
    sql_insert_sigfox = "INSERT INTO Sigfox_STM (Technology, Latitude, Longitude, SessionID, SequenceNumber) VALUES (%s, %s, %s, %s, %s)"
    

    with db.connect() as conn:
        for i in data_list:
            entry = i.split(',')
            if entry[0] == 'lte-m':
                conn.execute(sql_insert_lte, entry[0], entry[1], entry[2], entry[3], entry[4], entry[5], 'Telia')
            elif entry[0] == 'nb-iot':
                conn.execute(sql_insert_nbiot, entry[0], entry[1], entry[2], entry[3], entry[4], entry[5], 'Telia')
            elif entry[0] == 'lora':
                conn.execute(sql_insert_lora, entry[0], entry[1], entry[2], entry[3], entry[4], entry[5], entry[6])
            elif entry[0] == 'sigfox':
                conn.execute(sql_insert_sigfox, entry[0], entry[1], entry[2], entry[3][0:2], entry[3][-4:])
            else: 
                return f'ERROR'

    return f'OK'
