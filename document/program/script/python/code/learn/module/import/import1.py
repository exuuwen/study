#!/usr/bin/python

import sys

#path='/xxx/xx' # the mymodule.py should be in the directory
#sys.path.append(path)

import mymodule

print "the command line arguments are:"
for i in sys.argv:
	print i

if __name__ == '__main__':
	print 'The import.py program is being run by itself'
else:
	print 'The import.py program has been imported from another module'


mymodule.sayhi()

print "mymodule.version:", mymodule.version
