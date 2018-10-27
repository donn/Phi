#!/usr/bin/env perl

foreach $item (glob("Examples/*")) {
    print "Testing $item...\n";
    my @testable = $item;
    @testable = @testable . '/*' if (-d $item);
    `./phi @testable`;
    if ($? != 0) {
        print "Regression detected at test $item.\n";
    } else {
        print "Test successful.\n";
    }
    print "\n"
}