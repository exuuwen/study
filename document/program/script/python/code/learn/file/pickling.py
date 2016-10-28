#!/usr/bin/python

import cPickle as p

filename = 'shoplist.data'

shoplist = ['apple', 'mango', 'carrot', 'banana']

f = file(filename, 'w')
p.dump(shoplist, f)

del shoplist

f.close()


f = file(filename)
storelist = p.load(f)

print storelist
