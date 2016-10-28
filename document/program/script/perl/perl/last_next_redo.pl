#!/usr/bin/perl  -w
for(1..5)
{
	print "the num is $_\n";
	print "please input choice:last, next,redo or none of above.\n";  	
	$choice = <STDIN>;
	chomp($choice);
	last if $choice =~ /last/i;
	next if $choice =~ /next/i;
	redo if $choice =~ /redo/i;
	print "do not have any choice\n";
}
