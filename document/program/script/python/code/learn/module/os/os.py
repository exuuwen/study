#!/usr/bin/python

import sys
import os

if len(sys.argv) < 2:
	print 'no option'
	sys.exit()

if sys.argv[1].startswith('--'):
	option = sys.argv[1][2:]
	if option == 'version':
		print 'version1.2'
	elif option == 'help':
		print 'just a help'
	else:
		print 'unknow option'
	sys.exit()
else:
	for param in sys.argv[1:]:
		print "%s"%param

print "pwd:%s"%os.getcwd()
print "ls:%s"%os.listdir('.')
print "os:%s"%os.name
os.remove("a")

path = "/home/exuuwen/exuuwen/python/os/os.py"
path1 = "/home/exuuwen/exuuwen/python/os"
print "dir %s, file %s" %os.path.split("/home/exuuwen/a.file")

if os.path.isfile(path):
  print "%s is file"%path

if os.path.isdir(path1):
  print "%s is dir"%path1

