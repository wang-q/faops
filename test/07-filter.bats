#!/usr/bin/env bats

load test_helper

@test "filter: as formatter, sequence in one line" {
    exp=$($BATS_TEST_DIRNAME/../faops size $BATS_TEST_DIRNAME/test1.fa | wc -l | xargs echo)
    res=$($BATS_TEST_DIRNAME/../faops filter -l 0 $BATS_TEST_DIRNAME/test1.fa stdout | wc -l | xargs echo)
    assert_equal "$(( exp * 2 ))" "$res"
}

@test "filter: as formatter, identical headers" {
    exp=$(grep '^>' $BATS_TEST_DIRNAME/test1.fa)
    res=$($BATS_TEST_DIRNAME/../faops filter -l 0 $BATS_TEST_DIRNAME/test1.fa stdout | grep '^>')
    assert_equal "$exp" "$res"
}

@test "filter: as formatter, identical sequences" {
    exp=$(grep -v '^>' $BATS_TEST_DIRNAME/test1.fa | perl -ne 'chomp; print')
    res=$($BATS_TEST_DIRNAME/../faops filter -l 0 $BATS_TEST_DIRNAME/test1.fa stdout | grep -v '^>' | perl -ne 'chomp; print')
    assert_equal "$exp" "$res"
}
