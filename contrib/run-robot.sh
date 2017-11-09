#!/bin/sh

CfgFile=/www/search.sbs.de/test/conf/htfig.conf
BinDir=/www/search.sbs.de/test/bin
CgiBinDir=/www/search.sbs.de/test/cgi-bin
DataDir=/www/search.sbs.de/data/robot
Date=`date +%y%m%d`

date > $DataDir/$Date-runtime
$BinDir/htdig -v -t -s -c $CfgFile >> $DataDir/$Date-robot
$BinDir/htmerge -v -c $CfgFile >> $DataDir/$Date-robot
date >> $DataDir/$Date-runtime

$BinDir/whatsnew.pl -v > $DataDir/$Date-whatsnew
sort $BinDir/urls | uniq > $DataDir/$Date-urls

rm -f $DataDir/current-*
ln -s $DataDir/$Date-runtime $DataDir/current-runtime
ln -s $DataDir/$Date-robot $DataDir/current-robot
ln -s $DataDir/$Date-urls $DataDir/current-urls

$BinDir/status.pl -v > $DataDir/$Date-status

