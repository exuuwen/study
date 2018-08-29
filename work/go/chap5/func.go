package main
import "fmt"
import "os"

func sum(vals...int) int {
    total := 0
    for _, val := range vals {
        total += val
    }
    
    return total
}

func errorf(linenum int, format string, args ...interface{}) {
    fmt.Fprintf(os.Stderr, "Line %d: ", linenum)
    fmt.Fprintf(os.Stderr, format, args...)
    fmt.Fprintln(os.Stderr)
}

func test() {
    fmt.Println("in test")
}

func triple(x int) (result int) {
    defer func() { result += x }()
    return x + x
}

func defer_sample() {
    fmt.Println("s start")
    defer func() { fmt.Println("in defer") }()
    //defer test()
    fmt.Println("s end")
}

func my_panic_recover(value bool) {
    
    defer func() {
        if p := recover(); p != nil {
            fmt.Printf("internal error: %v\n", p)
        }
    }()

    if value {
        panic("hahha")
    }
    
}

func my_test() {
    my_panic_recover(true)
}

func main() {
    fmt.Println(sum())           // "0"
    fmt.Println(sum(3))          // "3"
    fmt.Println(sum(1, 2, 3, 4)) // "10"

    values := []int{1, 2, 3, 4}
    fmt.Println(sum(values...)) // "10"

    linenum, name , errnum := 12, "count", 100
    errorf(linenum, "undefined: %s %d", name, errnum) // "Line 12: undefined: count"

    fmt.Println(triple(4)) // "12"
    defer_sample()

    my_test()
}
