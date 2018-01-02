#!/bin/python

import urllib2, urllib, re

class JOBS:
	def __init__(self):
		self.joblist = []

	def getjoblist(self):
		url = "https://www.ucloud.cn/site/about/join/"
		user_agent = 'Mozilla/4.0 (compatible; MSIE 5.5; Windows NT)'
		headers = { 'User-Agent' : user_agent }
		
		try:
			request = urllib2.Request(url, headers = headers)
			response = urllib2.urlopen(request)
			content = response.read()

			pattern = re.compile('"join-title clear">.*?<h2>(.*?)</h2>.*?">(.*?)</div>', re.S)
			items = re.findall(pattern, content)
			for item in items:
				print item[0]
				print item[1]
				print ""
		except urllib2.URLError, e:
			if hasattr(e, "code"):
				print e.code
			if hasattr(e, "reason"):
				print e.reason

		

	def start(self):
		self.getjoblist()

spider = JOBS()

spider.getjoblist()
