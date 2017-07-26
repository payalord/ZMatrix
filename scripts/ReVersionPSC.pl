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
	
	if($line =~ s/OutFile\s([^\_]*)[0-9\_]*\n$/OutFile\ $1\_$VersionNumberArray[0]\_$VersionNumberArray[1]\_$VersionNumberArray[2]\n/g)
	{
	}
	else
	{
		$line =~ s/OutFile\s([^\_]*)[0-9\_]*([^\n]+)\n$/OutFile\ $1\_$VersionNumberArray[0]\_$VersionNumberArray[1]\_$VersionNumberArray[2]$2\n/g;
	}
	
	print FILE $line;
}
close(FILE);






