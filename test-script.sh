
#!/bin/bash

#Assignment Tester
#To run:
#
#chmod +x testall.sh
#./testall


#EDIT THIS PART TO MATCH YOUR FILES
sourceFile="assignment2/encrypt.c" 

#run a test 
#PARAMS: <nIn> <nWORK> <nOUT> <key> <bufSize> <fileToTest> <testname> <testNum>
function runTest
{
	./encrypt $4 $1 $2 $3 $6 "test$8.enc" $5
	if [ "$?" -eq "0" ]
		then
		echo -e "Running decryption\n"
		./encrypt -$4 $1 $2 $3 "test$8.enc" "test$8.dec" $5
		if [ "$?" -eq "0" ]
			then
			echo -e "Checking result:\n"
			diff -a  $6 "test$8.dec"
			if [ "$?" -eq "0" ]
				then
				echo -e "$7 ---Success..\n"
				rm "test$8.enc" "test$8.dec"
				return 2
			fi
		fi
	fi
	return 1
}

function printResult
{
	echo "Number of tests passed: $1/$2"
}

function main
{
		numTests=0
		success=0

	for i in $@
	do

		runTest 1 1 1 1 1 $i $i 1
		if [ "$?" -eq "2" ]
			then 
			let "success++"
		fi
		let "numTests++"

		runTest 1 1 1 1 100 $i $i 2
		if [ "$?" -eq "2" ]
			then 
			let "success++"
		fi
		let "numTests++"
		runTest 10 10 10 1 1 $i $i 3
		if [ "$?" -eq "2" ]
			then 
			let "success++"
		fi
		let "numTests++"
		runTest 10 10 10 1 100 $i $i 4
		if [ "$?" -eq "2" ]
			then 
			let "success++"
		fi
		let "numTests++"
	done

	printResult $success $numTests
	if [ $success -eq $numTests ]
		then
		echo "All the success in your life!!!!"
	fi
}

#Compile the program
#Add any special flags here
cc  -Wall $sourceFile -o encrypt -lpthread 
if [ "$?" -eq "0" ]
	then
	main $@
else
	echo "Compilation failed"
	exit -1
fi

