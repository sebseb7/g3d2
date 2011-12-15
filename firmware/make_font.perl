#!/usr/bin/perl


use strict;

my $chars = 
'xxx   x xxx xxx x x xxx xxx xxx xxx xxx'.
'x x   x   x   x x x x   x     x x x x x'.
'x x   x xxx xxx xxx xxx xxx  xx xxx xxx'.
'x x   x x     x   x   x x x   x x x   x'.
'xxx   x xxx xxx   x xxx xxx   x xxx xxx'
;

open outfile,'>addrfont.h';

print outfile "#ifndef _FONT_H\n";
print outfile "#define _FONT_H\n\n";

print outfile "#ifdef ADDR_0\n";
print_char(0);


foreach my $char (0,1,2,9,10,11,18,19,20,27,28,29)
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
			$i+= 1<<(2-$_) if substr($chars,$char*4+$j*39+$_,1) eq 'x';
			$i+= 1<<((2-$_)+4) if substr($chars,$char10*4+$j*39+$_,1) eq 'x';
		}
		print outfile $i;
		print outfile ',' if $j != 4;
		
	}
	print outfile "};\n";
}


__END__


#ifndef _FONT_H
#define _FONT_H


#ifdef ADDR_0
uint8_t addrfont[] = {128,128,0,0,128}; 
#elif ADDR_100
uint8_t addrfont[] = {119,85,85,85,119}; 
#elif ADDR_101
uint8_t addrfont[] = {113,81,81,81,113}; 
#elif ADDR_102
uint8_t addrfont[] = {119,81,87,84,119}; 
#elif ADDR_109
uint8_t addrfont[] = {119,85,87,81,119}; 
#elif ADDR_110
uint8_t addrfont[] = {23,21,21,21,23}; 
#elif ADDR_111
uint8_t addrfont[] = {17,17,17,17,17}; 
#elif ADDR_118
uint8_t addrfont[] = {23,21,23,21,23}; 
#elif ADDR_119
uint8_t addrfont[] = {23,21,23,17,23}; 
#elif ADDR_120
uint8_t addrfont[] = {119,21,117,69,119}; 
#elif ADDR_127
uint8_t addrfont[] = {119,17,115,65,113}; 
#elif ADDR_128
uint8_t addrfont[] = {119,21,119,69,119}; 
#elif ADDR_129
uint8_t addrfont[] = {119,21,119,65,119}; 
#endif


#endif

