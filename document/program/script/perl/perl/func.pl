#!/usr/bin/perl  -w
$n = 0;

sub marine
{
	$n += 1;
	print "now \$n is:$n\n";
}

&marine;
&marine;
&marine;
&marine;

$one = 5;
$two = 7;
sub sum
{
	print "this is sum call \$one + \$two\n";
	$one + $two;
}



$sum = &sum;
print "\$one + \$two is:$sum\n";


sub max
{
	if(@_ != 2)
	{
		print "Warning! &max should have two argument\n";
		return ;
	}
	if($_[0] > $_[1])
	{
		$_[0];
	}
	else
	{
		$_[1];
	}
}

$max = &max(10, 12);
if(defined($max))
{
	print "12 && 10 \$max is:$max\n"; 
}

$max = &max(10, 12, 13); #it is wrong just 2 argument 
if(defined($max))
{
	print "12 && 10 && 13 \$max is:$max\n"; 
}

sub max_all
{
	my($max) = shift(@_);
	#while(@_ > 0)
	#{
	#	my($temp) = shift(@_);
	#	if($temp > $max)
	#	{
	#		$max = $temp;
	#	}
	#}
	foreach (@_)
	{
		if($_ > $max)
		{
			$max = $_;     
		}
	}
	$max
}

$max_all = &max_all(10, 12, 13, 11, 2, 7, 8); 
if(defined($max_all))
{
	print "12 && 10 && 13 \$max_all is:$max_all\n"; 
}


use 5.010;
sub marine_state
{
	state $n = 0;
	$n += 3;
	print "now \$n_tsate is:$n\n";
}

&marine_state;
&marine_state;
&marine_state;
&marine_state;

	
