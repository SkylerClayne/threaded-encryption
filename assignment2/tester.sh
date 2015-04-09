

1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
17
18
19
20
21
22
23
24
25
26
27
28
29
30
31
32
33
34
35
36
37
38
39
40
41
42
43
44
45
46
47
48
49
50
51
52
53
54
55
56
57
58
59
60
61
62
63
64
65
66
67
68
69
70
71
72
73
74
75
76
77
78
79
80
81
82
83
84
85
86
87
88
89
90
91
92
93

#!/bin/bash

#Assignment Tester
#To run:
#
#chmod +x testall.sh
#./testall


#EDIT THIS PART TO MATCH YOUR FILES
sourceFile="encrypt.c" 

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

