#!/bin/bash

# First, make sure the environment is fully functional
if [  -f "/afs/fnal.gov/ups/etc/setups.sh" ]
then 
    . "/afs/fnal.gov/ups/etc/setups.sh"
    if ups exist login
    then
        setup login
    fi
fi
ENV=$HOME/.shrc
export ENV 
if [ "`basename $SHELL`" != ksh -a -r $ENV ]
then
    . $ENV
fi
PS1="<`hostname`> "; export PS1

# Setup the environemnt
source /ds50/app/ds50/setup_ds50
source /ds50/app/ds50/setup_ds50_build_e6
setup ifdhc -z /grid/fermiapp/products/common/db

# Renew Kerberos tickets

for f in $(ls -1 /tmp/krb5cc* 2> /dev/null); do
    OWNER=$(ls -l $f | awk '{print $3}')
    GROUP=$(ls -l $f | awk '{print $4}')
    
    if [ $OWNER == $USER ]; then
	echo "Renewing tickets for " $OWNER $GROUP $f
	kinit -l7d -R -c $f
	chown $OWNER:$GROUP $f
    fi
done

# Rerun jobsub_setup script
echo "Setting up jobsub environment..."
source /ds50/app/user/$USER/jobsub_setup.sh

# Make sure you are using the production version of DarkArtOD
echo "Setting DarkArtOD to produciton mode..."
SCRIPTDIR=/ds50/app/user/shawest/NeutronVetoAnalysis/DarkartScripts/
source /ds50/app/user/$USER/work/darkart-production/setup_ds50_build_production
echo "...recompiling..."
buildtool -j20
echo "In $PWD"
cd $SCRIPTDIR
echo "Moved to $PWD"

# Generate runlist
GENOUTPUT=$(. /ds50/app/user/shawest/NeutronVetoAnalysis/DarkartScripts/generateRunlist.sh)
RUNLIST=`echo -e $GENOUTPUT | awk '{sub(/.*runlist /,""); sub(/.txt.*/,".txt"); print}'`
DATE=$(eval date +%d%b%Y)
echo "$DATE : Submitting runlist $RUNLIST"

. /ds50/app/user/shawest/NeutronVetoAnalysis/DarkartScripts/batch_submit.sh $RUNLIST