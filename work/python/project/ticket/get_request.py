#!/ust/bin/python3

import requests

def get_request_data(date, from_station, to_station):
    """request跟12306拿数据"""
    url = 'https://kyfw.12306.cn/otn/leftTicket/log?leftTicketDTO.train_date={}&leftTicketDTO.from_station={}&leftTicketDTO.to_station={}&purpose_codes=ADULT'.format(date, from_station, to_station)
    # 添加verify=False参数不验证证书
    r = requests.get(url, verify=False)
    #print(r);

    url = 'https://kyfw.12306.cn/otn/leftTicket/query?leftTicketDTO.train_date={}&leftTicketDTO.from_station={}&leftTicketDTO.to_station={}&purpose_codes=ADULT'.format(date, from_station, to_station)
    r = requests.get(url, verify=False)
    print(url)
    #print(r.text);
    return r.json();

   
