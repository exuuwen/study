package main 
import (
	"fmt" 
)


func main() {  

	var a [3]int = [3]int{1, 2, 3}
	
	for _,data := range a {
		fmt.Println(data)
	}

	q := [...]int{1, 2, 3}
	fmt.Printf("%T\n", q) 

	nums := [...]string{1: "one", 2: "two", 3: "three"}
	parts := nums[1:3]
	fmt.Printf("%T\n", nums) 

	for _, d:= range parts {
		fmt.Println(d)
	}

	//ages := map[string]int{
	//"alice":  31,
	//"charlie": 34,
	//}
	ages := make(map[string]int) //must make first
	ages["alice"] = 31
	ages["charlie"] = 34
	ages["ss"] = 38

	fmt.Println(ages["alice"])
	delete(ages, "alice") 

	_, ok := ages["bob"]
	if !ok { /* "bob" is not a key in this map; age == 0. */ 
		fmt.Printf("no bob\n")
	}

	for name, age := range ages {
		fmt.Printf("%s\t%d\n", name, age)
	}

	type T struct{ a, b int }
	t1 := T{1, 2}
	t2 := T{a:1, b:2}

	fmt.Printf("t1.a %d\n", t1.a)
	fmt.Printf("t2.b %d\n", t2.b)
}
//go build hello.go
