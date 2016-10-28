#!/usr/bin/python

from heapq import *
from random import shuffle

data = range(10)
shuffle(data)

heap = []
for n in data:
  heappush(heap, n)

print heap
heappush(heap, 1.5)
print heap

print "pop", heappop(heap)


print "heapreplace", heapreplace(heap, 3.5)

print heap
