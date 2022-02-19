#!/bin/bash

# $1 is the key, $2 is in so it is ignored, $3 is the asocciative array
function exists ()
{
    eval '[ ${'$3'[$1]+1} ]'
}

# amount of arguments error handler
if [[ $# -lt 3 ]] || [[ $# -gt 3 ]]; then
    echo "Wrong amount of arguments given, must be three."
    echo "Usage: ./create_infiles.sh inputFile input_dir numFilesPerDirectory"
    printf "Aborting"; sleep 0.1; printf "."; sleep 0.1; printf "."; sleep 0.1; echo "."
    exit
fi

# checking if the first argument is a file
if [[ ! -f $1 ]]; then
    echo "First argument must be a file."
    printf "Aborting"; sleep 0.1; printf "."; sleep 0.1; printf "."; sleep 0.1; echo "."
    exit
fi

# checking if the given input directory exists
if [[ -d ./$2 ]]; then
    echo "Input directory with name : '$2' exists."
    printf "Aborting"; sleep 0.1; printf "."; sleep 0.1; printf "."; sleep 0.1; echo "."
    exit
fi

# checking for negative files in directories given
if [[ $3 -lt 1 ]]; then
    echo "Desired files in each direcotry must be greater than zero."
    printf "Aborting"; sleep 0.1; printf "."; sleep 0.1; printf "."; sleep 0.1; echo "."
    exit
fi

# creating the input directory in the project main directory
mkdir $2

# countries found in input file given
declare -A countries
declare -A persons
declare -A lastKnowWriteSpot

# read file line by line and create the needed subdirecotries and files
while read line; do
    # split the line into a string array with maximum 8 elements (this comes from the input format)
    stringarray=($line)

    # check if the country in this line exists already, country is always in the 3rd cell of the array
    # if the country does not exist then create a directory for it and the needed files inside
    if ! exists "${stringarray[3]}" in countries; then

        # country found one time, ignore the others
        countries["${stringarray[3]}"]=1

        # init file manager counter
        lastKnowWriteSpot["${stringarray[3]}"]=1

        # create the subdirectory
        mkdir ./$2/${stringarray[3]}

        # create the numFilesPerDirectory in the country directory
        for (( i=1; i<=$3; i++ )) do

            # build the file name
            filename="${stringarray[3]}-$i"

            # create empty file with the above file name
            touch ./$2/${stringarray[3]}/$filename.txt
        done
    fi

    # write the data in the correct file
    # find the country
    cntr=${stringarray[3]}

    # find the correct file of the country
    num=${lastKnowWriteSpot[$cntr]}

    # create path name
    pathname="$cntr-$num"

    # write the data from the array
    echo "${stringarray[@]}" >> ./$2/$cntr/$pathname.txt

    # update the file manager
    lastKnowWriteSpot[$cntr]=$(( ${lastKnowWriteSpot[$cntr]} + 1 ))

    # if the file manager counter is greater than the number of files reset it to 1
    if [[ ${lastKnowWriteSpot[$cntr]} -gt $3 ]]; then
        lastKnowWriteSpot[$cntr]=1
    fi

done < $1
