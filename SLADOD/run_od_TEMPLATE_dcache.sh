#!/bin/bash
#############################################################
## Script to start parallel condor jobs for darkart runs   ##
## Compatible with darkart version: v02_01_03              ##
## Created on 2015-03-31 by Shawn Westerdale               ##
##                                                         ##
## Must pass a fhiclFile, an SPE file, and a prompt offset ##
##  Optional parameter: which subrun to process            ##
#############################################################

# Take arguments
# FHiCL file to be used for run
fhiclFile=$1
# SPE File to use
speFile=$2
# Prompt Offset to use
promptOffset=$3
# List of channels to disable
disabledChans="$4"
# If run is being split into parts, second argument is the SUBRUN
SUBRUN=0
if [ $# -eq 5 ] ;  then
    SUBRUN=`printf %03d $5`
fi

notification=never
export DS50app=/ds50/app
export DS50data=/ds50/data
echo "PWD is $PWD"
OLDPWD=$PWD

# Establish external products
source ${DS50app}/products/setup
export PRODUCTS=${DS50app}/products:/grid/fermiapp/products/common/db
setup ifdhc

# set the terrifying library path
export LD_LIBRARY_PATH=/grid/fermiapp/products/common/prd/jobsub_tools/v1_2x/Linux-2/lib/:/ds50/app/user/shawest/work/build/darkart-production/lib:/ds50/app/products/caenvme/v2_30_2/Linux64bit+2/lib:/ds50/app/products/caencomm/v1_02/Linux64bit+2/lib:/ds50/app/products/caendigitizer/v2_2_1/Linux64bit+2/lib:/ds50/app/products/postgresql/v9_1_5b/Linux64bit+2.6-2.12/lib:/ds50/app/products/pqxx/v4_0_1b/Linux64bit+2.6-2.12-e4-prof/lib:/ds50/app/products/TRACE/v3_03_02/Linux64bit+2.6-2.12/lib:/ds50/app/products/xmlrpc_c/v1_25_26/Linux64bit+2.6-2.12-e4-prof/lib:/ds50/app/products/mpich/v3_0_4/Linux64bit+2.6-2.12-e4-prof/lib:/ds50/app/products/tbb/v4_1_3/Linux64bit+2.6-2.12-e4-prof/lib:/ds50/app/products/cppunit/v1_12_1/Linux64bit+2.6-2.12-e4-prof/lib:/ds50/app/products/clhep/v2_1_3_1/Linux64bit+2.6-2.12-e4-prof/lib:/ds50/app/products/sqlite/v3_08_00_02/Linux64bit+2.6-2.12/lib:/ds50/app/products/python/v2_7_5c/Linux64bit+2.6-2.12/lib:/ds50/app/products/xrootd/v3_3_4/Linux64bit+2.6-2.12-e4-prof/lib64:/ds50/app/products/libxml2/v2_9_1/Linux64bit+2.6-2.12-prof/lib:/ds50/app/products/fftw/v3_3_3/Linux64bit+2.6-2.12-prof/lib:/ds50/app/products/root/v5_34_21b/Linux64bit+2.6-2.12-e4-prof/lib:/ds50/app/products/gcc/v4_9_1/Linux64bit+2.6-2.12/lib64:/ds50/app/products/gcc/v4_9_1/Linux64bit+2.6-2.12/lib:/ds50/app/products/boost/v1_53_0/Linux64bit+2.6-2.12-e4-prof/lib:/ds50/app/products/cpp0x/v1_03_25/slf6.x86_64.e4.prof/lib:/ds50/app/products/cetlib/v1_03_25/slf6.x86_64.e4.prof/lib:/ds50/app/products/fhiclcpp/v2_17_12/slf6.x86_64.e4.prof/lib:/ds50/app/products/messagefacility/v1_10_26/slf6.x86_64.e4.prof/lib:/ds50/app/products/art/v1_08_10/slf6.x86_64.e4.prof/lib:/ds50/app/products/artdaq/v1_05_04/slf6.x86_64.e4.eth.prof/lib:/ds50/app/products/ds50daq/v1_02_01/slf6.x86_64.e4.eth.prof/lib:/grid/fermiapp/products/common/prd/git/v1_8_0_1/Linux64bit-2/lib

# Setup art path stuff
export ART_WORKBOOK_OUTPUT_BASE=/ds50/data/user
export ART_WORKBOOK_WORKING_BASE=/ds50/app/user
export ART_WORKBOOK_QUAL="e2"

# Set the path to find scripts here.
PATH=`dropit -se "/ds50/app/ds50"`

echo "x509 proxy =  $X509_USER_PROXY"
#export TRUEUSER=`basename $X509_USER_PROXY | cut -d "." -f 1`
export TRUEUSER=$GRID_USER
echo "The true user is $TRUEUSER "

export LOCAL_PRODUCTS=${DS50app}/user/${TRUEUSER}/work/local-products



export PRODUCTS=`dropit -sfe -p "${PRODUCTS}" "/grid/fermiapp/products/common/db" "${DS50app}/products"  "${LOCAL_PRODUCTS}" `
export CETPKG_J=4
export CETPKG_INSTALL=${LOCAL_PRODUCTS}
export CETPKG_SOURCE=/ds50/app/user/shawest/work/darkart-production
export CETPKG_BUILD=/ds50/app/user/shawest/work/build/darkart-production
export CETPKG_NAME=darkart
export CETPKG_VERSION=v1_01_04
#export CETPKG_QUAL=e4:eth:prof
export CETPKG_QUAL=e6:prof
export CETPKG_TYPE=Prof

. ${CETPKG_SOURCE}/ups/setup_for_development -p 
MY_FILE_PATH=${CETPKG_SOURCE}

export FHICL_FILE_PATH=.
export FHICL_FILE_PATH=`dropit -sfe -p "$FHICL_FILE_PATH" "$MY_FILE_PATH/fcl" "."`

cd $_CONDOR_SCRATCH_DIR

echo "working directory now $PWD"
echo "products:"
echo $PRODUCTS
echo "path:"
echo $PATH

# Transfer raw data file to the worker node
export IFDH_GRIDFTP_EXTRA='-stall-timeout 3600'
# Set the location of the raw data
RUNNUM=TEMPLATE
FULLRUNNUM=`printf %06d $RUNNUM`
echo "RUNNUM = $RUNNUM"
echo "FULLRUNNUM = $FULLRUNNUM"
RUNAB=${FULLRUNNUM:0:2}
RUNABCDE=${FULLRUNNUM:0:5}
phase=PHASE
LSVPATH="gsiftp://fndca1.fnal.gov:2811/raw/${phase}/lsv"
FILEPATH=""
if [ $RUNNUM -lt 1000 ]; then
    FILEPATH="ODRun${FULLRUNNUM%??}xx"
else
    FILEPATH="ODRun${RUNAB}xxxx/ODRun${RUNABCDE}x"
fi
echo "FILEPATH = $FILEPATH"

FILENAME=''
if [ $SUBRUN -eq 0 ]; then
    FILENAME="ODRun${FULLRUNNUM}_001.dat"
    ifdh cp -D --force=gridftp ${LSVPATH}/${FILEPATH}/${FILENAME} ./
else
    FILENAME="ODRun${FULLRUNNUM}_${SUBRUN}.dat"
    # Important run-level data is in the first SUBRUN, so copy that too
    echo "ifdh cp -D --force=gridftp ${LSVPATH}/${FILEPATH}/${FILENAME} ./"
    ifdh cp -D --force=gridftp ${LSVPATH}/${FILEPATH}/ODRun${FULLRUNNUM}_001.dat ./
    ifdh cp -D --force=gridftp ${LSVPATH}/${FILEPATH}/${FILENAME} ./
fi

#copy the spe files as needed
ifdh cp -r -D /ds50/app/user/shawest/work/darkart-production/aux/spe_OD $PWD/

#copy over the FHiCL file and edit
umask 0002

echo "ifdh cp -D --force=gridftp /ds50/app/user/shawest/work/darkart-production/fcl/${fhiclFile} $PWD/"
ifdh cp -D /ds50/app/user/shawest/work/darkart-production/fcl/${fhiclFile} $PWD/
sed -i "s/RUNTEMP/${FILENAME}/g" $fhiclFile
sed -i "s/PROMPTTEMP/${promptOffset}/g" $fhiclFile
sed -i "s/DISABLEDCHANTEMP/${disabledChans}/g" $fhiclFile
sed -i "s@SPETEMP@${speFile}@g" $fhiclFile

echo "Disabling channels ${disabledChans}"

echo "Files in current directory:"
ls -l .

echo "Using FHiCL file ${fhiclFile}"
cat $fhiclFile

echo "Copied the following files for processing:"
ls ODRun${FULLRUNNUM}_*.dat

# Run the DarkArtOD job
/ds50/app/products/art/v1_08_10/slf6.x86_64.e4.prof/bin/art -c $fhiclFile

# Transfer the output back to ds50
umask 0002
DARKARTFILENAME="darkart_od_output.root"
if [ $SUBRUN -eq 0 ]; then
    mv $DARKARTFILENAME $CONDOR_DIR_MYOUTPUT/ODRun${FULLRUNNUM}.root
else
    mv $DARKARTFILENAME $CONDOR_DIR_MYOUTPUT/ODRun${FULLRUNNUM}_${SUBRUN}.root 
fi