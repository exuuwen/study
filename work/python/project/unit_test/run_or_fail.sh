# helper method for providing error messages for a command
run_or_fail() {
	local explanation=$1
	shift 1
	"$@"
	
	if [ $? != 0 ]; then
		echo $explanation
		exit 1
	fi
}



# source run_or_fail.sh 
# m=$(run_or_fail "fail" ls m 2> /dev/null )
# echo $?
# echo $m

# run_or_fail "fail" ls m 2> /dev/null

