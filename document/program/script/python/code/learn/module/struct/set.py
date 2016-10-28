#!/usr/bin/python

print "set", set([1, 2, 4, 1, 9, 0, 2])

a = set([1, 2, 3])
b = set([2, 3, 4])
print "a|b", a | b

c = a & b
print "a&b", c

print "c<=a", c <= a
print "c>=a", c >= a


