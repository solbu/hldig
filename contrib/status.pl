#!/usr/local/bin/perl
#
# status.pl v1.0  960413 Iain Lea (iain@sbs.de)
#
# ChangeLog
# 960413 IL  
#
# Produces a HTML 'Search Engine Status' page with last 5 runs
# and 'Top 10' servers by #URLS indexed.
#
# Usage: status.pl [options]
#        -h       help
#        -F file  HTML footer 
#        -H file  HTML header
#        -o file  HTML generated file
#        -v       verbose
#
# TODO

require 'timelocal.pl';
require 'getopts.pl';
require '/www/search.sbs.de/bin/sbs.pl';

$DataDir = '/www/search.sbs.de/data/robot';
$RunTimeFile = "$DataDir/current-runtime";
$RobotFile = "$DataDir/current-robot";
$IndexFile = '/www/search.sbs.de/test/db/db.wordlist';

$DefOutputFile = '/www/search.sbs.de/test/pub/status.html';
$TmpFile = "/tmp/status.$$";
$DefFooter = '';
$DefHeader = '';
$Verbose = 0;
$Top10Servers = 10;

&ParseCmdLine;

print "Generating status.html...\n" if $Verbose;

&ReadDataFiles ($RunTimeFile, $RobotFile, $IndexFile);
&WriteStatus ($DataDir, $DefOutputFile, $DefHeader, $DefFooter);

exit 1;

#############################################################################
# Subroutines
#

sub ParseCmdLine
{
	&Getopts ('F:hH:o:v');

	if ($opt_h ne "") {
		print <<EndOfHelp
Produce an HTML 'Status' page of last 5 runs and Top 10 servers by #URLS.

Usage: $0 [options]
  -h       help
  -F file  HTML footer 
  -H file  HTML header
  -o file  HTML generated file
  -v       verbose

EndOfHelp
;
		exit 0;
	}       
	$DefFooter = $opt_F if ($opt_H ne "");
	$DefHeader = $opt_H if ($opt_H ne "");
	$DefOutputFile = $opt_o if ($opt_o ne "");
	$Verbose = 1 if ($opt_v ne "");
}

sub ReadDataFiles
{
	my ($RunTimeFile, $RobotFile, $IndexFile) = @_;
	my ($IndexSize, $NumWords, $NumURLS, $NumServers);
	my ($BegTime, $EndTime, $RunDate, $RunTime, $Key);
	my (%Months) = (
		'Jan', '0', 'Feb', '1', 'Mar', '2', 'Apr', '3', 'May',  '4', 'Jun',  '5',
		'Jul', '6', 'Aug', '7', 'Sep', '8', 'Oct', '9', 'Nov', '10', 'Dec', '11' );

	# RunDate : RunTime

	open (TIME, "$RunTimeFile") || die "Error: $RunTimeFile - $!\n";
	while (<TIME>) {
		chop;
		if (! $EndTime && $BegTime) {
			# Sat Apr 13 12:57:52 MET DST 1996
			/^...\ (...)\ ([0-9][0-9])\ (..):(..):(..)\ ... ... ([0-9]{4}$)/;
			$EndTime = timelocal ($5, $4, $3, $2, $Months{$1}, $6 - 1900);
			$RunTime = $EndTime - $BegTime;
			$RunTime = sprintf ("%02d%02d", $RunTime/3600, ($RunTime%3600)/60);
			print "END=[$_] [$EndTime] [$RunTime]\n" if $Verbose;
		}
		if (! $BegTime) {
			# Sat Apr 13 12:57:52 MET DST 1996
			/^...\ (...)\ ([0-9][0-9])\ (..):(..):(..)\ ... ... ([0-9]{4}$)/;
			$Mon = $Months{$1};
			$Year = $6 - 1900;
			$BegTime = timelocal ($5, $4, $3, $2, $Mon, $Year);
			$RunDate = sprintf ("%02d%02d%02d", $Year, $Mon+1, $2);
			print "BEG=[$_] [$BegTime] [$RunDate]\n" if $Verbose;
		}
	}
	close (TIME);

	# IndexSize : NumWords : NumURLS : NumServers

	@StatData = stat ($IndexFile);
	$IndexSize = $StatData[7];
	print "SIZE=[$IndexSize]\n" if $Verbose;

	# NumWords : NumURLS : NumServers

	$NumWords = $NumURLS = $NumServers = 0;

	open (ROBOT, "$RobotFile") || die "Error: $RobotFile - $!\n";
	while (<ROBOT>) {
		if (/^htdig:\s+(.*)\s+([0-9]*)\s+documents$/) {
			$NumURLS += $2;
			$NumServers++;
			if ($2 > 0) {
				$Key = sprintf ("%07d|%s", $2, $1);
				$Top10ByName{$Key} = $2; 
			}
			print "SERVER=[$1] DOCS=[$2]\n" if $Verbose;
		} elsif (/^Read\s+([0-9]*)\s+words$/) {
			$NumWords = $1;
			print "WORDS=[$NumWords]\n" if $Verbose;
		}
	}
	close (ROBOT);

	# Write data to YYMMDD-info file

	$InfoFile = "$DataDir/$RunDate-info";
	$CurrFile = "$DataDir/current-info";

	open (INFO, ">$InfoFile") || die "Error: $InfoFile - $!\n";
	print "$RunDate:$RunTime:$IndexSize:$NumWords:$NumURLS:$NumServers\n" if $Verbose;
	print INFO "$RunDate:$RunTime:$IndexSize:$NumWords:$NumURLS:$NumServers\n";
	close (INFO);
	unlink ($CurrFile);
	symlink ($InfoFile, $CurrFile);
}

