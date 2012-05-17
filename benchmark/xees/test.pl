#!/usr/bin/env perl

my $line;

my $re = qr/$ARGV[0]/;

while ($line = <STDIN>) {
  if ($line =~ $re) {
    print $line;
  }
}
