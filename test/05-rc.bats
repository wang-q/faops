#!/usr/bin/env bats

load test_helper

@test "rc: output same length" {
    exp=$($BATS_TEST_DIRNAME/../faops size $BATS_TEST_DIRNAME/test1.fa)
    res=$($BATS_TEST_DIRNAME/../faops rc -n $BATS_TEST_DIRNAME/test1.fa stdout \
        | $BATS_TEST_DIRNAME/../faops size stdin)
    assert_equal "$exp" "$res"
}

@test "rc: double rc" {
    exp=$($BATS_TEST_DIRNAME/../faops filter $BATS_TEST_DIRNAME/test1.fa stdout)
    res=$($BATS_TEST_DIRNAME/../faops rc -n $BATS_TEST_DIRNAME/test1.fa stdout \
        | $BATS_TEST_DIRNAME/../faops rc -n stdin stdout)
    assert_equal "$exp" "$res"
}

@test "rc: perl regex" {
    paste <($BATS_TEST_DIRNAME/../faops rc -l 0 $BATS_TEST_DIRNAME/test1.fa stdout | grep -v '^>') \
        <($BATS_TEST_DIRNAME/../faops filter -l 0 $BATS_TEST_DIRNAME/test1.fa stdout | grep -v '^>') \
        | perl -ane '
            $F[0] = uc($F[0]);
            $F[1] =~ tr/ACGTacgt/TGCATGCA/;
            $F[1] = reverse($F[1]);
            exit(1) unless $F[0] eq $F[1]
        '
    assert_success
}
