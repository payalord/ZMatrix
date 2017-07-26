#
#  Project      :  zmatrix.n3.net - File Preprocessing Module
#  Filename     :  $RCSfile: html_module.pm,v $
#  Author       :  $Author: deadc0de $
#  Maintainer   :  deadc0de@sourceforge.net
#  File version :  $Revision: 1.1 $
#  Last changed :  $Date: 2003/02/01 20:09:09 $
#  Description  :  Does custom html preprocessing.
#  Licence      :  GNU copyleft
#
########################################################################
# THIS IS A FILEPP MODULE, YOU NEED FILEPP TO USE IT!!!
# usage: filepp -m html_module.pm <files>
########################################################################

package html_module;

#use FindBin;
#use lib "$FindBin::Bin/IO-Stringy/";

#use File::Basename;
#use lib dirname ($0). "/IO-Stringy"; ## Or whatever


#use FindBin;
#use lib $FindBin::Dir . "/IO-Stringy/";

#print STDERR $FindBin::Script . "/IO-Stringy/\n";

use IO::Scalar;                   ### or whatever

use strict;

# version number of module
my $VERSION = '1.0.0';

Filepp::UseModule("blc/blc.pm");
Filepp::UseModule("bigdef/bigdef.pm");
Filepp::UseModule("regexp/regexp.pm");

##############################################################################
# set keyword char to <!-- - allows normal C pre-processor stuff through
##############################################################################
Filepp::SetKeywordchar("<!--#");

##############################################################################
# set optional line end char to something other than ""
##############################################################################
Filepp::SetOptLineEndchar("-->");


##############################################################################
# Custom processor for the site that gets run before all other processors
##############################################################################
sub PreProcessor
{
    my $input = shift;
    if($input)
    {
		my $includeoutput = "";
		
		if($input =~ /\<[\ \t\n]*html[^\>]*\>/ig)
	    {
			tie *Filepp::OUTPUT,'IO::Scalar',\$includeoutput;
			
			Filepp::Include("\"Globals.in\"");
			
			untie *Filepp::OUTPUT;
			
			$input =~ s/(\<[\ \t\n]*html[^\>]*\>)/$includeoutput$1/ig;
		}
		elsif($input =~ /\<[\ \t\n]*head[^\>]*\>/ig)
	    {
			tie *Filepp::OUTPUT,'IO::Scalar',\$includeoutput;
			
			Filepp::Include("\"PreHead.in\"");
			
			untie *Filepp::OUTPUT;
			
			$input =~ s/(\<[\ \t\n]*head[^\>]*\>)/$includeoutput$1/ig;
			$includeoutput = "";
			
			tie *Filepp::OUTPUT,'IO::Scalar',\$includeoutput;
			
			Filepp::Include("\"HeadStart.in\"");
			
			untie *Filepp::OUTPUT;
			
			$input =~ s/(\<[\ \t\n]*head[^\>]*\>)/$1$includeoutput/ig;
		}
		elsif($input =~ /\<[\ \t\n]*\/head[^\>]*\>/ig)
	    {
			tie *Filepp::OUTPUT,'IO::Scalar',\$includeoutput;
			
			Filepp::Include("\"HeadEnd.in\"");
			
			untie *Filepp::OUTPUT;
			
			$input =~ s/(\<[\ \t\n]*\/head[^\>]*\>)/$includeoutput$1/ig;
		}
		elsif($input =~ /\<[\ \t\n]*body[^\>]*\>/ig)
	    {
			tie *Filepp::OUTPUT,'IO::Scalar',\$includeoutput;
			
			Filepp::Include("\"BodyStart.in\"");
			
			untie *Filepp::OUTPUT;
			
			$input =~ s/(\<[\ \t\n]*body[^\>]*\>)/$1$includeoutput/ig;
		}
		elsif($input =~ /\<[\ \t\n]*\/body[^\>]*\>/ig)
	    {
			tie *Filepp::OUTPUT,'IO::Scalar',\$includeoutput;
			
			Filepp::Include("\"BodyEnd.in\"");
			
			untie *Filepp::OUTPUT;
			
			$input =~ s/(\<[\ \t\n]*\/body[^\>]*\>)/$includeoutput$1/ig;
		}
	    
    }
    
    return $input;
}
##############################################################################
# add pre processor
##############################################################################
Filepp::AddProcessor("html_module::PreProcessor",1,1);

##############################################################################
# Custom processor for the site that gets run after all other processors
##############################################################################
sub PostProcessor
{
    my $input = shift;
    return $input;
}
##############################################################################
# add post processor
##############################################################################
Filepp::AddProcessor("html_module::PostProcessor",0,1);


return 1;

########################################################################
# End of file
########################################################################