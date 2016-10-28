flex test.l
gcc -o main main.c lex.yy.c 
echo -e "start run main\n"
./main
