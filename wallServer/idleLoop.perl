#!/usr/bin/perl -I/Users/seb/gitrepos/g3d2/wallServer

use g3d2;
use strict;
use Time::HiRes qw(usleep time);
use List::Util qw(shuffle);


g3d2::init();
g3d2::readline();

g3d2::setLevel(0);
g3d2::readline();


#close STDOUT;
#open STDOUT,'>>logfile.txt';
#open STDERR,'>&STDOUT';


my $PATH = "/Users/seb/Sites/wallRecords_g3d2";
$0 = 'idleloop';
my $event = '';

my $oks = 5;

my $frames = 0;
my $lasttime = time;

while(1)
{
#	eval
#	{


		my @files;
		
		opendir(my $dh, $PATH) || die "can't opendir: $!";
		while(my $file  = readdir($dh)) {
			next if $file !~  /\.rec$/;
			push @files,$file;
		}
		closedir $dh;
		@files = shuffle(@files);

		foreach my $file (@files)
		{

			my $start = 'init';
			my $last = 0 ;

			$0 = 'idleloop - playing: '.$file;
			
			open infile,'/Users/seb/Sites/wallRecords_g3d2/'.$file or next;

			#warn 'idleloop - playing: '.$file;
			while(<infile>)
			{
				if(/^(\d+) (.*)\r\n$/)
				{
					my $delay = 0;
					if($start eq 'init')
					{
						$start = $1;
					}else{
						$delay = $1-$start;#- (time*1000-$start);
						my $diff = time*1000-$last;
						$delay -= $diff;
						$start = $1;
					}
					if( ($delay > 0) and ($delay < 60000))
					{
						while($delay > 100)
						{
							#warn 'x';
							$delay -= 100;
							usleep(100*1000);
							if($event eq 'next')
							{
								$event = '';
							#	last;
							}
						}
					
						usleep($delay*1000);
						
					}
					#warn 'a';
					$last = time*1000;
					g3d2::binFrame($2);
					$oks-- if $oks > 0;
					g3d2::readline() if $oks == 0;
					#warn 'ok';
					
					$frames++;

					if($frames == 100)
					{
						#warn 1/((time-$lasttime)/$frames);
						$frames = 0;
						$lasttime = time;
					}


				}
				if($event eq 'next')
				{
					$event = '';
				#	last;
				}
			}
			close infile;
			# black frame
		};
#	};
#	warn $@ if $@;
}



