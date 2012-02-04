#!/usr/bin/perl

use strict;
use POSIX;
use IO::Socket;
use IO::Socket::INET6;
use IO::Select;
use Socket;
use Fcntl;
use Device::SerialPort;
use Time::HiRes qw(usleep time);
use Data::Dumper;

$0='g3d2-server';

close STDERR;
close STDOUT;
open STDOUT,'>>logfile2.txt';
open STDERR,'>&STDOUT';


#02471ff

#globals & configuration

my $usbDevice = '/dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A100DDXG-if00-port0';
my $tcpPort = 1339;
my $buffer = '/dev/shm/g3d2_image';	#where to store the buffer for the webviewer
my $prioLevels = 4;				#begins with level id 0
my $defaultLevel = 1;
my $currentPrio = 0;
my $isRecording = 0;
my $recordPath = '/opt/wallRecords_g3d2/';
my $currentRecordingFile;

my $frameBuffer={};  			#one buffer for each prioLevel
my %activePrios;				#show which connections is on with level

my $wallName = 'g3d2';
my $wallWidth = 72;
my $wallHeight = 32;
my $wallSubpixel = 1;
my $wallBpp = 4;
my $noHardware = 0;

							
warn localtime(time).' started';



my	%intermediaterequests; # this stores intermediate requests 
my $serial;



