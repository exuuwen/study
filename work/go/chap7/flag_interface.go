package main

import (
    "flag"
    "fmt"
)

type Celsius int
type celsiusFlag struct { Celsius }

func (f *celsiusFlag) Set(s string) error {
    var unit string
    var value int
    fmt.Sscanf(s, "%d%s", &value, &unit) // no error check needed
    switch unit {
        case "C", "°C":
            f.Celsius = Celsius(value)
            return nil
    }
    
    return fmt.Errorf("invalid temperature %q", s)
}

func (f *celsiusFlag) String() string {
    return  fmt.Sprintf("%d°C", f.Celsius)
}

func CelsiusFlag(name string, value Celsius, usage string) *Celsius {
    f := celsiusFlag{value}
    flag.CommandLine.Var(&f, name, usage)
    return &f.Celsius
}

var temp = CelsiusFlag("temp", 20.0, "the temperature")

func main() {
    flag.Parse()
    fmt.Println(*temp)
}

