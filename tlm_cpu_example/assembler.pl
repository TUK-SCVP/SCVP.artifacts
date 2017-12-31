#!/usr/bin/perl -w
#
# Copyright 2017 Matthias Jung
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Authors:
#     - Matthias Jung

use warnings;
use strict;

my $file = shift || die("Provide assembler input file");

open(IF, $file);

my $programmCounter = 0;
my %labels;

# Find Labels
while(<IF>)
{
    print $_;
    if($_ =~ /^\s*(.+):\s*$/) # LABEL
    {
        if(exists($labels{$1}))
        {
            die("Double Lable");
        }
        $labels{$1} = $programmCounter;
    }
    if($_ =~ /^\s*(add|addi|mul|lw|sw|jal|jr|bne|nop)\s+.*\s*$/)
    {
        $programmCounter += 4;
    }
}
print "\n";

close(IF);

open(IF, $file);

$programmCounter = 0;

open(OF, ">:raw", $file.".bin");

# Parse asm file and replace labels with relative adresses
while(<IF>)
{
    my $data = 0;

    #                      rd       rs1       rs2
    if($_ =~ /^\s*add\s+x(\d+),\s*x(\d+),\s*x(\d+)\s*$/)
    {
        # Some concistency checks:
        ($1 != 0) || die("x0 must not be destination register");
        ($1 >  0 && $1 < 32) || die("Register does not exist");
        ($2 >= 0 && $2 < 32) || die("Register does not exist");
        ($3 >= 0 && $3 < 32) || die("Register does not exist");

        $data |= 0b0110011;
        $data |= ($1 & 0b11111) << 7;
        $data |= ($2 & 0b11111) << 15;
        $data |= ($3 & 0b11111) << 20;

        printf("$programmCounter:\t%032b:\tadd x$1, x$2, x$3\n",$data);
        print OF pack('I<',$data);
        $programmCounter += 4;
    }
    #                          rd       rs1       imm
    elsif($_ =~ /^\s*addi\s+x(\d+),\s*x(\d+),\s*(\d+|[\d-]*)\s*$/)
    {
        # Some concistency checks:
        ($1 != 0) || die("x0 must not be destination register");
        ($1 >  0 && $1 < 32)    || die("Register does not exist");
        ($2 >= 0 && $2 < 32)    || die("Register does not exist");
        # TODO: ($3 >= 0 && $3 < 2**12) || die("Out of range");

        $data |= 0b0010011;
        $data |= ($1 & 0b11111) << 7;
        $data |= ($2 & 0b11111) << 15;
        $data |= ($3 & 0b111111111111) << 20;

        printf("$programmCounter:\t%032b:\taddi x$1, x$2, $3\n",$data);
        print OF pack('I<',$data);
        $programmCounter += 4;
    }
    #                         rd       rs1       rs2
    elsif($_ =~ /^\s*mul\s+x(\d+),\s*x(\d+),\s*x(\d+)\s*$/)
    {
        # Some concistency checks:
        ($1 != 0) || die("x0 must not be destination register");
        ($1 >  0 && $1 < 32) || die("Register does not exist");
        ($2 >= 0 && $2 < 32) || die("Register does not exist");
        ($3 >= 0 && $3 < 32) || die("Register does not exist");

        $data |= 0b0110011;
        $data |= ($1 & 0b11111) << 7;
        $data |= ($2 & 0b11111) << 15;
        $data |= ($3 & 0b11111) << 20;
        $data |= (0b0000001) << 25;

        printf("$programmCounter:\t%032b:\tadd x$1, x$2, x$3\n",$data);
        print OF pack('I<',$data);
        $programmCounter += 4;
    }
    #                       rd       imm     rs1
    elsif($_ =~ /^\s*lw\s+x(\d+),\s*(\d+)\(x(\d+)\)\s*$/)
    {
        # Some concistency checks:
        ($1 != 0) || die("x0 must not be destination register");
        ($1 >  0 && $1 < 32)    || die("Register does not exist");
        ($2 >= 0 && $2 < 2**12) || die("Out of range");
        ($3 >= 0 && $3 < 32)    || die("Register does not exist");

        $data |= 0b0000011;
        $data |= (0b010) << 12;
        $data |= ($1 & 0b11111) << 7;
        $data |= ($2 & 0b111111111111) << 20;
        $data |= ($3 & 0b11111) << 15;

        printf("$programmCounter:\t%032b:\tlw x$1, $2(x$3)\n",$data);
        print OF pack('I<',$data);
        $programmCounter += 4;
    }
    #                       rs2      imm     rs1
    elsif($_ =~ /^\s*sw\s+x(\d+),\s*(\d+)\(x(\d+)\)\s*$/)
    {
        # Some concistency checks:
        ($1 >  0 && $1 < 32)    || die("Register does not exist");
        ($2 >= 0 && $2 < 2**12) || die("Register does not exist");
        ($3 >= 0 && $3 < 32)    || die("Register does not exist");

        my $i1 = $2  & 0b11111;
        my $i2 = ($2 & 0b111111100000) >> 5;

        $data |= 0b0100011;
        $data |= (0b010) << 12;
        $data |= ($1 & 0b11111) << 20;
        $data |= ($i1) << 7;
        $data |= ($i2) << 25;
        $data |= ($3 & 0b11111) << 15;

        printf("$programmCounter:\t%032b:\tlw x$1, $2(x$3)\n",$data);
        print OF pack('I<',$data);
        $programmCounter += 4;
    }
    #                    rs1
    elsif($_ =~ /^\s*jr\s+x(\d+)\s*$/)
    {
        # Some concistency checks:
        ($1 >= 0 && $1 < 32) || die("Register does not exist");

        $data |= 0b1100111;
        $data |= ($1 & 0b11111) << 15;

        printf("$programmCounter:\t%032b:\tjr x$1\n",$data);
        print OF pack('I<',$data);
        $programmCounter += 4;
    }
    #                        rd       imm
    elsif($_ =~ /^\s*jal\s+x(\d+),\s*(\d+|[\d-]*)\s*$/)
    {
        # Some concistency checks:
        ($1 != 0) || die("x0 must not be destination register");
        ($1 >  0 && $1 < 32)    || die("Register does not exist");

        $data |= 0b1101111;
        $data |= ($1 & 0b11111) << 7;
        $data |= ($2 & 0b11111111111111111111) << 12;

        printf("$programmCounter:\t%032b:\tjal x$1, $2\n",$data);
        print OF pack('I<',$data);
        $programmCounter += 4;
    }
    #                        rd       imm
    elsif($_ =~ /^\s*jal\s+x(\d+),\s*(.+)\s*$/)
    {
        # Some concistency checks:
        ($1 != 0) || die("x0 must not be destination register");
        ($1 >  0 && $1 < 32)    || die("Register does not exist");

        my $imm = $labels{$2} - $programmCounter;

        $data |= 0b1101111;
        $data |= ($1 & 0b11111) << 7;
        $data |= ($imm & 0b11111111111111111111) << 12;

        printf("$programmCounter:\t%032b:\tjal x$1, $imm ($2)\n",$data);
        print OF pack('I<',$data);
        $programmCounter += 4;
    }
    #                        rs1      rs2     imm
    elsif($_ =~ /^\s*bne\s+x(\d+),\s*x(\d+),\s*(\d+|[\d-]*)\s*$/)
    {
        # Some concistency checks:
        ($1 >=  0 && $1 < 32)   || die("Register does not exist");
        ($2 >= 0 && $2 < 32)    || die("Register does not exist");
        # TODO: ($3 >= 0 && $3 < 2**12) || die("Out of range");

        my $imm = $3;
        my $i1 =  (int($imm)) & 0b11111;
        my $i2 = ((int($imm)) & 0b111111100000) >> 5;

        $data |= 0b1100011;
        $data |= (0b001) << 12;
        $data |= ($1 & 0b11111) << 15;
        $data |= ($i1) << 7;
        $data |= ($i2) << 25;
        $data |= ($2 & 0b11111) << 20;

        printf("$programmCounter:\t%032b:\tbne x$1, x$2, $3\n",$data);
        print OF pack('I<',$data);
        $programmCounter += 4;
    }
    #                        rs1       rs2     imm
    elsif($_ =~ /^\s*bne\s+x(\d+),\s*x(\d+),\s*(.+)\s*$/)
    {
        # Some concistency checks:
        ($1 >= 0 && $1 < 32) || die("Register does not exist");
        ($2 >= 0 && $2 < 32) || die("Register does not exist");
        # TODO: label does not exist

        my $imm = $labels{$3} - $programmCounter;

        my $i1 =  (int($imm)) & 0b11111;
        my $i2 = ((int($imm)) & 0b111111100000) >> 5;

        $data |= 0b1100011;
        $data |= (0b001) << 12;
        $data |= ($1 & 0b11111) << 15;
        $data |= ($i1) << 7;
        $data |= ($i2) << 25;
        $data |= ($2 & 0b11111) << 20;

        printf("$programmCounter:\t%032b:\tbne x$1, x$2, $imm ($3)\n",$data);
        print OF pack('I<',$data);
        $programmCounter += 4;
    }
    elsif($_ =~ /^\s*nop\s*$/)
    {
        printf("$programmCounter:\t%032b:\tnop\n",$data);
        print OF pack('I<',$data);
        $programmCounter += 4;
    }
}

close(IF);
close(OF);
