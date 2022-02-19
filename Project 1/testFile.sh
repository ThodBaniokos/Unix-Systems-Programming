#!/bin/bash
#functions 
# $1 is the key, $2 is in so it is ignored, $3 is the asocciative array
function exists ()
{
    eval '[ ${'$3'[$1]+1} ]'
}

# amount of arguments error handler
if [[ $# -lt 4 ]] || [[ $# -gt 4 ]]; then
    echo "Wrong amount of arguments given, must be four."
    echo "Usage: ./testFile.sh virusesFile countriesFile numLines duplicatesAllowed"
    printf "Aborting"; sleep 0.1; printf "."; sleep 0.1; printf "."; sleep 0.1; echo "."
    exit
fi

# checking if the first two arguments are files
if [[ ! -f $1 ]] || [[ ! -f $2 ]]; then
    echo "First two arguments must be files."
    echo "First one is the viruses file, second one is the countries file."
    printf "Aborting"; sleep 0.1; printf "."; sleep 0.1; printf "."; sleep 0.1; echo "."
    exit
fi

# checking for negative lines given
if [[ $3 -lt 1 ]]; then
    echo "Desired lines of the generated file must be greater than zero."
    printf "Aborting"; sleep 0.1; printf "."; sleep 0.1; printf "."; sleep 0.1; echo "."
    exit
fi

# checking if the flag is not zero or one
if [[ $4 -lt 0 ]] || [[ $4 -gt 1 ]]; then
    echo "Duplicate records allowed flag must be zero or one."
    printf "Aborting"; sleep 0.1; printf "."; sleep 0.1; printf "."; sleep 0.1; echo "."
    exit
fi

# old value of ifs variable
OLDIFS=$IFS

# reading virus file line by line
IFS=$'\n' viruses=($(cat "$1"))
virusCount=${#viruses[@]}

# reding countries file line by line
IFS=$'\n' countries=($(cat "$2"))
countryCount=${#countries[@]}

# resotring ifs value
IFS=$OLDIFS

# duplicate id array
declare -A duplicateID

# records array
declare -a records

# person info array
declare -a persons
personsCounter=$((0))

# generating the input file, $3 is the given lines of the input files
for ((i=0; i<$3; i++))
do

    # add the id first
    if [[ $4 -eq 0 ]]; then
        # if the duplicates flag is zero, duplicates are not allowed, create unique id's
        while : ; do
            id=$(( $(( RANDOM % 9999 )) + 1 ))
            if ! exists "$id" in duplicateID; then
                duplicateID["$id"]=1
                break
            fi
        done
    else
        # if not then just create an id
        id=$(( $(( RANDOM % 9999 )) + 1 ))
    fi

    # create the full name of the person
    fullname=$( cat /dev/urandom | tr -dc A-Za-z | head -c $(( 2 * $(( RANDOM % 10)) + 4)) )
    
    # add country
    country=${countries[$(( RANDOM % countryCount ))]}

    # add age
    age=$(( RANDOM % 121 + 1 ))

    # add virus
    virus=${viruses[$(( RANDOM % virusCount ))]}

    # add if the person is vaccinated or not, and if yes add the date of vaccination
    if [[ $(( RANDOM % 11 )) -gt 4 ]]; then
        answer="YES $(( RANDOM % 30 + 1 ))-$(( RANDOM % 12 + 1 ))-$(( $(( RANDOM % 121 )) + 1901 ))"
    else
        answer="NO"
    fi

    # if the duplicates flag is 1 and the persons array has one inside and the random function returns
    # something greater than five then create a record with an existing person
    if [[ $4 -eq 1 ]] && [[ $(( RANDOM % 11 )) -gt 5 ]] && [[ ${#persons[@]} -gt 0 ]]; then
        records+=("${persons[$(($RANDOM % $personsCounter))]} $virus $answer")
        continue
    fi

    # split the full name in half, for first and last name
    name=${fullname:0:${#fullname}/2}
    surname=${fullname:${#fullname}/2}

    # add the person info in the right array
    persons+=("$id $name $surname $country $age")
    personsCounter=$(( $personsCounter + 1 ))

    # the the record in the records array
    records+=("$id $name $surname $country $age $virus $answer")

done

# write the whole records array in the appropriate file
printf "%s\n" "${records[@]}" > ./input/citizenRecordsFile.txt