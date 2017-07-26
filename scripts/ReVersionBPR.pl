#!perl

use strict;

my @infile;
my $line;
my $version=$ARGV[0];
my $filename = $ARGV[1];
my @VersionNumberArray = split(/\./,$version);

while($#VersionNumberArray < 3)
{
	push(@VersionNumberArray,0);
}

$version = join('.',@VersionNumberArray);


# open the file to read
open (FILE,"<$filename");
flock (FILE,2);
@infile = <FILE>;
close(FILE);


# open the file
open (FILE,">$filename");
flock (FILE,2);
foreach $line(@infile)
{
	$line =~ s/^MajorVer\=[0-9]\n$/MajorVer\=$VersionNumberArray[0]\n/g;
	$line =~ s/^MinorVer\=[0-9]\n$/MinorVer\=$VersionNumberArray[1]\n/g;
	$line =~ s/^Release\=[0-9]\n$/Release\=$VersionNumberArray[2]\n/g;
	$line =~ s/^Build\=[0-9]\n$/Build\=$VersionNumberArray[3]\n/g;
	
	$line =~ s/^FileVersion\=[0-9\.]*\n$/FileVersion\=$version\n/g;
	$line =~ s/^ProductVersion\=[0-9\.]*\n$/ProductVersion\=$version\n/g;
	print FILE $line;
}
close(FILE);






