#!/usr/bin/perl -w


use strict;


while (<>) {
    if ($_ =~ /int CLASS main(.*)/ && defined($1) && $1 ne "") {
	print "int CLASS dcraw_main". "$1";
    }
    else {
	print $_;
    }
}
