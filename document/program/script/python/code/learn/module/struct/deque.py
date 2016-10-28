#!/usr/bin/python
from collections import deque

q = deque(range(5))

print "deque", q

q.append(5)
print "append 5", q

q.appendleft(6)
print "appendleft 6", q

print "pop", q.pop()
print "popleft", q.popleft()

q.rotate(3)
print "rotate 3", q

q.rotate(-1)
print "rotate -1", q



