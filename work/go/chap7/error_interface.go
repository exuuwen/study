package main

import (
    "fmt"
)

type Errno uint 

var errors = [...]string{
    1:   "operation not permitted",   // EPERM
    2:   "no such file or directory", // ENOENT
    3:   "no such process",           // ESRCH
}

func (e Errno) Error() string {
    if 0 <= int(e) && int(e) < len(errors) {
        return errors[e]
    }
    
    return fmt.Sprintf("errno %d", e)
}


func main() {
    var err error = Errno(2);

    fmt.Println(err) 
}
