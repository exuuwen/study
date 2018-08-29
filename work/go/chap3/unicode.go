package main

import "fmt"
import "unicode/utf8"

func main() {
    s := "文旭"
    fmt.Printf("% x\n", s) //byte
    r := []rune(s) // unicode
    fmt.Printf("%x\n", r) 
    fmt.Println(string(r))
    fmt.Println(utf8.RuneCountInString(s))
}
