#!/bin/python

import urllib2, urllib, re, json

class JOBS:
	def __init__(self):
		self.joblist = []

	def getjoblist(self):
		base_url = "http://tieba.baidu.com/p/3138733512"
		params = {"see_lz" : 1, "pn" : 1}
		data = urllib.urlencode(params)
		url = base_url + '?' + data
		print url
		user_agent = 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2272.101 Safari/537.36'
		headers = { 'User-Agent' : user_agent }
		
		try:
			request = urllib2.Request(url, headers = headers)
			response = urllib2.urlopen(request)
			content = response.read()
			content = re.sub(re.compile('&quot;'), '"', content)
			content = re.sub(re.compile('&lt;'), '<', content)
			content = re.sub(re.compile('&gt;'), '>', content)
			content = re.sub(re.compile('<img.*?>| {7}|'), '', content)
			content = re.sub(re.compile('<a.*?>|</a>'), '', content)

			pattern = re.compile('''<div class="l_post l_post_bright.*? data-field='(.*?)' >''', re.S)
			items = re.findall(pattern, content)
			for item in items:
				#json to dict
				a = json.loads(item)
				print '-----------------------------------------------------------'
				print a['content']['post_index'], a['author']['user_name'], a['author']['user_id']
				print re.sub(re.compile('<br><br>|<br>'), '\n', a['content']['content'])
		except urllib2.URLError, e:
			if hasattr(e, "code"):
				print e.code
			if hasattr(e, "reason"):
				print e.reason

		

	def start(self):
		self.getjoblist()

spider = JOBS()

spider.getjoblist()
