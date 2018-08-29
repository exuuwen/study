# -*- encoding:utf-8 -*-
import time
import requests
from lxml import html
from PIL import Image  
from StringIO import StringIO
#from io import BytesIO

url_base='https://www.douban.com/accounts/login'
#pic_url='https://www.douban.com/misc/captcha?id='

form_data = {
    'source':'index_nav',
    'form_email':'vincent7.wen@gmail.com',
    'form_password':'wxztt8789'
}

s = requests.session()

r = s.get(url_base,  allow_redirects=False)

tree = html.fromstring(r.content)
#<div class="captcha_block">
#    <input type="text" autocomplete="off" class="inp" id="captcha_field" name="captcha-solution" tabindex="3" placeholder="验证码"/>
#    <input type="hidden" name="captcha-id" value="HVM5DLBE807A8eEuzDuVXhep:en"/>
#</div>
#<img id="captcha_image" src="https://www.douban.com/misc/captcha?id=HVM5DLBE807A8eEuzDuVXhep:en&amp;size=s" alt="captcha" class="captcha_image" title="看不清楚?点图片可以换一个"/>

result = tree.xpath('//div[@class="captcha_block"]/input[@name="captcha-id"]/@value')
if len(result):
    form_data['captcha-id'] = result[0]
    print("id: %s" %form_data['captcha-id'])

    result_url = tree.xpath('//img[@id="captcha_image"]/@src')
    pic_url = result_url[0]
    print(pic_url)
    response = requests.get(pic_url)
    img = Image.open(StringIO(response.content))
    #img = Image.open(BytesIO(response.content))
    print('请输入你看到的字母')
    img.show()  
    solution = raw_input()
    #solution = input()
    form_data['captcha-solution'] = solution
    print("solution: %s" %form_data['captcha-solution'])

s.post(url_base, data=form_data,  allow_redirects=False)

my_url = "https://www.douban.com/people/29276930/"
#站内的测试链接，用来判断是否登入成功（你要选你自己的链接，不能用我这个）
r = s.get(my_url, allow_redirects=False)
if r.status_code==200 :
        print('登陆成功')
else:
        print('登录失败')


