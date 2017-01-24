#!/usr/bin/env bats

load test_helper

@test "order: inline names" {
    exp=$($BATS_TEST_DIRNAME/../faops filter -l 0 $BATS_TEST_DIRNAME/ufasta.fa stdout | grep -A 1 '^>read12')
    res=$($BATS_TEST_DIRNAME/../faops order -l 0 $BATS_TEST_DIRNAME/ufasta.fa <(echo read12) stdout)
    assert_equal "$exp" "$res"
}

@test "order: correct orders" {
    exp=$($BATS_TEST_DIRNAME/../faops filter -l 0 $BATS_TEST_DIRNAME/ufasta.fa stdout | grep -A 1 '^>read12')
    exp+=$'\n'
    exp+=$($BATS_TEST_DIRNAME/../faops filter -l 0 $BATS_TEST_DIRNAME/ufasta.fa stdout | grep -A 1 '^>read5')
    res=$($BATS_TEST_DIRNAME/../faops order -l 0 $BATS_TEST_DIRNAME/ufasta.fa <(echo read12 read5) stdout)
    assert_equal "$exp" "$res"
}
