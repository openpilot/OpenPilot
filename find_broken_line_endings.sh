#!/bin/sh

headers=$( find . -iname "*.h" )
cfiles=$( find . -iname "*.c" )
cppfiles=$( find . -iname "*.cpp" )
prifiles=$( find . -iname "*.pri" )
profiles=$( find . -iname "*.pro" )
makefiles=$( find . -iname "Makefile*" )

files="$headers
$cfiles
$cppfiles
$prifiles
$profiles
$makefiles"

error=0
unixt=0
dost=0
filest=0
unixf=0
dosf=0
errorlist=""
for file in $files; do

	dos=$( grep -cE "$" "$file" )
	unix=$( grep -cE "\n$" "$file" )

	filest=$(( filest+1 ))
	if [ $unix -eq 0 ]; then
		true
		echo $file is dos
		dost=$(( dost+1 ))
	elif [ $dos -eq 0 ]; then
		true
		echo $file is unix
		unixt=$(( unixt+1 ))
	elif [ $dos -gt $unix ]; then
		echo $file is a dosfile but has $unix broken endings
		error=$(( error+1 ))
		dosf=$(( dosf+1 ))
		errorlist="$errorlist
$file"
	else
		echo $file is a unixfile but has $dos broken endings
		error=$(( error+1 ))
		unixf=$(( unixf+1 ))
		errorlist="$errorlist
$file"
	fi
done
echo -----------------------------
echo summary: $filest files checked
echo -----------------------------
echo Unix line ending files: $unixt
echo DOS line ending files: $dost
echo Unix files tainted with DOS endings: $unixf
echo DOS files blessed with Unix endings: $dosf
echo Total files with errors: $error
echo List of broken files:
echo "$errorlist"
