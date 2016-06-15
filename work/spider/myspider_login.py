import scrapy

#from tutorial.items import DmozItem
class MySpider(scrapy.Spider):
	name = "my.spider"
	allowed_domains = ["my"]

	def start_requests(self):
		return [scrapy.FormRequest("http://42.62.105.202/login?username=zhidemai&password=ZhiDeMai%21%40%23", callback=self.logged_in)]

	def logged_in(self, response):
		print response.url	

