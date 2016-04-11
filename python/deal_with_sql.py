#! /usr/bin/python 
#encoding:utf-8

import MySQLdb
import urllib2
import sys

if __name__ == '__main__':
    # 建立链接
    conn = MySQLdb.connect(
            host="120.131.68.189",
            user="work",
            passwd="work123456",
            db="dingfu_data",
            charset="utf8") 
    cursor = conn.cursor()  
    if not cursor:
        print("open cursor error!")
        sys.exit(-1)

    # 执行 sql
    cursor.execute("select * from tb_related_schema")

    # 每行中的列通过 row[i] 来得到
    rows = cursor.fetchall()
    for row in rows:
        if len(row) < 2:
            continue
        ori = row[0]
        rel = row[1]

        cursor.close()

    # 结束
    conn.commit()
    conn.close()

