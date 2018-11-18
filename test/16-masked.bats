#!/usr/bin/env bats

load test_helper

@test "masked" {
    exp="read46:3-4"
    res=$($BATS_TEST_DIRNAME/../faops masked $BATS_TEST_DIRNAME/ufasta.fa | grep '^read46' | head -n 1)
    assert_equal "$exp" "$res"
}
