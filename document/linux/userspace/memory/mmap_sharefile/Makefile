object1=write_process.o
object2=read_process.o

all:write_process read_process

read_process:$(object2)
	cc -o read_process $(object2)

write_process:$(object1)
	cc -o write_process $(object1)

clean:
	rm -rf *.o write_process read_process