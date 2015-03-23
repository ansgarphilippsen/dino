#!/usr/bin/perl
# get number of command line args
$ARGC=@ARGV;

# must be one or -h
if($ARGC==0 || $ARGV[0] =~ "-h")
{
    print "Syntax: pdb2xyz FILE.pdb > FILE.xyz\n";
    exit;
}

# open pdb file for input
open PDB, "<$ARGV[0]" or die "Error: could not open input file\n";


# assign the array for the names
while (<DATA>) {
    if (/^$|^#/) {
	# ignore empty lines and comments
    } else {
	if (/^radius/) {
	    ($dummy, $key, $value1, $value2, $value3) = split;
	    $radii1{ $key }= $value1;
	    $radii2{ $key }= $value2;
	    if($value3 =~ /^[0-9]/) {
		$radii3{ $key }= $value3;
	    }
	} else {
	    push @names, [split];
	}
    }
}

for $i ( 0 .. $#names ) {
    $names[$i]->[0] =~ s/^\*/\.\*/;
#    print "$names[$i]->[1]  ->  ";
    $names[$i]->[1] = "^\\s*$names[$i]->[1]\\s*\$";
#    print "$names[$i]->[1]\n";
}


# go through all lines in the PDB file
while (<PDB>)
{
    chomp;
    # record is one of these two
    if (/^(ATOM|HETATM)/) 
    {
	# extract residue-, atom-name, x-, y- and z-coordinates
        $aname=substr($_,12,4);
        $rname=substr($_,17,3);
        $coordx=substr($_,30,8);
        $coordy=substr($_,38,8);
        $coordz=substr($_,46,8);
	# search through the list of names
	for $i ( 0 .. $#names ) {
	    $nref = $names[$i];
	    # $nref->[0] is residue name
	    # $nref->[1] is atom name
	    # $nref->[2] is index into radii
	    if($rname =~ m/$nref->[0]/) {
		if($aname =~ m/$nref->[1]/) {
#		    print "$rname $nref->[0]  $aname $nref->[1]  $nref->[2]  ";
		    if(exists($radii3{$nref->[2]})) {
			print "$coordx $coordy $coordz $radii3{$nref->[2]}\n";
		    } elsif(exists($radii2{$nref->[2]})) {
			print "$coordx $coordy $coordz $radii2{$nref->[2]}\n";
		    } else {
			print "$coordx $coordy $coordz 1.5\n";
		    }
		    last;
		}
	    }
	}
    }
}

#for $i ( 0 .. $#names ) {
#   print "$names[$i]->[0] $names[$i]->[1] $names[$i]->[2]\n"; 
#}

#while (($key,$value) = each(%radii2)) {
#    print "$key $value\n";
#}

close PDB;

# the DATA section is originally from atmtypenumber

__DATA__

#   atom num dist  Rexplicit Runited-atom
radius    1  0.57    1.40
radius    2  0.66    1.40         1.60
radius    3  0.57    1.40
radius    4  0.70    1.54         1.70
radius    5  0.70    1.54         1.80
radius    6  0.70    1.54         2.00
radius    7  0.77    1.74         2.00
radius    8  0.77    1.74         2.00
radius    9  0.77    1.74         2.00
radius   10  0.67    1.74
radius   11  0.70    1.74         1.86
radius   12  1.04    1.80         1.85
radius   13  1.04    1.80  # P, S, and LonePairs
radius   14  0.70    1.54  # non-protonated nitrogens
radius   15  0.37    1.20  # H, D  hydrogen and deuterium
radius   16  0.70    0.00         1.50   # obsolete entry, purpose unknown
radius   17  3.50    5.00  # pseudoatom - big ball
radius   18  1.74    1.97  # Ca calcium
radius   19  1.25    1.40  # Zn zinc    (traditional radius)
radius   20  1.17    1.40  # Cu copper  (traditional radius)
radius   21  1.45    1.30  # Fe heme iron
radius   22  1.41    1.49  # Cd cadmium
radius   23  0.01    0.01  # pseudoatom - tiny dot
radius   24  0.37    1.20         0.00   # hydrogen vanishing if united-atoms
radius   25  1.16    1.24  # Fe not in heme
radius   26  1.36    1.60  # Mg magnesium
radius   27  1.17    1.24  # Mn manganese
radius   28  1.16    1.25  # Co cobalt
radius   29  1.17    2.15  # Se selenium
radius   30  3.00    3.00  # obsolete entry, original purpose unknown
radius   38  0.95    1.80  # obsolete entry, original purpose unknown
#
# note that the metal values are UNCHARGED radii, see
#  http://shef.ac.uk/chemistry/web-elements for info  - Mike Pique

# Hydrogens - not differentiated here
*    H.*    15
*    D.*    15

# Waters and confusingly-named metals
WAT|HOH|H2O|DOD|DIS  O.*        2
CA   CA      18 
CD   CD      22
.*    CD__    22

# Atoms that are sometimes named like mainchain atoms but aren't really
#  get caught here
ACE  CA	      9

# Mainchain atoms - invariant by residue/nucleotide type
*    N        4
*    CA       7
*    C       10
*    O        1
*    P       13

# CB - C beta
ALA  CB       9
ILE|THR|VAL  CB   7
*    CB       8

