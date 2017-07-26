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
	$line =~ s/\#define\sMAJOR_VER\s([^\n]+)\n$/\#define MAJOR_VER \"$VersionNumberArray[0]\"\n/g;
	$line =~ s/\#define\sMINOR_VER\s([^\n]+)\n$/\#define MINOR_VER \"$VersionNumberArray[1]\"\n/g;
	$line =~ s/\#define\sRELEASE_VER\s([^\n]+)\n$/\#define RELEASE_VER \"$VersionNumberArray[2]\"\n/g;
	
	
	print FILE $line;
}
close(FILE);






