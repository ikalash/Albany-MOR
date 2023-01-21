#!/bin/bash

#-------------------------------------------
#  
# Prototype script to checkout, compile 
# Albany 
# 
# This script is executed from run_master.sh
#
# BvBW  10/06/08
# AGS  04/09
#-------------------------------------------

#-------------------------------------------
# setup and housekeeping
#-------------------------------------------

if [ -a $NIGHTLYDIR/Albany ]; then \rm -rf $NIGHTLYDIR/Albany
fi

if [ -a $ALBOUTDIR ]; then \rm -rf $ALBOUTDIR
fi

cd $NIGHTLYDIR
mkdir $ALBOUTDIR

#-------------------------------------------
# git clone Albany
#-------------------------------------------

git clone software.sandia.gov:/space/git/Albany > $ALBOUTDIR/albany_checkout.out 2>&1
cd Albany
echo "Switching Albany to branch ", $ALBANY_BRANCH
git checkout $ALBANY_BRANCH

#-------------------------------------------
# cmake:  configure and make Albany 
#-------------------------------------------

cd $ALBDIR
rm -rf $ALBDIR/build
mkdir $ALBDIR/build
cd $ALBDIR/build

echo "    Starting Albany cmake" ; date

cp $SCRIPTDIR/do-cmake-albany .
source ./do-cmake-albany > $ALBOUTDIR/albany_cmake.out 2>&1

echo "    Finished Albany cmake, starting make" ; date

/usr/bin/make -j 8 Albany > $ALBOUTDIR/albany_make.out 2>&1
/usr/bin/make -j 2       >> $ALBOUTDIR/albany_make.out 2>&1
