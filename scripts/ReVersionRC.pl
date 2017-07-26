#!perl

use strict;

my @infile;
my $line;
my $version=$ARGV[0];
my $versionCommaNoSpaces = $ARGV[0];
my $filename = $ARGV[1];
my @VersionNumberArray = split(/\./,$version);

while($#VersionNumberArray < 3)
{
	push(@VersionNumberArray,0);
}

$version = join('.',@VersionNumberArray);
$versionCommaNoSpaces = join(',',@VersionNumberArray);

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
	$line =~ s/\sFILEVERSION\s([^\n]*)/ FILEVERSION $versionCommaNoSpaces/g;
	$line =~ s/\sPRODUCTVERSION\s([^\n]*)/ PRODUCTVERSION $versionCommaNoSpaces/g;

	$line =~ s/VALUE\s\"FileVersion\"\,\s\"([^\"]*)\"/VALUE \"FileVersion\"\, \"$version\\0\"/g;
	$line =~ s/VALUE\s\"ProductVersion\"\,\s\"([^\"]*)\"/VALUE \"ProductVersion\"\, \"$version\\0\"/g;
	print FILE $line;
}
close(FILE);






