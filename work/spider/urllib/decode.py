#!/bin/python

a = u"\u6700\u9760\u8c31\u7684\u5173\u952e\u5148\u751f\u56e7\u56e7\u68ee\u4e0a\u8d5b\u5b63\u6570\u636e"
print a, type(a)
w = a.encode("utf-8")
print w, type(w)

b = "abc\xe2\x80\x93"
print b, type(b)
y = b.decode("utf-8")
print y, type(y)
