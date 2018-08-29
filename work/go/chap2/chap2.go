package main 
import (
	"fmt" 
	"os"
)

func main() {  
	fmt.Printf("hello, world\n") 

	var s, sep string
	for _, arg := range os.Args[1:] {
		s += sep + arg
		sep = " "
	}

	fmt.Println(s)

	var i = 0;
	for i != 10 {
		fmt.Printf("a\n") 
		i++;
	}

	var a [3]int = [3]int{1, 2, 3}
	a[0] = 2;
	
	for i, v := range a {
		fmt.Printf("%d %d\n", i, v)
	}

	q := [...]int{1, 2, 3}
	fmt.Printf("%T\n", q) 

	r := [...]int{3: -1}
	for i, v := range r {
		fmt.Printf("%d %d\n", i, v)
	}
}
//go build hello.go
