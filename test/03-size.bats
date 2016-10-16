#!/usr/bin/env bats

load test_helper

@test "faops size" {
    run bash -c "$BATS_TEST_DIRNAME/../faops size $BATS_TEST_DIRNAME/test1.fa | head -n 2"
    assert_equal "read0${tab}359" "${lines[0]}"
    assert_equal "read1${tab}106" "${lines[1]}"
}
