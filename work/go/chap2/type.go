package main

import "fmt"

type Test uint8

func (test Test)String() string { return fmt.Sprintf("%dT", test)}

const t Test = 120

func main() {
    fmt.Printf("%s\n", t) 
}

