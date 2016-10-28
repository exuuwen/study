function errtrap {

    es=$?

    echo "ERROR line $1: Command exited with status $es."

}

function bad {

    return 17
}


trap "echo 'the shell exit'"  EXIT
trap 'errtrap $LINENO' ERR


read data
if [ $data -lt 10 ]
then
	echo "ok data is $data"
	exit
fi

bad




