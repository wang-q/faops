#!/usr/bin/env bats

load test_helper

@test "some: inline name" {
    exp=$($BATS_TEST_DIRNAME/../faops filter -l 0 $BATS_TEST_DIRNAME/ufasta.fa stdout | grep -A 1 '^>read12')
    res=$($BATS_TEST_DIRNAME/../faops some -l 0 $BATS_TEST_DIRNAME/ufasta.fa <(echo read12) stdout)
    assert_equal "$exp" "$res"
}
