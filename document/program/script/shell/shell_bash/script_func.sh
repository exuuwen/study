function afunc
{
  #local var1 #if no this  is diffrent

  echo in function: $0 $1 $2

  var1="in function"

  echo var1: $var1
}

var1="outside function"

echo var1: $var1

echo $0: $1 $2

afunc funcarg1 funcarg2


(
	var1="in the sub shell"
	echo var1: $var1
)

echo var1: $var1

echo $0: $1 $2
