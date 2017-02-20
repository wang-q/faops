#!/usr/bin/env bats

load test_helper

# create replace.tsv
# faops size test/ufasta.fa | perl -nl -e "/\s+0$/ or print" > test/replace.tsv

# ./faops replace -l 0 test/ufasta.fa <(printf "%s\t%s\n" read12 428) stdout

# ./faops replace -l 0 test/ufasta.fa test/replace.tsv stdout

@test "replace: inline names" {
    exp=">428"
    res=$($BATS_TEST_DIRNAME/../faops replace $BATS_TEST_DIRNAME/ufasta.fa \
        <(printf "%s\t%s\n" read12 428) stdout \
        | grep '^>428')
    assert_equal "$exp" "$res"
}

@test "replace: with replace.tsv" {
    exp=$(cat $BATS_TEST_DIRNAME/replace.tsv | cut -f 2)
    res=$($BATS_TEST_DIRNAME/../faops replace $BATS_TEST_DIRNAME/ufasta.fa \
        $BATS_TEST_DIRNAME/replace.tsv stdout \
        | grep '^>' | grep -v 'read' | sed 's/>//' )
    assert_equal "$exp" "$res"
}
