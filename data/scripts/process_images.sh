#!/bin/sh

# usage : <first-frame> <last-frame> <offset-out> <command> <in-file-pattern> <out-file-pattern>
#
# Author: croussil

for ((i = $1; $i <= $2; i++)); do
	echo "$4 `printf ""$5"" $i` `printf ""$6"" $(($i+$3))`"
	$4 `printf "$5" $i` `printf "$6" $(($i+$3))`;
done
