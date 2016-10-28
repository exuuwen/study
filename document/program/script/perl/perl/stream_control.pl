#!/usr/bin/perl  -w
print "input a num small than 10:";
$num = <STDIN>;

if($num eq "\n")
{
	print "it is a blank input \n";
}
else
{
	chomp($num);
	print "input is : $num\n";
}


while($num < 10)
{
	$num += 1;
	print "now num is:$num\n" ;
}
 
print "undef is:$undef\n"; #will be print warning 

if(defined($undef))
{
	print "the data is defineds is: $undef\n";
}
else
{
	print "the data is not define\n";
}
	
