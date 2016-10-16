#!/usr/bin/env bats

load test_helper

@test "output same length" {
    ori=$($BATS_TEST_DIRNAME/../faops size $BATS_TEST_DIRNAME/test1.fa)
    rc=$($BATS_TEST_DIRNAME/../faops rc -n $BATS_TEST_DIRNAME/test1.fa stdout \
        | $BATS_TEST_DIRNAME/../faops size stdin)
    assert_equal "$ori" "$rc"
}
