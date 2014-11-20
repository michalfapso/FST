#!/usr/bin/perl
use strict;
use Getopt::Long;

my $loop_label = "";
my $opt_result = GetOptions ("loop-label=s" => \$loop_label); # string
if (!$opt_result || $loop_label eq "") {
	die("Usage: $0 --loop-label LABEL < fst.txt > fst_with_loops.txt");
}

my $max_state_id = 0;

my @fst = ();

# READ DATA AND GET $max_state_id
while (<STDIN>)
{
	chomp;
	my @a = split(/\s+/, $_);
#	if (scalar(@a) == 2) {
#		print STDERR "ERROR: State $a[0] has non-zero final weight. Push weights first!\n";
#		exit 1;
#	}
	if (scalar(@a) == 5) {
		print STDERR "ERROR: Too many fields! FST must be an acceptor!\n";
		exit 1;
	}
	if ($max_state_id < $a[0]) {
		$max_state_id = $a[0];
	}
	push @fst, $_;
}

# PRINT THE MODIFIED FST
print "0 0 $loop_label $loop_label\n";
print "0 1 TERM_START <eps>\n";
my $states_shift = 1;
$max_state_id += $states_shift;
foreach $_ (@fst)
{
	my @a = split(/\s+/, $_);
	
	$a[0] += $states_shift; # Shift states
	if (scalar(@a) > 2) {
		$a[1] += $states_shift; # Shift states
		print "$a[0] $a[1] $a[2] $a[2]";
		if (scalar(@a) > 3) {
			print " $a[3]";
		}
		print "\n";
	} else {
		# FINAL STATE
		print "$a[0] ".($max_state_id+1)." TERM_END <eps>";
		if (scalar(@a) == 2) {
			print " $a[1]";
		}
		print "\n";
	}
}
print "".($max_state_id+1)." ".($max_state_id+1)." $loop_label $loop_label\n";
print "".($max_state_id+1)."\n";

