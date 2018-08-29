#/usr/bin/env python3

import requests

url_base='https://github.com/'  

raw_cookies = '_octo=GH1.1.1056759442.1495791294; _gat=1; _ga=GA1.2.719021476.1495791294; tz=Asia%2FShanghai; user_session=0gD8ooKT0CKDo9t5WtwC_CY9WBhpfAUyQgqsfn4jjkKVpDN5;__Host-user_session_same_site=0gD8ooKT0CKDo9t5WtwC_CY9WBhpfAUyQgqsfn4jjkKVpDN5; logged_in=yes; dotcom_user=exuuwen; _gh_sess=eyJsYXN0X3dyaXRlIjoxNDk2MjEzODIzNzU0LCJmbGFzaCI6eyJkaXNjYXJkIjpbXSwiZmxhc2hlcyI6eyJhbmFseXRpY3NfZGltZW5zaW9uIjp7Im5hbWUiOiJkaW1lbnNpb241IiwidmFsdWUiOiJMb2dnZWQgSW4ifX19LCJzZXNzaW9uX2lkIjoiODU3ZjI3MzA5MGRiNmIwNDg4YzJjOGRlNzVlMWFjODYifQ%3D%3D--ae3f29bd21f4dd3dd4411d3402fbaf2b990fffc1'

cookies={}  
for line in raw_cookies.split(';'):  
    key,value=line.split('=', 1)#1代表只分一次，得到两个数据  
    cookies[key]=value 

s = requests.Session()

r = s.get(url_base, cookies=cookies) #only the first time need
r = s.get(url_base) 

print(r.text)

