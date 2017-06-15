#!/usr/bin/env bats

load test_helper

@test "region: from file" {
    exp=$(cat $BATS_TEST_DIRNAME/region.txt | wc -l | xargs echo)
    res=$($BATS_TEST_DIRNAME/../faops region -l 0 $BATS_TEST_DIRNAME/ufasta.fa $BATS_TEST_DIRNAME/region.txt stdout | wc -l | xargs echo)
    assert_equal "$(($exp * 2))" "$res"
}

@test "region: frag" {
    exp=$($BATS_TEST_DIRNAME/../faops frag $BATS_TEST_DIRNAME/ufasta.fa 1 10 stdout)
    res=$($BATS_TEST_DIRNAME/../faops region -l 0 $BATS_TEST_DIRNAME/ufasta.fa <(echo read0:1-10) stdout)
    assert_equal "${exp}" "${res}"
}
