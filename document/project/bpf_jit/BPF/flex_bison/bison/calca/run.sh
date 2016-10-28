bison -d cale.y
flex cale.l
gcc -o cale cale.c lex.yy.c cale.tab.c
echo -e "start run main\n"
./cale
