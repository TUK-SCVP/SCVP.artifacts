#!/usr/bin/perl -w
use warnings;
use strict;
use Bit::Vector;

sub getNumberOfOnes()
{
    my $n = shift;
    my $i = shift;
    my $number = Bit::Vector->new_Dec($n,$i)->Norm;
    return $number;
}

sub getNumbers()
{
    my $n = shift;
    my $count = shift;
    my @result;

    for(my $i = 0; $i < 2**$n; $i++)
    {
        if(&getNumberOfOnes($n,$i) == $count)
        {
            push(@result, $i);
        }
    }

    return @result;
}

open(FH, ">machine.dot");

print FH '
digraph finite_state_machine {
    graph [fontname = "Frutiger LT"];
    node  [shape=circle, fontname = "Frutiger LT", style=filled, fillcolor="#179C7D", fontcolor="white"];
    edge  [fontname = "Frutiger LT"];
    ';


my $n = shift;

#ACT and PRE:
for(my $i = 0; $i < 2**$n-1; $i++)
{
    my @connections = &getNumbers($n,&getNumberOfOnes($n,$i)+1); 
    foreach my $x (@connections) {
        if($i == 0)
        {
            print FH "IDLE -> \"ACTIVE\n".sprintf("%0".$n."b",$x)."\" [ label = \"ACT\" ]\n";
            print FH "\"ACTIVE\n".sprintf("%0".$n."b",$x)."\" -> IDLE [ label = \"PRE\" ]\n";
        }
        else
        {
            print FH "\"ACTIVE\n".sprintf("%0".$n."b",$i)."\" -> \"ACTIVE\n".sprintf("%0".$n."b",$x)."\" [ label = \"ACT\" ]\n";
            print FH "\"ACTIVE\n".sprintf("%0".$n."b",$x)."\" -> \"ACTIVE\n".sprintf("%0".$n."b",$i)."\" [ label = \"PRE\" ]\n";
        }
    }
}

#RD or WR: 
for(my $i = 0; $i < 2**$n; $i++)
{
    if($i != 0)
    {
        print FH "\"ACTIVE\n".sprintf("%0".$n."b",$i)."\" -> \"ACTIVE\n".sprintf("%0".$n."b",$i)."\" [ label = \"RD or WR\" ]\n";
    }
}

print FH '}';

close FH;

if($n >= 5)
{
    system("sfdp -x -Goverlap=scale -Tpng machine.dot -o machine.png");
}
else
{
    system("dot -Tpng machine.dot -o machine.png");
}
