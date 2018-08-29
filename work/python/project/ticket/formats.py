#!/usr/bin/python3

'''
from prettytable import PrettyTable
from colorama import init, Fore

def format_data(datas, options):
    """返回规范的 table"""
    ptable = PrettyTable()
    header = '车次 车站 时间 历时 一等 二等 软卧 硬卧 硬座 无座'.split()
    ptable._set_field_names(header)
    for train in datas:
        trains = train.get('queryLeftNewDTO')
        if trains is not None and trains != '':
            initial = trains['station_train_code'].lower()[0]
            if not options or initial in options:
                ptable.add_row([trains['station_train_code'], Fore.GREEN + "%s -> %s" % (trains['from_station_name'], trains['to_station_name']) + Fore.RESET, Fore.RED + "%s -> %s" % (trains['start_time'], trains['arrive_time']) + Fore.RESET , trains['lishi'], trains['zy_num'], trains['ze_num'], trains['rw_num'], trains['yw_num'], trains['yz_num'], trains['wz_num']])
    return ptable
'''

def format_data(datas, options):
    if datas.get('result') is not None and datas.get('result') != '':
       trains = datas.get('result') 
       return trains[0].split('|')

