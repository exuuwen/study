#!/usr/bin/python

command = 'print "hello"'
exec command

agloth = '2*3'
print agloth, '=', eval(agloth)

assert True

#change a to a string 
a = 50

print `a`
print repr(a)
