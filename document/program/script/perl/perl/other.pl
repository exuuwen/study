#!/usr/bin/perl   -w
use File::Basename;#one
#use File::Basename qw/ basename / #two
#use File::Basename () #three
$name = "/usr/sbin/perl";
print "\$name:$name\n";
$basename = basename $name;#调用库函数
#$basename = File::Basename::basename $name;#three
print "\$basename:$basename\n";

$filename = "test.pl";
print "the $filename is exists\n" if -e $filename;

if( -r $filename and -w _)
{
	print "the $filename  can r & w\n";
}

if( -r -w  $filename )
{
	print "2 the $filename  can r & w\n";
}

@state = stat($filename);
print "state:@state\n";

open OP,"<test.pl";
$line = readline OP;
print "\$line:$line";

if(chdir ".")
{
	@all_file = <*>;#glob "*";
	print "file:@all_file\n";
}
$dir_path = "/home/cr7/perl";
opendir DH, $dir_path or die "can not open $dir_path\n";
@dir = readdir DH;
print "dir file:@dir\n";
closedir DH;


$big = "wen xu is here sha";
$small = "is";
$pos = index($big, $small);#the same as rindex
print "the pos:$pos\n";
$pos = index($big, $small,3);
print "the pos:$pos\n";
$pos = index($big, $small, 8);
print "the pos:$pos\n";

$str = substr($big, 5);
print "str:$str\n";
$str = substr($big, 3, 5);
print "str:$str\n";
$str = substr($big, 0, 200);
print "str:$str\n";
$str = substr($big, -3, 3);#from back
print "str:$str\n";

@input = (5, 3, 6,2,7);
sub by_num #2个参数的排序程序
{
	$a <=> $b;#比较2个数返回－1，0，1
}
@array = sort by_num @input;
print "@array\n";
@input = ("dm", "as", "ac", "da", "ae");
sub by_assic
{
	$a cmp $b;
}
@array = sort by_assic @input;
print "@array\n";

sub by_score 
{
	$score{$a} <=> $score{$b}
	or
	$a cmp $b; 
}
%score = ("s" => 9, "a" => 8, "f" => 7, "m" => 8);
@array = sort by_score keys %score;
print "@array\n";

use 5.010;
$condition = 0;
given( $condition ) {
	when($_ == 0) {print "it is 0\n"; continue}
	when($_ > -1)  {print "it is > -1\n"}
	when($_ < -1)  {print "it is < -1\n"}
	#default       {print "it isxxnot a num\n"}#why
}
print "\n";
$n = 1;
@num = (8,2, -1, -3,7);
foreach (@num)
{
	when($_ == 0) {print "$n is 0\n";$n++;}
	when($_ > -1)  {print "$n is > -1\n";$n++;}
	when($_ < -1)  {print "$n is < -1\n";$n++;}
	default       {print "$n isxxnot a num\n"; $n++}#why
	
}

