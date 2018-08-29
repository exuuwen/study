import os
import re
import time
import math
import threading

cputils_last = []
cputils = []
cur = 0
SLOT = 5

lock = threading.Lock()

def get_cpustat():
	global cur
	global cputils
	global cputils_last
	global SLOT
	global lock

	f = open('/proc/stat', 'rb')
	try:
		lines = f.readlines()
	finally:
		f.close()

	num = 0;
	for line in lines:
		a = re.search(r'cpu. (?P<number0>\d+) (?P<number1>\d+) (?P<number2>\d+) (?P<number3>\d+) (?P<number4>\d+) (?P<number5>\d+) (?P<number6>\d+) (?P<number7>\d+) (?P<number8>\d+) (?P<number9>\d+)', line)
		if a:
			total = long(a.group('number0')) + long(a.group('number1')) + long(a.group('number2')) + long(a.group('number3')) + long(a.group('number4')) + long(a.group('number5')) + long(a.group('number6')) +long(a.group('number7')) +long(a.group('number8')) +long(a.group('number9')) 
			idle = long(a.group('number3'))
			if cur == 0:
				cputils_last.append([idle, total])
				cputils.append([])
			else: 
				s_idle = idle - cputils_last[num][0]
				s_total = total - cputils_last[num][1]
				print num, cur, s_idle, s_total
				if cur < SLOT + 1:
					cputils[num].append([s_idle, s_total])
				else:
					#cur 0 is no store
					cputils[num][(cur-1)%SLOT] = [s_idle, s_total]

				cputils_last[num] = [idle, total]
				
                	num += 1
	if cur%SLOT == 0 and cur > 0:
		for iter in cputils:
			print iter

	if cur > 0 and (cputils[2][(cur-1)%SLOT][0] < 20 or cputils[1][(cur-1)%SLOT][0] < 20) and lock.acquire(False):
		func = threading.Thread(target=just_print, args=(cur,))
		func.start()	

	cur += 1;
	timer = threading.Timer(1.0, get_cpustat)
	timer.start()

def just_print(cur):
	print 'hahahhahaha1', cur
	time.sleep(10)
	print 'hahahhahaha2', cur
	lock.release()

if __name__ == "__main__":
	get_cpustat()