sub WriteStatus
{
	my ($DataDir, $OutFile, $Header, $Footer) = @_;

	$RobotInfo = &ReadRobotInfo ("$DataDir/current-info");

	open (HTML, ">$OutFile") || die "Error: $OutFile - $!\n";

	&PrintBoilerPlate ($Header, 1);

	print HTML <<EOT
<p>
<strong>$RobotInfo</strong>
<p>
<table border=2 width=400>
<caption>Table of last 5 robot runs.</caption>
<th>Run Date<th>Run Time<th># Servers<th># URL's<th># Words<th>Index (MB)
<tr>
EOT
;
	# read YYMMDD-info files
	opendir (DIR, $DataDir) || die "Error: $DataDir - $!\n";
	@InfoFiles = grep (/^[0-9]{6}-info$/, readdir (DIR));
	closedir (DIR);
	@InfoFiles = reverse (sort (@InfoFiles));

	@InfoFiles = @InfoFiles[0,1,2,3,4]; 
	foreach $File (@InfoFiles) {
		$File = "$DataDir/$File";
		open (INFO, "$File") || die "Error: $File - $!\n";
		chop (($_ = <INFO>));
		($RunDate, $RunTime, $IndexSize, $NumWords, $NumURLS, $NumServers) = split (':');
		$IndexSize = sprintf ("%.1f", $IndexSize / (1024*1024));
		$RunTime =~ /(..)(..)/;
		$RunTime = "$1:$2";
		print HTML <<EOT
<td align="center">$RunDate</td>
<td align="center">$RunTime</td>
<td align="right">$NumServers</td>
<td align="right">$NumURLS</td>
<td align="right">$NumWords</td>
<td align="right">$IndexSize</td>
<tr>
EOT
;
		close (INFO);
	}

	print HTML <<EOT
</table>
<p>
<p>
<table border=2 width=400>
<caption>Table of Top 10 servers listed by number of indexed documents.</caption>
<th>Top 10 Servers<th># URL's
<tr>
EOT
;
	$NumServers = 0;
	foreach $Key (reverse (sort (keys (%Top10ByName)))) {
		if ($NumServers < $Top10Servers) {
			$NumServers++;
			$NumURLS = $Top10ByName{$Key};
			$Key =~ /^[0-9]*\|(.*)$/;
			$Server = $1;		
			$Server =~ s/:80$//;		
			print HTML <<EOT
<td width="80%" align="left"><a href="http://$Server/">$Server</a></td>
<td width="20%" align="right">$NumURLS</td>
<tr>
EOT
;
		}
	}

	print HTML "</table>\n";

	&PrintBoilerPlate ($Footer, 0);

	close (HTML);
}

sub PrintBoilerPlate
{
	my ($File, $IsHeader) = @_;

	if ($File ne "" && -e $File) { 
		open (FILE, $File) || die "Error: $File - $!\n";
		while (<FILE>) {
			print HTML;
		}
		close (FILE);
	} else {
		if ($IsHeader) {
			print HTML <<EOT
<html>
<head>
<title>Search Engine Status</title>
</head>
<body>
<h2>Search Engine Status</h2>
<hr>
<p>
EOT
;
		} else {
			&PrintFooterHTML;
		}
	}
}

