#!/usr/bin/perl -w


use strict;


while (<>) {
    if ($_ =~ /int main(.*)/ && defined($1) && $1 ne "") {
	print "int dcraw_parse_main". "$1";
    }
    else {
	print $_;
    }
}
