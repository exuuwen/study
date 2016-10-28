#!/usr/bin/perl  -w

$_ = "sxsa abab xsod";
if(/abab/)
{
	print "\"abab\" is matched with $_\n";
}

$_ = "yabba xsod";
if(/y(.)(.)\2\1/) #\2 is for second (.), \1 is for first (.)
{
	print "\"abab\" is matched with $_\n";
}

$_ = Yes;
if(/yes/i) {#大小写无关
print "In that case, I recommend that you go bowling.\n";
}

$_ = "I saw Barney\ndown at the bowing alley\nwith Fred\nlast night.\n";
if(/Barney.*Fred/s){
print "That string mentions Fred after Barney:$_\n";
}


$some_other = "I dream of betty rubble.";#defaut is for $_
if($some_other =~ /\brub/){
print "Aye, there’s a rub.\n";
}

$_ = "Hello there, neighbor";
if(/\s(\w+), (\w+)/){ #空格和逗号之间的词
print "the word was:$1:$2\n"; #the word was there
}

$_ = "Hello there, neighbor";
if(/\s(?:\w+), (\w+)/){ #空格和逗号之间的词,  跳过（不捕获）第一个 
print "the word was:$1\n"; #the word was there
}

 if ("Hello there, neighbor"=~ /\S(\w+),/){
print "That was ($`)($&)($')\n";
}

$names = "fred and joye";
if( $names =~ /(?<name2>\w+) (and|or) (?<name1>\w+)/ )
{
	print "I saw $+{name1} and $+{name2}\n";
}
