package main

import (
    "time"
    "fmt"
    "text/tabwriter"
    "os"
    "sort"
)


type Track struct {
    Title  string
    Year   int
    Length time.Duration
}

var tracks = []*Track{ 
    {"aaa", 1000, length("3m38s") },
    {"bbb", 500, length("1m38s") },
    {"ccc", 2000, length("2m38s") },
    {"aaa", 500, length("4m38s") },
}


func length(s string) time.Duration {
    d, err := time.ParseDuration(s)
    if err != nil {
        panic(s)
    }
    
    return d
}

func printTracks(tracks []*Track) {
    const format = "%v\t%v\t%v\n"
    tw := new(tabwriter.Writer).Init(os.Stdout, 0, 8, 2, ' ', 0)
    fmt.Fprintf(tw, format, "Title", "Year", "Length")
    fmt.Fprintf(tw, format, "-----", "------", "-----")
    for _, t := range tracks {
        fmt.Fprintf(tw, format, t.Title, t.Year, t.Length)
    }
    tw.Flush() // calculate column widths and print table
}

type byTitle []*Track
func (x byTitle) Len() int           { return len(x) }
func (x byTitle) Less(i, j int) bool { return x[i].Title < x[j].Title }
func (x byTitle) Swap(i, j int)      { x[i], x[j] = x[j], x[i] }

type byYear []*Track
func (x byYear) Len() int           { return len(x) }
func (x byYear) Less(i, j int) bool { return x[i].Year < x[j].Year }
func (x byYear) Swap(i, j int)      { x[i], x[j] = x[j], x[i] }

type byLength []*Track
func (x byLength) Len() int           { return len(x) }
func (x byLength) Less(i, j int) bool { return x[i].Length < x[j].Length }
func (x byLength) Swap(i, j int)      { x[i], x[j] = x[j], x[i] }


type customSort struct {
    t []*Track
    less func(x, y *Track) bool
}

func (x customSort) Len() int           { return len(x.t) }
func (x customSort) Less(i, j int) bool { return x.less(x.t[i], x.t[j]) }
func (x customSort) Swap(i, j int)      { x.t[i], x.t[j] = x.t[j], x.t[i] }


func customLess(x, y *Track) bool {
    if x.Year != y.Year {
        return x.Year < y.Year
    }

    if x.Title != y.Title {
        return x.Title < y.Title
    }

    if x.Length != y.Length {
        return x.Length < y.Length
    }

    return false
}


func main() {

    printTracks(tracks)
    sort.Sort(byTitle(tracks))
    printTracks(tracks)
    sort.Sort(byYear(tracks))
    printTracks(tracks)
    sort.Sort(byLength(tracks))
    printTracks(tracks)

    sort.Sort(customSort{tracks, customLess})
    printTracks(tracks)
    

    values := []int{3, 1, 4, 1}
    fmt.Println(sort.IntsAreSorted(values)) // "false"
    sort.Ints(values)
    fmt.Println(values)                     // "[1 1 3 4]"
    fmt.Println(sort.IntsAreSorted(values)) // "true"
    sort.Sort(sort.Reverse(sort.IntSlice(values)))
    fmt.Println(values)                     // "[4 3 1 1]"
    fmt.Println(sort.IntsAreSorted(values)) // "false"
}
