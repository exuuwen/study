import commands
import sys

if len(sys.argv) < 2:
        print 'usage: %s command' %sys.argv[0]
        exit(1)

cmd = sys.argv[1]
(status, result)  = commands.getstatusoutput(cmd)
if status:
        print 'fail exec %s(%s)'%(sys.argv[1], result)
else:
        lines = result.split('\n')
	num = 0
        for line in lines:
                print num,line
		num += 1

