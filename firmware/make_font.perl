#!/usr/bin/perl


use strict;

my $chars = 
'XXX   X XXX XXX X X XXX XXX XXX XXX XXX'.
'X X   X   X   X X X X   X     X X X X X'.
'X X   X XXX XXX XXX XXX XXX  XX XXX XXX'.
'X X   X X     X   X   X X X   X X X   X'.
'XXX   X XXX XXX   X XXX XXX   X XXX XXX'
;

open outfile,'>addrfont.h';

print outfile "#ifndef _FONT_H\n";
print outfile "#define _FONT_H\n\n";

print outfile "#ifdef ADDR_0\n";
print_char(0);


foreach my $char (0..45)
{
	print outfile "#elif ADDR_".($char+100)."\n";
	print_char($char);
}



print outfile "#endif\n\n\n";
print outfile "#endif\n";

close outfile;


sub print_char($)
{
	my $char = shift;
	
	my $char10 = ($char - $char%10)/10;
	my $char = $char%10;
	
	print outfile "uint8_t addrfont[] = {";
	foreach my $j (0..4)
	{
		my $i;		
		for(0..2)
		{
			$i+= 1<<(2-$_) if substr($chars,$char*4+$j*39+$_,1) eq 'X';
			$i+= 1<<((2-$_)+4) if substr($chars,$char10*4+$j*39+$_,1) eq 'X';
		}
		print outfile $i;
		print outfile ',' if $j != 4;
		
	}
	print outfile "};\n";
}

