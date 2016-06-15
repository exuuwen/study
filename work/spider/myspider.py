import scrapy

#from tutorial.items import DmozItem
class MySpider(scrapy.Spider):
	name = "my.spider"
	allowed_domains = ["my"]
	start_urls = [
		"http://42.62.105.202/login"	
	]

	def parse(self, response):
		print response.url	
		for sel in response.xpath('//ul/li'):
			title = sel.xpath('a/text()').extract()
			link = sel.xpath('a/@href').extract()
			desc = sel.xpath('text()').extract()
			print title, link, desc
