#!/usr/bin/python
i = 1
print i
i = i + 1
print i

i = 'hello world\
hah!'
a = '''aaaa
dee'''
print i, a, 2

guess = 10
running = True

while running:
	input1 = raw_input('Enter an integer or quit or bypass:')
	if input1 == "quit":
		print "a quit command"
		break
	elif input1 == "bypass":
		print "a bypass command"
		continue;
	elif guess == int(input1):
		print "good u r right"
		running = False	
	elif guess > int(input1):
		print "it is a little small"
		print "go on loop to guess"
	else:
		print "it is a little big"
		print "go on loop to guess"
else:#option
	print "jump out the loop"		
	print "Done"

for i in range(1, 5):
	print i
else:#option
	print "the loop is over"

def func(b, a=15):#just right ones can be setted as defealt
	global x
	print "local x:", x, "b:", b, "a:", a
	x = 2
	print "local x to:", x

x = 10;
print "global x:", x
func(5)
print "global x change to x:", x
	
def func1(a, b=5, c=10):
	print "a is", a, "b is", b, "c is", c

func1(3, 7)
func1(25, c=24)
func1(c=50, a=100)

def func_return(a, b):
	return a*b

print "func_return(2, 3):", func_return(2, 3)

#you also can **args: dictionary(key:value)
def varable_param(pos, *args):
        print "pos is %d" %pos
        for option in args:
                print option

varable_param(10, "a", "b", "se", 2)

def special_param(*args, **keypar):
	print args
	print keypar
	print "%(name)d, %(job)s" %keypar

special_param(1, 2, 3, name = 1, job = 'sss')

lis = ('a', 'b')
test = {'job': 'www', 'name': 2}
special_param(*lis, **test)	
