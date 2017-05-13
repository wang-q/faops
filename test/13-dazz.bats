#!/usr/bin/env bats

load test_helper

@test "dazz: empty seqs count" {
    exp=$($BATS_TEST_DIRNAME/../faops size $BATS_TEST_DIRNAME/ufasta.fa | grep "\s0" | wc -l | xargs echo)
    res=$($BATS_TEST_DIRNAME/../faops dazz $BATS_TEST_DIRNAME/ufasta.fa stdout | grep "0_0" | wc -l | xargs echo)
    assert_equal "$exp" "$res"
}
