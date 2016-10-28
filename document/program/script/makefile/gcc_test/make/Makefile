main: hello.o temp.o  hello.h  temp.h
	gcc -o main hello.o temp.o
hello.o: hello.c  hello.h
	gcc     -c     -I /home/cr7/temp/make/h/  /home/cr7/temp/make/s/hello.c       
temp.o: temp.c  temp.h
	gcc    -c    -I /home/cr7/temp/make/h/   /home/cr7/temp/make/s/temp.c         
clean:
	rm main hello.o temp.o
