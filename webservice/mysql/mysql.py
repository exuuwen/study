#!/usr/bin/env python2
# coding=utf8

import sys, os
import time
import MySQLdb

'''
@summary: 与Mysql连接以及执行语句
'''
class MySqlClient:
    
    def init(self, ip, port, usr, pwd, db):
        # connect to mysql
        try:
            self.conn = MySQLdb.connect(host=ip,port=port,user=usr,passwd=pwd, charset='utf8', db=db)
            self.cursor = self.conn.cursor()
            
        except MySQLdb.Error, e:
            cur_time = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(time.time()))
            print "%s - Error %d: %s" % (cur_time, e.args[0], e.args[1]) 
            return False

        return True
    
    def close(self):
        self.conn.close()
        self.cursor.close()

    def mysql_exec(self, sql):
        try:
            self.cursor.execute(sql)
	    self.conn.commit()
            rows = self.cursor.fetchall()
           
	    if 'select' in sql: 
                desc = self.cursor.description
                num = len(desc)
                i = 0
                while(i < num):
                   print desc[0][i],
                   i += 1  

                for row in rows:
                    print row
            else:
                print "commit" 
                #raw[0] row[key]
                for row in rows:
                    print row

        except MySQLdb.Error, e:
            cur_time = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(time.time()))
            print "%s - Error %d: %s" % (cur_time, e.args[0], e.args[1]) 
            return False

        return True 


if __name__ == "__main__":

    argc = len(sys.argv)
    
    if argc < 7:
        print "usage: %s mysql_ip mysql_port mysql_usr mysql_pwd mysql_db command" % sys.argv[0]
        exit(1)
    
    mysql_ip = sys.argv[1]
    mysql_port = int(sys.argv[2])
    mysql_usr = sys.argv[3]
    mysql_pwd = sys.argv[4]
    mysql_db = sys.argv[5]
    commands = sys.argv[6]

    mysql_client = MySqlClient()
    ret = mysql_client.init(mysql_ip, mysql_port, mysql_usr, mysql_pwd, mysql_db)
    mysql_client.mysql_exec(commands) 
    mysql_client.close()

    exit(0)
