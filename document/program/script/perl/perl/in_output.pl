#!/usr/bin/perl   -w
print "program name is $0\n";
print "argument is @ARGV\n";
while (defined($line = <>)){ #input from arg,defaut from stdin if there is no arg
  chomp($line);
  print "it is $line I see!\n";                    
}

$r_suc=open READ, "<read";
if(!$r_suc)
{
	die "can not open read\n";
}
$w_suc=open WRITE, ">write";#>> is add after
if(!$w_suc)
{
	die "can not open write\n";
}
$num = 1;
while(defined($line=<READ>))
{
	chomp($line);
	print WRITE "$num:$line\n";
	$num++;
}
close READ;
select WRITE;
print "select function write into write file\n";
select STDOUT;
print "select function write into console\n";
close WRITE;
#print <> #'cat'source code 
