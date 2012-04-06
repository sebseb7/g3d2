#!/usr/bin/perl

use Device::SerialPort;
use Time::HiRes qw(usleep);

my $hostname = `hostname`;
chomp $hostname;

my $port = "/dev/cu.usbserial-A100DDXG";
if($hostname eq 'ernie')
{
	$port = '/dev/ttyUSB0';
}

my $port = Device::SerialPort->new($port);
$port->databits(8);
$port->baudrate(500000);
$port->parity("none");
$port->stopbits(1);

#my $return=$port->write(chr(102).esc(chr(35+42))."\n");
#while(1)
#{
	my $return=$port->write(chr(102).esc(chr($ARGV[0]))."\n");
#	sleep(1);
#}
warn $port->read(1);

sub esc($)
{
    my $data = shift;
    
    
	$data =~ s/\x65/\x65\x3/go;
	$data =~ s/\x67/\x65\x1/go;
	$data =~ s/\x68/\x65\x2/go;
	$data =~ s/\x66/\x65\x4/go;
                                                            
	return $data;
}
