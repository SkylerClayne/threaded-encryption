#!/bin/bash

#Assignment Tester
#To run:
#
#chmod +x testall.sh
#./testall

#testall.sh (Compiles, many test cases can be included)

#EDIT THIS PART TO MATCH YOUR FILES
sourceFile="encrypt.c" 
smallFile="from.txt"
bigFile="from.txt"



#run a test 
#PARAMS: <nIn> <nWORK> <nOUT> <key> <bufSize> <fileToTest> <testname> <testNum>
function runTest
{
		#echo -e "Running Test $7...\n"
			./encrypt $4 $1 $2 $3 $6 "test$8.enc" $5
				if [ "$?" -eq "0" ]
							then
										#echo -e "Running decryption\n"
												./encrypt -$4 $1 $2 $3 "test$8.enc" "test$8.dec" $5
														if [ "$?" -eq "0" ]
																		then
																						#echo -e "Checking result:\n"
																									diff -a  $6 "test$8.dec"
																												if [ "$?" -eq "0" ]
																																	then
																																						echo -e "$7 ---Success..\n"
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


																																																					#HERE IS THE TEST PART, YOU CAN ADD AS MANY AS YOU LIKE

																																																						#runTest <nIN> <nWORK> <nOUT> <KEY> <bufSize> <fileToTest> <testname> <testNum>

																																																							#This part is editable
																																																								runTest 1 1 1 1 1 $smallFile "small_file_basic" 1
																																																									#This is NOT
																																																										if [ "$?" -eq "2" ]
																																																													then 
																																																																let "success++"
																																																																	fi
																																																																		let "numTests++"

																																																																			#------------------------------------------------
																																																																				runTest 1 1 1 1 100 $smallFile "small_file_large_buffer" 2
																																																																					if [ "$?" -eq "2" ]
																																																																								then 
																																																																											let "success++"
																																																																												fi
																																																																													let "numTests++"
																																																																														runTest 10 10 10 1 1 $smallFile "small_file_single_buffer_10_threads" 3
																																																																															if [ "$?" -eq "2" ]
																																																																																		then 
																																																																																					let "success++"
																																																																																						fi
																																																																																							let "numTests++"
																																																																																								runTest 10 10 10 1 100 $smallFile "small_file_large_buffer_10_threads" 4
																																																																																									if [ "$?" -eq "2" ]
																																																																																												then 
																																																																																															let "success++"
																																																																																																fi
																																																																																																	let "numTests++"
																																																																																																		runTest 1 1 1 1 200 $bigFile "big_file_large_buffer" 5
																																																																																																			if [ "$?" -eq "2" ]
																																																																																																						then 
																																																																																																									let "success++"
																																																																																																										fi
																																																																																																											let "numTests++"
																																																																																																												runTest 10 10 10 1 10 $bigFile "big_file_buffer_10_threads_10_each" 6
																																																																																																													if [ "$?" -eq "2" ]
																																																																																																																then 
																																																																																																																			let "success++"
																																																																																																																				fi
																																																																																																																					let "numTests++"
																																																																																																																						runTest 100 5 5 1 100 $bigFile "big_file_big_buffer_100In_5_work_5_out" 7
																																																																																																																							if [ "$?" -eq "2" ]
																																																																																																																										then 
																																																																																																																													let "success++"
																																																																																																																														fi
																																																																																																																															let "numTests++"
																																																																																																																																runTest 5 100 5 1 100 $bigFile "big_file_big_buffer_5In_100_work_5_out" 8
																																																																																																																																	if [ "$?" -eq "2" ]
																																																																																																																																				then 
																																																																																																																																							let "success++"
																																																																																																																																								fi
																																																																																																																																									let "numTests++"
																																																																																																																																										runTest 5 5 100 1 100 $bigFile "big_file_big_buffer_5In_5_work_100_out" 9
																																																																																																																																											if [ "$?" -eq "2" ]
																																																																																																																																														then 
																																																																																																																																																	let "success++"
																																																																																																																																																		fi
																																																																																																																																																			let "numTests++"
																																																																																																																																																			#	runTest 1000 1000 1000 1 1000 $bigFile "big_file_1000_all" 10
																																																																																																																																																			#	if [ "$?" -eq "2" ]
																																																																																																																																																			#		then 
																																																																																																																																																			#		let "success++"
																																																																																																																																																			#	fi
																																																																																																																																																			#	let "numTests++"

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
																																																																																																																																																																echo "Compilatin failed"
																																																																																																																																																																	exit -1
																																																																																																																																																																fi
