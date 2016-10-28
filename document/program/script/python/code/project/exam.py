#!/usr/bin/python
import os
import random
import sys

if len(sys.argv) < 2:
  print "usage: ./exam.py examname.txt"
  sys.exit(1)

number = 1
fw = file(sys.argv[1], 'w')

typelist = []
dirpath = "type"
typenum = 1

while True:
  path = dirpath + repr(typenum) 
  if os.path.isdir(path):
    typelist.append(typenum)
    typenum += 1
  else:
    break

print "typelist", typelist

for t in typelist: 
  examlist = []
  filepath = 1;
  while True:
    path = dirpath + repr(t) + os.sep + repr(filepath)
    if os.path.isfile(path):
      examlist.append(filepath)
      filepath += 1
    else:
      break

  print "type %d examlist"%t, examlist

  chooselist = random.sample(examlist, 3)
  print "type %d chooselist"%t, chooselist


  for i in chooselist:
    path = dirpath + repr(t) + os.sep + repr(i)
    fr = file(path)
    while True:
      line = fr.readline()
      if len(line) == 0:
        break;
      line = repr(number) + ". " + line
      fw.write(line)
      number += 1

    fr.close()

fw.close()

