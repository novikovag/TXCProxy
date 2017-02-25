#!/usr/bin/perl -w
use utf8;
use strict;

use IO::File;
use IO::Socket::INET qw(:crlf);

$\ = $CRLF;
$/ = $CRLF;

# use your TCXproxy server IP/port here
my $addr = '192.168.0.1';
my $port = '4444';
# use your broker TransaqConnector account login/pass here
my $user = 'Username';
my $pass = 'Password';

# file to store XML from broker (all in one, as flow from socket)
# you can use any XML parser to parse this XML as you wish...
my $fnam = 'exampleXML.txt';

# Sosket for send XML commands to broker
my $s1 = IO::Socket::INET->new(PeerAddr => $addr,
                               PeerPort => $port) or die "Sock error";

binmode $s1 => ":encoding(utf8)";.

print "START: " . <$s1>;

print $s1 "USER $user";
print "USER: " . <$s1>;

print $s1 "PASS $pass";
print "PASS: " . <$s1>;

print $s1 "DATA";
my ($dp) = (<$s1> =~ /#(\d+)/);

# Port for receive XML data
print "DP: $dp\n";

# You can use telnet to DP port to receive XML data
# if you (!) open manually in separate terminal (!)
# Or use this child process to read from DP to file

my $pid = fork();
die "fork() failed: $!" unless defined $pid;

if ($pid) {
   # Socket for receive XML answers from broker
   my $s2 = IO::Socket::INET->new(PeerAddr => $addr,
                                  PeerPort => $dp) or die "DP sock error";.
   my $fh = IO::File->new("> $fnam");

   sleep(10);

   print $fh $_ while (<$s2>);

   exit(0);
}

# Data can be slow sometimes, optimize sleep() calls for your broker data flow
sleep(10);


print $s1 "OPEN";
print "OPEN: " . <$s1>;

sleep(10);

# To write command you must parse some XMLs first for use your broker current IDs: ClientID, secid for example.
# You should do that, becouse your broker can change values time-to-time or give diffirent for each trade session.

# Command to req 500 candles from broker
print $s1 "SEND <command id=\"gethistorydata\" secid=\"27\" period=\"1\" count=\"500\" reset=\"true\"/></command>";
print "SEND: " . <$s1>;

sleep(10);

# Command to send real order to Buy 100 shares.
# Commented for safe your account. ;-) Uncomment for real use.
#print $s1 "SEND <command id=\"neworder\"><secid>27</secid><client>ClientID</client><quantity>100</quantity><buysell>buy</buysell><bymarket/>";
#print "SEND: " . <$s1>;

sleep(20);

print $s1 "QUIT";

exit(0);
