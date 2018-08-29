package main

import (
    "fmt"
    "io"
    "bytes"
    "os"
)

//w is a interface with nil type and nil value
var w io.Writer

//一个包含nil指针的接口不是nil接口
func f(out io.Writer) {
    if out != nil {
        fmt.Println("out is not nil")
    } else {
        fmt.Println("out is nil")
    }

    return 
}

func main() {

    if w == nil {
        fmt.Println("w is nil")
    }

    // buf is a interface with buffr type and nil value
    var buf *bytes.Buffer
    
    // is ok 
    var buf2 io.Writer

    f(buf)
    f(buf2)

    //x.(T) 类型断言
    var w2 io.Writer = os.Stdout
    if _, ok := w2.(*os.File); ok {
        // ...use f...
        fmt.Println("use f as a file")
    }

    return
}
