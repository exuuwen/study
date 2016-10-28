#!/usr/bin/python

poem = '''snuwhduedh
skwisjwijwjddjiw
seidjeudueduedhd!
'''
#mode 'w', 'r', 'a'
#file is equal open
f = file("test.txt", 'w')
f.write(poem)
lines = ["last second sentence\n", "last sentence\n"] 
f.writelines(lines)
f.close()

f = file('test.txt')
#default is 'r'
print "readline:"
while True:
	line = f.readline()
	if len(line) == 0:
		break;
	print line,

f.close()


f = file('test.txt')
print "readlines:"
print f.readlines()
f.close()


f = file('test.txt')

print "read(10):"
print  f.read(10)
f.close() 

f = file("test.txt")
print "line:"
for line in f:
  print line,


