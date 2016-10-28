#!/usr/bin/perl  -w

$_ = "wasd fred sasddd";
if(s/fred/wx/)
{
	print "$_\n"; 
}

if(s/(\w+) (\w+)/$2,$1/)
{
	print "$_\n"; 
}

$_ = "wasd fred wasd";
if(s/wasd/wx/g)# /g for all the wasd
{
	print "$_\n"; 
}

$_ = "I saw Barney witH Fred.";
if(s/(\w+) witH (\w+)/\U$2\E \uwit\lh \L$1/)
{
	print "$_\n"; 
}
$string = "::a:b:c:d:::";
@sp = split /:/, $string;#("","",a,b,c,d)
print "@sp\n";


@field = ("a","b","cc","da");
$connect = ":";
$jo = join $connect, @field;
print "$jo\n";


$_ = "hh aa: sd, wws xxs ad";
@get = /(\w+)/g;
print "@get\n";

$_ = "hh aa: sd,";
@get = /(\S+) (\S+) (\S+)/;
print "@get\n";


$_ = "haa ssx \nhaha dds";
if(/^haha\b/m)
{
	print "find \"hah\" in the syntax\n";
}
