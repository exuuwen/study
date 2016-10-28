#!/usr/bin/perl  -w
#$fred[0] = "one";
#$fred[1] = "two";
#$fred[2] = "three";
#$fred[3] = "four";
#$fred[99] = "ninety_nine";
@fred = qw(one two three four);
$fred[99] = "ninety_nine";
$num = 2;
print "fred $num is:$fred[$num]\n";
print "$fred[$#fred],$fred[-1]\n";


@data = (1..5);
$end = $#data;
$pos = 0;
while($pos <= $end)
{
	print "data[$pos] is:$data[$pos]\n";
	$pos += 1;
}	

@all = (@fred, @data, "last");
$end = $#all;
$pos = 0;
while($pos <= $end)
{
	if(defined($all[$pos]))
	{
		print "all[$pos] is:$all[$pos]\n";
	}	

	
	$pos += 1;
}



$temp = pop(@data);
print "pop temp is $temp\n";
push(@data, 100);
push(@data, 200);
$end = $#data;
$pos = 0;
while($pos <= $end)
{
	print "data[$pos] is:$data[$pos]\n";
	$pos += 1;
}	

$temp = shift(@data);
print "shift temp is $temp\n";
unshift(@data, 10);
unshift(@data, 20);
$end = $#data;
$pos = 0;
while($pos <= $end)
{
	print "data[$pos] is:$data[$pos]\n";
	$pos += 1;
}	

@re = reverse(1..5);
print "reverses is :@re\n";

@sr = sort(5,2,3,2,5,6,4,1);
print "sort is : @sr\n";
print "sort is : ".scalar @sr."\n";#force biaoliang

foreach $temp (@data)
{
	$temp = "\t$temp";
	$temp.="\n";
}

print "data is : @data";

@input = <STDIN>;
chomp(@input);
print "input is :@input\n";



