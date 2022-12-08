#!/usr/bin/perl

=head1 DESCRIPTION

=cut
print "Status: 200 OK\r\n";
print "Content-Type: text/plain\r\n\r\n";

foreach ( sort keys %ENV ) {
    print "$_=\"$ENV{$_}\"\n";
}