# CG - C gamma
ASN|ASP|ASX|HIS|HIP|HIE|HID|HISN|HISL|LEU|PHE|TRP|TYR  CG      10
ARG|GLU|GLN|GLX|MET  CG       8
LEU  CG       7
*    CG       8


# Other amino acid residues listed by residue type
# note the "question mark" matches zero or one occurances of pattern
GLN  O1       3
GLN  O2       3
ACE  CH3      9
ARG  CD       8
ARG  NE       4
ARG  CZ      10
ARG  NH[12][AB]?  5
ASN  OD1      1
ASN  ND2      5
ASN  AD1      3
ASN  AD2      3
ASP  OD[12][AB]? 3
ASX  OD1[AB]? 1
ASX  ND2      5
ASX  AD1      3
ASX  AD2      3
ASX  OD2      3
CYS|MET  LP[12]     13
CY[SXM]  SG      13
CYH  SG      12
GLU  OE[12][AB]?    3
GLU|GLN|GLX  CD      10
GLN  OE1      1
GLN  NE2      5
GLN|GLX  AE[12]      3

# His and relatives
# There are 4 kinds of HIS rings: HIS (no protons), HID (proton on Delta),
#   HIE (proton on epsilon), and HIP (protons on both)
# Protonated nitrogens are numbered 4, else 14
# HIS is treated here as the same as HIE
# 
# HISL is a deprotonated HIS (the L means liganded)

HIS|HID|HIE|HIP|HISL	CE1|CD2		11
HIS|HIE|HISL 		ND1		14
HID|HIP			ND1		 4
HIS|HIE|HIP		NE2		 4
HID|HISL		NE2		14
HIS|HID|HIP|HISD	A[DE][12]	 4

ILE  CG1      8
ILE  CG2      9
ILE  CD|CD1      9

LEU  CD1      9
LEU  CD2      9
LYS  C[GDE]       8
LYS  NZ       6
MET  SD      13
MET  CE       9

PHE  C[DE][12]     11
PHE  CZ      11

PRO|CPR  C[GD]       8
CSO  SE       9
CSO  SEG      9
CSO  OD1      3
CSO  OD2      3
SER  OG       2
THR  OG1      2
THR  CG2      9
TRP  CD1     11
TRP  CD2     10
TRP  CE2     10
TRP  NE1      4
TRP  CE3     11
TRP  CZ2     11
TRP  CZ3     11
TRP  CH2     11
TYR  C[DE][12]     11
TYR  CZ      10
TYR  OH       2
VAL  CG1      9
VAL  CG2      9

# catch common atom names for non-standard residue names
*    CD       8
*    CE       8

#
# check these next two with JAT & EDG mp
# (numbering up to 7 is on suggestion of Dave Stout 20 Feb 90 mp)
FS[34]  FE[1-7]     21
FS[34]  S[1-7]      13
FS3  OXO      1
FEO  FE1     21
FEO  FE2     21


HEM  O1       1
HEM  O2       1
HEM  FE      21
HEM  CH[A-D] 11
HEM  N[A-D]  14
HEM  N_[A-D] 14
HEM  C[1-4][A-D]     10
HEM  CM[A-D]  9
HEM  C[AB][AD]     8
HEM  CG[AD]  10
HEM  O[12][AD]      3
HEM  C[AB][BC]     11
HEM  OH2      2
AZI  N[123]  14
MPD  C1       9
MPD  C2      10
MPD  C3       8
MPD  C4       7
MPD  C5       9
MPD  C6       9
MPD  O7       2
MPD  O8       2
SO4|SUL  S   13
SO4|SUL  O[1234]  3
PO4|PHO  O[1234]  3
PC   O4       3    
PC   P1      13    
PC   O[123]       3    
PC   C[12]    8    
PC   N1      14    
PC   C[345]   9    

# Special giant atom (big ball)
BIG  BAL     17

# Special tiny atom (point)
POI  POI     23
DOT  DOT     23

# Metals:

# Heroes of SOD
*   CU      20
*   ZN      19
# Note 24 and 25 have not been OKed by John & Libby - MP, September 1992
*   MN      24
# This free FE is not the same number as heme FE
*   FE      25
# These are metals, considered uncharged so use at your own risk
*   MG      26
*   MN      27
*   CO      28
*   SE      29

# Unknown soldiers:

# FMN is cofactor Flavin mononucleotide
FMN  N1       4
FMN  C[2478] 10
FMN  O2       1
FMN  N3      14
FMN  O4       1
FMN  C[459]A 10
FMN  N5       4
FMN  C[69]   11
FMN  C[78]M   9
FMN  N10      4
FMN  C10     10
FMN  C[12345]\* 8
FMN  O[234]\* 2
FMN  O5\*     3
FMN  OP[1-3]  3

ALK|MYR  OT1  3
ALK|MYR  C01 10
ALK  C16      9
MYR  C14      9
ALK|MYR  C.*  8

# Catch-alls

*    SEG      9
*    OXT      3
*    OT.*      3
*    S.*      13
*    C.*	7
*    O.*	1
*    N.*	4
# PB might be Pb (lead) but its academic... Many PDB files have named P atoms
*    P[A-D]   13
