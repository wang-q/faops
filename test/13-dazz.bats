#!/usr/bin/env bats

load test_helper

@test "dazz: empty seqs count" {
    exp=$($BATS_TEST_DIRNAME/../faops size $BATS_TEST_DIRNAME/ufasta.fa | grep "\s0" | wc -l | xargs echo)
    res=$($BATS_TEST_DIRNAME/../faops dazz -a $BATS_TEST_DIRNAME/ufasta.fa stdout | grep "0_0" | wc -l | xargs echo)
    assert_equal "$exp" "$res"
}

@test "dazz: deduplicate seqs" {
    exp=$($BATS_TEST_DIRNAME/../faops size $BATS_TEST_DIRNAME/ufasta.fa $BATS_TEST_DIRNAME/ufasta.fa.gz | grep "\s0" | wc -l | xargs echo)
    res=$(gzip -d -c -f $BATS_TEST_DIRNAME/ufasta.fa $BATS_TEST_DIRNAME/ufasta.fa.gz | $BATS_TEST_DIRNAME/../faops dazz stdin stdout | grep "0_0" | wc -l | xargs echo)
    assert_equal "$(($exp / 2))" "$res"
}

@test "dazz: duplicated seqs" {
    exp=$($BATS_TEST_DIRNAME/../faops size $BATS_TEST_DIRNAME/ufasta.fa $BATS_TEST_DIRNAME/ufasta.fa.gz | grep "\s0" | wc -l | xargs echo)
    res=$(gzip -d -c -f $BATS_TEST_DIRNAME/ufasta.fa $BATS_TEST_DIRNAME/ufasta.fa.gz | $BATS_TEST_DIRNAME/../faops dazz -a stdin stdout | grep "0_0" | wc -l | xargs echo)
    assert_equal "$exp" "$res"
}
