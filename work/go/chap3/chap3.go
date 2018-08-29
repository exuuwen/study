package main 
import (
	"fmt" 
	"strings"
	"strconv"
)

func basename(s string) string {
	slash := strings.LastIndex(s, "/") // -1 if "/" not found
	s = s[slash+1:]
	if dot := strings.LastIndex(s, "."); dot >= 0 {
		s = s[:dot]
	}

	return s
}

var ascii = 'a'

func main() {  
	var x uint8 = 1<<1 | 1<<5
	var y uint8 = 1<<1 | 1<<2

	fmt.Printf("%08b\n", x&^y)

	var apples int32 = 1
	var oranges int16 = 2
	//var compote int = apples + oranges // compile error
	var compote = int(apples) + int(oranges)
	fmt.Printf("%d\n", compote);

	z := int64(0xdeadbeef)
	fmt.Printf("%d %[1]x %#[1]x %#[1]X\n", z)

	fmt.Println(basename("a/b/c.go")) // "c"
	fmt.Println(basename("c.d.go"))  // "c.d"
	fmt.Println(basename("abc"))  // "abc"

    fmt.Printf("%d %[1]c %[1]q\n", ascii) //97 a 'a'

	m := 123
	n := fmt.Sprintf("%x", m)
	fmt.Println(n, strconv.Itoa(m)) // "123 123"

	xx, _ := strconv.Atoi("123");
	fmt.Println(xx);

}
//go build hello.go
