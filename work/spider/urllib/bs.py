#!/bin/python

import bs4, re
from bs4 import BeautifulSoup

html = """
<html><head><title>The Dormouse's story</title></head>
<body>
<p class="title" name="dromouse"><b>The Dormouse's params</b></p>
<p class="story">Once upon a time there were three little sisters; and their names were
<a href="http://example.com/elsie" class="sister" id="link1"><!-- Elsie --></a>,
<a href="http://example.com/lacie" class="sister" id="link2">Lacie</a> and
<a href="http://example.com/tillie" class="sister" id="link3">Tillie</a>;
and they lived at the bottom of a well.</p>
<p class="story">...</p>
<div class="list">
<ul class="test">
<li>11111</li>
<li>22222</li>
<li>33333</li>
<li>44444</li>
<li>55555</li>
</ul>
</div>
</body>
"""

soup = BeautifulSoup(html)
#print soup.prettify()

print '-------------'
print soup.find_all('a')
#print soup.find_all(['a', 'b'])
print soup.find_all(id='link2')
print soup.find_all('a', href=re.compile("elsie"), id='link1')
#print soup.find_all("a", class_="sister")
#other attr`
#data_soup.find_all(attrs={"data-foo": "value"})
print soup.find_all(text=re.compile("Dormouse"))
print '-------------'
print soup.select('a')
print soup.select('.sister') #class
print soup.select('#link2')#id

print soup.select('a[id="link2"]')
print soup.select('p a[href="http://example.com/elsie"]')

print soup.select('div[class="list"] ul[class="test"] li')
print '-------------'
print soup.title
print soup.a
print soup.p
print '-------------'
print soup.title.name
print soup.a.name
print soup.p.name
print '-------------'
print soup.title.attrs
print soup.a.attrs
print soup.p.attrs
print '-------------'
print soup.a['href']
print soup.p['class']
print '-------------'
print soup.title.string
print soup.a.string
print soup.p.string

print '-----------------------------------------------'
#each tag's prev_sibling and next_sibling is string for '\n' or others
a = (soup.p.next_sibling.next_sibling)
print a['class']
print '-----------------------------------------------'
for child in  a.children:
	if type(child) == bs4.element.Tag and child['id'] == 'link2':
			print 'wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww'		
	else:
		print child

soup.find_all(id='link2')
