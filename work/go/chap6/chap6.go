package main

import "fmt"

import "math"

type Point struct{ X, Y float64 }

// traditional function
func Distance(p, q Point) float64 {
    return math.Hypot(q.X-p.X, q.Y-p.Y)
}


// same thing, but as a method of the Point type
func (p Point) Distance(q Point) float64 {
    return math.Hypot(q.X-p.X, q.Y-p.Y)
}

func (p *Point) ScaleBy(factor float64) {

    if p == nil {
       return
    }

    p.X *= factor
    p.Y *= factor
}


func main() {

    p := Point{1, 2}
    q := Point{4, 6}
    fmt.Println(Distance(p, q)) // "5", function call
    fmt.Println(p.Distance(q))  // "5", method call

    fmt.Println((&p).Distance(q))  // "5", method call

    //不管你的method的receiver是指针类型还是非指针类型，都是可以通过指针/非指针类型进行调用的，编译器会帮你做类型转换
    (&p).ScaleBy(2)
    p.ScaleBy(2)
    fmt.Println(p)

    var pptr *Point
    pptr = nil
    pptr.ScaleBy(2)


    distance := Point.Distance   // method expression
    fmt.Println(distance(p, q))  // "5"
    fmt.Printf("%T\n", distance) // "func(Point, Point) float64"

    scale := (*Point).ScaleBy
    scale(&p, 2)
    fmt.Println(p)            // "{2 4}"
    fmt.Printf("%T\n", scale) // "func(*Point, float64)"

}
