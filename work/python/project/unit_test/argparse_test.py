import argparse

def test():
    parser = argparse.ArgumentParser()
    parser.add_argument("echo", help="echo the string you use here")  # ./xxxx 10  //10 is echo must be
    #parser.add_argument("echo", type=int, choices=[1,2,3], help="echo the string you use here")  # ./xxxx 10  //10 is echo must be
    parser.add_argument( "--verbose1", help="turn on the verbose2") #./xxx 10 --verbose 1 //1 is verbose option
    parser.add_argument("--verbose2", help="turn on the verbose2", action="store_true") # ./xxx 10 --verbose 1 --verbos2 //True is verbose2 optional 
    #parser.add_argument("-V", "--verbose2", help="turn on the verbose2", action="store_true") 
    #parser.add_argument("-v", "--verbose3", help="turn on the verbose3", default=0, action="count") // -vvv  args.verbose3 is 2
    group = parser.add_mutually_exclusive_group()
    group.add_argument("-v", "--verbose", action="store_true") # [-v | -q]
    group.add_argument("-q", "--quiet", action="store_true")
    args = parser.parse_args()
    print(args.echo);
    print(args.verbose1);
    print(args.verbose2);
    print(args.verbose);
    print(args.quiet);

if __name__ == "__main__":
    test()
    
