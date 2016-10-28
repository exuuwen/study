#!/usr/bin/perl  -w
$_ = " I am wx";
unless(/wx/)
{
	print  "there is no wx\n";
}
else
{
	print "wx is in the \$_\n";
}

print "wx is in the \$_\n" if /wx/;

$n = 0;
until($n > 5)
{
	print "\$n is:$n\n";
	$n++;
}

$n = 0;
if($n > 0)
{
	print "\$n is bigger than 0\n";
}
elsif($n < 0)
{
	print "\$n is smaller than 0\n";
}
else 
{
	print "elsif \$n is 0\n";
}

for($i = 0; $i < 3; $i++)
{
	print "for:\$i is:$i\n"
}

for(1..10)
{
	print "$_\t"
}
print "\n";

use 5.010;
$my_name = $undefine // 'default';
print "$my_name\n";
