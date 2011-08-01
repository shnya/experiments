#!/usr/bin/perl
use Unicode::Japanese;
my %hash;
my %kanji_hash;
my $kanji_id = 1;

open(OUT, "> out.eucjp") or die ("can't open out.eucjp");

open(IN,"./yomi_hyouki.txt") or die("can't open yomi_hyouki.txt");
while(<IN>){
  chomp;
  my @field = split /\s+/;
  my $id;
  $field[0] = Unicode::Japanese->new($field[0],'euc')->kata2hira->euc;
  if(exists $kanji_hash{$field[1]}){
    $id = $kanji_hash{$field[1]};
  }else{
    $kanji_hash{$field[1]} = $id = $kanji_id++;
  }
  push @{$hash{$field[0]}}, $id;
}

my $i = 1;
my @keys = keys %hash;
print OUT $#keys+1, "\n";
foreach my $key (@keys){
  print OUT $i++, "\t", $key, "\n";
}

$i = 1;
print OUT $#keys+1, "\n";
foreach my $key (@keys){
  print OUT $i++, "\t", join(' ',@{$hash{$key}}), "\n";
}



@keys = sort {$kanji_hash{$a} <=> $kanji_hash{$b}} keys %kanji_hash;
print OUT $#keys+1, "\n";
foreach my $key (@keys){
  print OUT $kanji_hash{$key}, "\t", $key, "\n";
}

close(IN);
my %unigram;
open(IN2,"< 2-gram.txt") or die("can't open 2-gram.txt");
while(<IN2>){
  chomp;
  my @field1 = split /\s+/;
  my @field3 = split /\//, $field1[2];
  if(exists $kanji_hash{$field3[0]}){
    $unigram{$kanji_hash{$field3[0]}} += $field1[0];
  }else{
    next;
  }
}
close(IN2);
open(IN3,"< 2-gram.txt") or die("can't open 2-gram.txt");
while(<IN3>){
  chomp;
  my @field1 = split /\s+/;
  my @field2 = split /\//, $field1[1];
  my @field3 = split /\//, $field1[2];
  if(exists $kanji_hash{$field2[0]} && exists $kanji_hash{$field3[0]}){
    print OUT ($field1[0]/$unigram{$kanji_hash{$field3[0]}})
               , " ", $kanji_hash{$field2[0]}
               ," ", $kanji_hash{$field3[0]}, "\n";
  }else{
    next;
  }
}
close(IN3);
close(OUT);
system("nkf -w out.eucjp > out.txt");
unlink("./out.eucjp");
