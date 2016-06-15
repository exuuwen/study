#!/bin/python

import urllib, urllib2, cookielib

#url = 'http://42.62.105.202/login?username=zhidemai&password=ZhiDeMai%21%40%23'
url = 'http://42.62.105.202/login'

values = {'username' : 'zhidemai', 'password' : 'ZhiDeMai!@#'}
data = urllib.urlencode(values)


user_agent = 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2272.101 Safari/537.36'

header = { 'User-Agent' : user_agent }

cookie = cookielib.MozillaCookieJar('mycookie.txt') 
opener = urllib2.build_opener(urllib2.HTTPCookieProcessor(cookie))
urllib2.install_opener(opener)

#GET method
url = url + '?' + data
req = urllib2.Request(url, headers=header)

#POST method
#req = urllib2.Request(url, data, headers=header)

#req.get_method = lambda:'PUT'
try:
	content = urllib2.urlopen(req)
except urllib2.URLError, e:
	print e.code
	print e.reason
else:
	print content.read()
	for item in cookie:
		print 'Name = '+item.name
		print 'Value = '+item.value
	cookie.save(ignore_discard=True, ignore_expires=True)
