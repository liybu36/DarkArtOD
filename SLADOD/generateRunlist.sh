#!/bin/bash

basedir="gsiftp://fndca4a.fnal.gov:2811/raw/"
searchdir="/lsv/*/*"

# Generate the name of the runlist to be produced
scriptdir="/ds50/app/user/shawest/NeutronVetoAnalysis/DarkartScripts/"
dir="${scriptdir}Runlists/"
prefix="AutoGenRunlist"
suffix=".txt"
date=$(eval date +%d%b%Y)
outlist="${dir}${prefix}_${date}${suffix}"
# Check if a runlist with this name already exists
counter=0
while [ -e $outlist ]; do
    counter=$[$counter+1]
    outlist="${dir}${prefix}`printf %03d ${counter}`_${date}${suffix}"
done
echo "Creating runlist $outlist"

listOfRuns=""
for phase in "commissioning" "calibration"; do
    echo "Checking $phase"
    runs=$(uberftp -ls $basedir$phase$searchdir | grep 001.dat | sed 's/.*ODRun//' | sed 's/_.*//' | sed 's/^0*//')

    # Check if each run is a past runlist
    for run in $runs; do
	check=""
	check=$(eval grep "^${run}$" ${scriptdir}Runlists/*.txt)

	if [ -z "$check" ]; then
	    if [ -z $listOfRuns ]; then
		listOfRuns="${run}"
	    else
		listOfRuns="${listOfRuns}~${run}"
	    fi
	fi
    done
done
# Sort by run number and create output file
echo "Sorting runlist..."
echo $listOfRuns | tr "~" "\n" | sort -g > $outlist
echo "Done!"