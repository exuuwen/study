#!/usr/bin/perl   -w
$family_hash{"asha"} = "father";
$family_hash{"buble"} = "father";
$family_hash{"camen"} = "son";
foreach $person (qw<asha buble camen>){
       print "I’ve heard of $person is $family_hash{$person}.\n";                                                       
}

 %last_name = (
   "fred" => "flintstone",
   "dino" => "wu",
   "barney" =>"rubble" ,
   "betty" => "rubble" ,
);
@name = %last_name;
print "@name\n";


%hash = ("a"=>1, "b"=>2, "c"=>3);
@k = keys %hash;
@v = values %hash;
$numk = keys %hash;
$numv = values %hash;
print "hash numk is:$numk,numv is:$numv,values is:@v,keys is @k\n";

if(%hash)
{
   print "That was a true value!\n";
}

while (($key, $value) = each %hash){
   print "$key => $value\n" ;
}

$dino = "a";
if(exists $hash{$dino}){
   print "Hey, there’s a key is \"a\" !\n";
}

$person = "a";
  delete $hash{$person};

while (($key, $value) = each %hash){
   print "$key => $value\n" ;
}


