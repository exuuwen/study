# /usr/bin/python3
# -*- coding: utf-8 -*-

"""取得城市的英文缩写，保存在 stations.py 里面"""

import re
import requests


def store_stations():
    requests.packages.urllib3.disable_warnings()
    urls = 'https://kyfw.12306.cn/otn/resources/js/framework/station_name.js'
    response = requests.get(urls, verify=False)
    stations_array = re.findall(u'([\u4e00-\u9fa5]+)\|([A-Z]+)', response.text)

    #print(stations_array)
    return dict(stations_array)

