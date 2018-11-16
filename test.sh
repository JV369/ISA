#!/bin/sh
POSIXLY_CORRECT=yes


testNum=1

RED=`tput setaf 1` 
GREEN=`tput setaf 2`
NC=`tput sgr0`
for input in ./test/*/*.in; do

	input=$(echo $input | sed 's/.in$//')
	inText=$(cat $(echo $input.in))

	echo "-------------------------------------"
	echo "Test number: $testNum"
	echo "File used: $input"
	output=$(exec $inText)
	retCode=$(echo $?)

	if [ "$retCode" = "$(cat $(echo $input.ret))" ]; then
		echo "Return code: ${GREEN}OK${NC}"
	else
		echo "Return code: ${RED}FAIL${NC}"
	fi

	if [ -f $(echo $input.out) ]; then
		outRcv=$(cat $(echo $input.out))
		if [ "$outRcv" = "$output" ]; then
			echo "Output compare: ${GREEN}OK${NC}"
		else
			echo "Output compare: ${RED}FAIL${NC}"
		fi
	fi
	testNum=$(($testNum + 1))
done