{
	
	my  $bindSocket = IO::Socket::INET6->new(Proto => 'tcp',Reuse=>1,LocalPort => $tcpPort , Listen => 10) 							#	create listen socket
							|| die "Can't create Socket on host xx on port xx due to  $!\n";				
	my	$socketflags	= fcntl($bindSocket, F_GETFL, 0)					|| die "Can't get flags on Socket due to  $!\n";				#	get the flags and 
	fcntl($bindSocket, F_SETFL, $socketflags | O_NONBLOCK) 					|| die "Can't set O_NONBLOCK flag on Socket due to $!\n";		#	set socket nonblocking
#	$bindSocket->sockopt(SO_RCVBUF, 220);		
		
	my $select = IO::Select->new($bindSocket);


	serialConnect();


	foreach my $level (0..$prioLevels)
	{
		$frameBuffer->{$level} = '0' x ($wallHeight*$wallWidth);
	
	}
	if(-f $buffer)
	{
		open infile,$buffer;
		my $data = <infile>;
		chomp($data);
		$frameBuffer->{$defaultLevel} = $data;
		close infile;		
	}
	
	setFrame($frameBuffer->{$defaultLevel});
	open outfile,'>'.$buffer.'_tmp';
	print outfile $frameBuffer->{$defaultLevel}."\n";
	close outfile;
	rename $buffer.'_tmp',$buffer;



	while( 1 )
	{
		foreach my $socket ($select->can_read(1))
		{
			if( $socket == $bindSocket )								#	if the socket that is ready is the bindSocket a new Connection has occurred 
			{		
				my $client = $bindSocket->accept();						#	get the socket as seperate filedescriptor
				my $host = undef;
				my $port = undef;
				my $lport = undef;
				eval { $host	= $client->peerhost(); $port	= $client->peerport(); $lport	= $client->sockport(); };
				if( defined($host) && defined($port) && defined($lport) )
				{
					$select->add($client);									#	add the newly created socket to our select
	
					$intermediaterequests{$host.':'.$port.':'.$lport}{'lastactive'}=time();
					$intermediaterequests{$host.':'.$port.':'.$lport}{'socket'}=\$client;
					$intermediaterequests{$host.':'.$port.':'.$lport}{'prio'}=$defaultLevel;
					$activePrios{$defaultLevel}->{$host.':'.$port.':'.$lport}=1;
					
					updatePrioLevel();

					my	$socketflags	= fcntl($socket, F_GETFL, 0)			|| die "Can't get flags on Socket due to  $!\n";				#	get the flags and 
					fcntl($bindSocket, F_SETFL, $socketflags | O_NONBLOCK) 		|| die "Can't set O_NONBLOCK flag on Socket due to $!\n";		#	set socket nonblocking
				}
				else
				{
					print STDERR localtime().' Got weird invitation from '.$host.':'.$port."\n";
					close($client);	
				}	
			} 
			else
			{
				my $host = undef;
				my $port = undef;
				my $lport = undef;
				eval { $host	= $socket->peerhost(); $port	= $socket->peerport();$lport	= $socket->sockport(); };
				
				if(defined($host) && defined($port) && defined($lport))
				{
				
					{
						my $address	= $socket->recv(my $data,3000, 0);

						if(  defined($address)  && length($data) )
						{
							$intermediaterequests{$host.':'.$port.':'.$lport}{'lastactive'}=time();
							$intermediaterequests{$host.':'.$port.':'.$lport}{'readbuffer'}.=$data;

							while($intermediaterequests{$host.':'.$port.':'.$lport}{'readbuffer'} =~  /\x0D\x0A/)
							{
								$intermediaterequests{$host.':'.$port.':'.$lport}{'writebuffer'} .= handlerequest($host,$port,$lport,$`);
								$intermediaterequests{$host.':'.$port.':'.$lport}{'readbuffer'} = $';
							}
						}
						else
						{
							$select->remove($socket);
							close($socket);	
							delete $activePrios{$intermediaterequests{$host.':'.$port.':'.$lport}{'prio'}}->{$host.':'.$port.':'.$lport};
							delete $intermediaterequests{$host.':'.$port.':'.$lport};
							updatePrioLevel();
						}
					}

					
				}
				else
				{
					warn localtime(time).' readfail b';
					$select->remove($socket);
					close($socket);	
					delete $activePrios{$intermediaterequests{$host.':'.$port.':'.$lport}{'prio'}}->{$host.':'.$port.':'.$lport};
					delete $intermediaterequests{$host.':'.$port.':'.$lport};
					updatePrioLevel();
				}
			}
		}

		foreach my $socket ($select->can_write(1))
		{
			my $host = undef;
			my $port = undef;
			my $lport = undef;
			eval { $host	= $socket->peerhost(); $port	= $socket->peerport();$lport	= $socket->sockport() };
			
			if( defined($host) && defined($port)&& defined($lport) )
			{
				my $data	= $intermediaterequests{$host.':'.$port.':'.$lport}{'writebuffer'};
				next if !length($data);

				my $byteswritten;
				eval { $byteswritten = $socket->send($data,0) };
				warn localtime(time).' '.$@ if $@;

				if( $byteswritten )
				{
					$intermediaterequests{$host.':'.$port.':'.$lport}{'lastactive'}=time();
					$intermediaterequests{$host.':'.$port.':'.$lport}{'writebuffer'}  = substr($data, $byteswritten, length($data) - $byteswritten);
				} 
				elsif( POSIX::EWOULDBLOCK != $!)
				{
					warn localtime(time).' wouldblock';
					$select->remove($socket);
					close($socket);	
					delete $activePrios{$intermediaterequests{$host.':'.$port.':'.$lport}{'prio'}}->{$host.':'.$port.':'.$lport};
					delete $intermediaterequests{$host.':'.$port.':'.$lport};
					updatePrioLevel();
				}
				else
				{
					warn localtime(time).' code 1';
				}
			}
			else
			{
				$select->remove($socket);
				close($socket);	
				delete $activePrios{$intermediaterequests{$host.':'.$port.':'.$lport}{'prio'}}->{$host.':'.$port.':'.$lport};
				delete $intermediaterequests{$host.':'.$port.':'.$lport};
				updatePrioLevel();
			}	
		}


		####
		#	cleanup sockets that haven't been active
		####
		foreach my $hostport (keys(%intermediaterequests))
		{
			my $socket = ${$intermediaterequests{$hostport}{'socket'}};
			my $host;my $port;my $lport;
			eval { $host	= $socket->peerhost(); $port	= $socket->peerport();$lport	= $socket->sockport(); };
			warn 'doh!' if $@;
			

			if( defined($host) && defined($port)&& defined($lport) )
			{
			}
			else
			{

				$select->remove(${$intermediaterequests{$hostport}{'socket'}});
				close(${$intermediaterequests{$hostport}{'socket'}}) if	${$intermediaterequests{$hostport}{'socket'}};
				delete $activePrios{$intermediaterequests{$hostport}{'prio'}}->{$hostport};
				delete $intermediaterequests{$hostport};
				updatePrioLevel();
			}

			next if $intermediaterequests{$hostport}{'lastactive'} > (time()-300);
			$select->remove(${$intermediaterequests{$hostport}{'socket'}});
			close(${$intermediaterequests{$hostport}{'socket'}}) if	${$intermediaterequests{$hostport}{'socket'}};
			delete $activePrios{$intermediaterequests{$hostport}{'prio'}}->{$hostport};
			delete $intermediaterequests{$hostport};
			updatePrioLevel();
		}
	}
}


sub handlerequest($$$$)
{
	my $host = shift;
	my $port = shift;
	my $lport = shift;
	my $data = shift;
	
	my $myPrio = $intermediaterequests{$host.':'.$port.':'.$lport}{'prio'};
	
	#keep alive
	if($data =~ /^00$/)
	{
		return "width=$wallWidth\r\nheight=$wallHeight\r\nname=$wallName\r\n\r\n";
	}
	if($data =~ /^01$/)
	{
		return "ok\r\n";
	}
	#set Pixel
	elsif($data =~ /^02(.)$/)
	{
		my $green_h = $1;

		$frameBuffer->{$myPrio} = $green_h x ($wallHeight*$wallWidth);

		setFrame($frameBuffer->{$myPrio}) if $myPrio == $currentPrio;

		if($myPrio == $currentPrio)
		{
			open outfile,'>'.$buffer.'_tmp';
			print outfile $frameBuffer->{$myPrio}."\n";
			close outfile;
			rename $buffer.'_tmp',$buffer;
		}
		
		if(($isRecording)and($myPrio == $currentPrio))
		{

			if((time-$currentRecordingFile) > 60*5)
			{
				warn localtime(time).' autostop';
				$isRecording=0;
			}
			else
			{
				open outfile,'>>'.$recordPath.$currentRecordingFile.'.rec';
				print outfile int((time-$currentRecordingFile)*1000).' ';
				print outfile $data."\r\n";
				close outfile;
			}

		}
		
		return 'ok'."\r\n";
	}
	elsif($data =~ /^02(..)(..)(.)$/)
	{
		my $x = hex($1);
		my $y = hex($2);
		my $green_h = $3;
		warn $green_h;
		my $green = hex($3);
		warn $data && return 'bad'."\r\n" if $x > $wallWidth;
		warn $data && return 'bad'."\r\n" if $y > $wallHeight;

		substr($frameBuffer->{$myPrio},(($y-1)*$wallWidth+($x-1)),1,$green_h);
		setPixel($x,$y,$green) if $myPrio == $currentPrio;

		if($myPrio == $currentPrio)
		{
			open outfile,'>'.$buffer.'_tmp';
			print outfile $frameBuffer->{$myPrio}."\n";
			close outfile;
			rename $buffer.'_tmp',$buffer;
		}
		
		if(($isRecording)and($myPrio == $currentPrio))
		{

			if((time-$currentRecordingFile) > 60*5)
			{
				warn localtime(time).' autostop';
				$isRecording=0;
			}
			else
			{
				open outfile,'>>'.$recordPath.$currentRecordingFile.'.rec';
				print outfile int((time-$currentRecordingFile)*1000).' ';
				print outfile $data."\r\n";
				close outfile;
			}

		}
		
		return 'ok'."\r\n";
	}
	#set Frame
	elsif($data =~ /^03([0-9a-fA-F]+)$/)
	{
		my $frame = $1;
	
		$frameBuffer->{$myPrio} = $frame;		

		setFrame($frame) if $myPrio == $currentPrio;

		if($myPrio == $currentPrio)
		{
			open outfile,'>'.$buffer.'_tmp';
			print outfile $frameBuffer->{$myPrio}."\n";
			close outfile;
			rename $buffer.'_tmp',$buffer;
		}

		if(($isRecording)and($myPrio == $currentPrio))
		{
			
			if((time-$currentRecordingFile) > 60*5)
			{
				warn localtime(time).' autostop';
				$isRecording=0;
			}
			else
			{
				open outfile,'>>'.$recordPath.$currentRecordingFile.'.rec';
				print outfile int((time-$currentRecordingFile)*1000).' ';
				print outfile $data."\r\n";
				close outfile;
			}
			
		}


		return 'ok'."\r\n";
	}
	#change prio
	elsif($data =~ /^04(\d\d)$/)
	{
		my $targetPrio = hex $1;
		return 'bad'."\r\n" if $targetPrio > $prioLevels;

		$intermediaterequests{$host.':'.$port.':'.$lport}{'prio'} = $targetPrio;

		delete $activePrios{$myPrio}->{$host.':'.$port.':'.$lport};
		$activePrios{$targetPrio}->{$host.':'.$port.':'.$lport}=1;

		updatePrioLevel();


		return 'ok'."\r\n";
	}
	#start recording
	elsif($data =~ /^05$/)
	{
		warn localtime(time).' start recording';
		$isRecording = 1;
		$currentRecordingFile = int time;
		

			open outfile,'>>'.$recordPath.$currentRecordingFile.'.rec';
			print outfile '0 03'.$frameBuffer->{$myPrio}."\r\n";
			close outfile;
		
		return 'ok'."\r\n";
	}
	#stop recording
	elsif($data =~ /^06$/)
	{
		open outfile,'>>'.$recordPath.$currentRecordingFile.'.rec';
		print outfile int((time-$currentRecordingFile)*1000).' ';
		close outfile;
		$isRecording = 0;
		return $currentRecordingFile."\r\n";
	}
	# play recorded file
	elsif($data =~ /^07(\d+)$/)
	{
		my $fileid = $1;


		#### to implement


		return "ok\r\n";
	}
	# stop play
	elsif($data =~ /^08$/)
	{

		#### to implement

		return "ok\r\n";
	}
	elsif($data =~ /^0901$/)
	{
		$intermediaterequests{$host.':'.$port.':'.$lport}{'listen'} = 1;

		#### to implement

		return "ok\r\n";
	}
	elsif($data =~ /^0900$/)
	{
		$intermediaterequests{$host.':'.$port.':'.$lport}{'listen'} = 0;

		#### to implement

		return "ok\r\n";
	}
	elsif($data =~ /^0A(.*)$/)
	{
		my $data = $1;
		foreach my $hostport (keys(%intermediaterequests))
		{
			next if $hostport eq $host.':'.$port.':'.$lport;
			next if $intermediaterequests{$hostport}{'listen'} != 1;
			next if $intermediaterequests{$hostport}{'prio'} < $currentPrio;
			$intermediaterequests{$hostport}{'writebuffer'} .= '09'.$data."\r\n";
		}

		#### to implement

		return "ok\r\n";
	}
	elsif($data =~ /^0B$/)
	{
		my $debug = Data::Dumper->Dump([\%intermediaterequests]);
		
		$debug .= Data::Dumper->Dump([\%activePrios]);

		return $debug."\r\n";
	}
	else
	{
		return 'bad'."\r\n";
	}
	
}

sub updatePrioLevel()
{
	my $newPrio = 0;
	foreach my $level (0..$prioLevels)
	{
		my $count = int scalar keys %{$activePrios{$level}};
		$newPrio = $level if $count; 
	}
	if($newPrio != $currentPrio)
	{
		$currentPrio = $newPrio;
		setFrame($frameBuffer->{$currentPrio});
		open outfile,'>'.$buffer.'_tmp';
		print outfile $frameBuffer->{$currentPrio}."\n";
		close outfile;
		rename $buffer.'_tmp',$buffer;
	}

	foreach my $level (($currentPrio+1)..$prioLevels)
	{
		$frameBuffer->{$level} = '0' x ($wallHeight*$wallWidth);
	}

}

sub setPixel($$$)
{
	my $x = shift;
	my $y = shift;
	my $green  =shift;


	my $x2 = $x % 8;
	my $y2 = $y % 8;
	my $mod = ($x-$x2)/8  + (($y-$y2)/8*9);
	 
	setModulePixel($mod,$x2,$y2,$green);
	
}


sub setModulePixel($$$$)
{
	my $module = shift;
	my $x = shift;
	my $y = shift;
	my $green  =shift;

	return if $noHardware;

	do
	{
		eval
		{

			warn 'pixel write';
			my $bytes;
			eval
			{
				if($serial)
				{
					$bytes = $serial->write(chr(0x68).escape(chr($module).chr($x).chr($y).chr($green)));
					warn $bytes;
				}
			};
			warn localtime(time).' connection error '.$@ if $@;
			if(! $bytes)
			{
				$serial = Device::SerialPort->new($usbDevice);
				$serial->databits(8);
				$serial->handshake("xoff");
				$serial->baudrate(500000);
				$serial->parity("none");
				$serial->stopbits(1);
				localtime(time).' reconnected';
			}
		};
		if(! $serial) { sleep 1 };
	}until($serial);

}


sub escape($)
{
	my $data = shift;
	
	
	$data =~ s/\x67/\x65\x1/go;
	$data =~ s/\x68/\x65\x2/go;
	$data =~ s/\x65/\x65\x3/go;
	$data =~ s/\x66/\x65\x4/go;
	
	return $data;
}

sub setFrame($)
{
	return if $noHardware;

	warn 'setFrame';
	
	my $frame=shift;

	$serial->write(chr(0x67));#

	my $ppp = $wallWidth*$wallSubpixel;

	for(0..((($wallWidth*$wallSubpixel*$wallHeight)/$ppp) - 1))
	{
		my $packet = $_;
			
		my $data;
		for(0..(($ppp-1)/2))
		{

			$data.=chr(
						hex(substr($frame,($packet*$ppp+($_*2)),1))+
						hex(substr($frame,($packet*$ppp+(($_*2)+1)),1))*0x10
						);
		}

		do
		{
			eval
			{
				my $bytes;
				eval
				{
					if($serial)
					{
						$bytes = $serial->write(escape($data));
					}
				};
				warn localtime(time).' connection error '.$@ if $@;
				if(! $bytes)
				{
					$serial = Device::SerialPort->new($usbDevice);
					$serial->databits(8);
					$serial->handshake("xoff");
					$serial->baudrate(500000);
					$serial->parity("none");
					$serial->stopbits(1);
					localtime(time).' reconnected';
				}
			};
			if(! $serial) { sleep 1 };
		}until($serial);

	}
	$serial->write(chr(0x0));#

}

sub serialConnect()
{
	return if $noHardware;
	do
	{
		warn 'connect';
		eval
		{
			$serial = Device::SerialPort->new($usbDevice);
			$serial->databits(8);
			$serial->handshake("xoff");
			$serial->baudrate(500000);
			$serial->parity("none");
			$serial->stopbits(1);

			$serial->write('B'.chr(0).chr(0).chr(50).chr(50).chr(100));
			sleep(2);
			$serial->write('B'.chr(0).chr(0).chr(0).chr(0).chr(0));
		};
		warn $@ if $@;
		if(! $serial) { warn 'sleep' ;sleep 1 };
	}until($serial);
}
