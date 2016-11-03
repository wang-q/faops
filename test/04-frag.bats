#!/usr/bin/env bats

load test_helper

@test "frag: from first sequence" {
    res=$($BATS_TEST_DIRNAME/../faops frag $BATS_TEST_DIRNAME/ufasta.fa 1 10 stdout | grep -v "^>")
    assert_equal "tCGTTTAACC" "${res}"
}

@test "frag: from specified sequence" {
    res=$($BATS_TEST_DIRNAME/../faops some $BATS_TEST_DIRNAME/ufasta.fa <(echo read12) stdout \
        | $BATS_TEST_DIRNAME/../faops frag stdin 1 10 stdout \
        | grep -v "^>")
    assert_equal "AGCgCcccaa" "${res}"
}
