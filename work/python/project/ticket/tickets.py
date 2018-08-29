# /usr/bin/python3
# -*- coding: utf-8 -*-


"""命令行火车票查看器

Usage:
    tickets [-gdtkz] <from> <to> <date>

Options:
    -h,--help   显示帮助菜单
    -g          高铁
    -d          动车
    -t          特快
    -k          快速
    -z          直达

Example:
    tickets 北京 上海 2016-10-10
    tickets -dg 成都 南京 2016-10-10
"""

def cli():
    from docopt import docopt
    from stations import store_stations
    from get_request import get_request_data
    from formats import format_data

    stations_map = store_stations()
    
    arguments = docopt(__doc__)
    from_station = stations_map.get(arguments['<from>'])
    to_station = stations_map.get(arguments['<to>'])
    date = arguments['<date>']
    options = ''.join([
        key for key, value in arguments.items() if value is True
    ])

    print(from_station, to_station, date, options)
    result = get_request_data(date, from_station, to_station)
    if result['httpstatus'] == 200 and result.get('data') is not None and result.get('data') != '':
        return format_data(result.get('data'), options)
    else:
        return result['messages']



if __name__ == '__main__':
    print(cli())
