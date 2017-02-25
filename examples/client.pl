#use utf8;
use strict;

use IO::File;
use IO::Socket::INET qw(:crlf);

$\ = $CRLF;
$/ = $CRLF;

my $addr = '127.0.0.1';
my $port = '4444';
my $user = 'MEDVED';
my $pass = 'PREVED';
my $fnam = 'perldump.txt';

my $s1 = IO::Socket::INET->new(PeerAddr => $addr,
                               PeerPort => $port) or die "Sock error";

#binmode $s1 => ":encoding(utf8)"; 

print "START: " . <$s1>;

print $s1 "USER $user";
print "USER: " . <$s1>;

print $s1 "PASS $pass";
print "PASS: " . <$s1>;

print $s1 "DATA";
my ($dp) = (<$s1> =~ /#(\d+)/);

print "DP: $dp\n";

my $pid = fork();
die "fork() failed: $!" unless defined $pid;

if ($pid) {
   my $s2 = IO::Socket::INET->new(PeerAddr => $addr,
                                  PeerPort => $dp) or die "DP sock error"; 
   my $fh = IO::File->new("> $fnam");

   print $fh $_ while (<$s2>);

   exit(0);
}

print $s1 "OPEN";
print "OPEN: " . <$s1>;

sleep(30);

print $s1 "QUIT";


