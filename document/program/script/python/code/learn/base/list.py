#!/usr/bin/python

#list
s = ['yy', 'ss']
shoplist = ['z', 'b', 'm', 'e', 'aw', s]

print 'I have', len(shoplist), 'item to purchase'

print 'these are:',

for item in shoplist:
	print item,

print '\nappend f to list'
shoplist.append('f')

print 'now my list:', shoplist

shoplist.sort()
print 'sort my list:', shoplist

print 'the first item:', shoplist[0]
print 'last to four item:', shoplist[1:3]
print 'the last item:', shoplist[len(shoplist) - 1]

print 'the first item second letter:', shoplist[0][1]
print 'delet the second item'

del shoplist[1]  
print 'last my list:', shoplist

#tuple

zoo = ('dog', 'monkey', 'bird', s)

print 'zoo animal number:', len(zoo)

new_zoo = ('elepant', 'snake', 'cat', zoo)

print 'new zoo animal number:', len(zoo)
print 'All new zoo animal:', new_zoo

print 'new zoo buy from old zoo:', new_zoo[3]
print 'the last one buy from old zoo:', new_zoo[3][len(zoo) - 1][len(s) - 1]
new_zoo[3][len(zoo) - 1].sort()
print 'after s sort the last one buy from old zoo:', new_zoo[3][len(zoo) - 1][len(s) - 1]


name = 'xu wen'
age = 25

print 'this is %s, age %d' % (name, age)


#dict

ab = {  'wx' : 'aaaaa',
	    'pfx' : 'bbbbc',
		'zxb' : 'ccccc',
		'ruf' : 'sssss'
	 }

print 'wx:%s'%ab['wx']

ab['jss'] = 'ddddd'

del ab['ruf']

print 'the dict len %d'%len(ab)

for name, addr in ab.items():
	print 'name %s, addr %s'%(name, addr)

if ab.has_key('jss'):
	print 'jss addr %s'%ab['jss']


#reference
 
print 'Simple Assignment' 
shoplist = ['apple', 'mango', 'carrot', 'banana'] 
mylist = shoplist # mylist is just another name pointing to the same object! 
del shoplist[0] 
print 'shoplist is', shoplist
print 'mylist is', mylist # notice that both shoplist and mylist both print the same list without 
# the 'apple' confirming that they point to the same object print 'Copy by making a full slice' 
mylist = shoplist[:] # make a copy by doing a full slice 
del mylist[0] # remove first item 
print 'shoplist is', shoplist
print 'mylist is', mylist 
# notice that now the two lists are different

name='Swaroop'

if name.startswith('Swar'):
	print 'yes start with Swar'

if 'roop' in name:
	print 'yes contains "roop"'

if name.find('war')!=-1:
	print 'yes find "war"'

print name.replace('roop', 's')

delimiter='*'
mylist=['a','b','c']

print delimiter.join(mylist)
