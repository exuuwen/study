#!/usr/bin/python

import sys

class ShortInputException(Exception):
	def __init__(self, length, atleast):
		Exception.__init__(self)
		self.length = length

		self.atleast = atleast

try:
	s = raw_input("Enter something again--->")
	if len(s) < 4:
		raise ShortInputException(len(s), 4)
except EOFError:
        print '\nWhy did you do a EOF on me?'
#capture n error in a block
#except (AError, BError), e:
except ShortInputException, x:
        print '\nShortInputException error occurred, input length %d\
was exceting at least %d'%(x.length, x.atleast)
except:
	print '\nSomething error occurred'
#else:
finally:# It will be here whatever the except
        print 'Done'
