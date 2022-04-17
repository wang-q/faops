#!/usr/bin/env bats

load test_helper

@test "interleave: empty seqs count" {
    exp=$($BATS_TEST_DIRNAME/../faops size $BATS_TEST_DIRNAME/ufasta.fa | grep "\s0" | wc -l | xargs echo)
    res=$($BATS_TEST_DIRNAME/../faops interleave $BATS_TEST_DIRNAME/ufasta.fa $BATS_TEST_DIRNAME/ufasta.fa.gz | grep "^$" | wc -l | xargs echo)
    assert_equal "$(($exp * 2))" "$res"
}

@test "interleave: empty seqs count (single)" {
    exp=$($BATS_TEST_DIRNAME/../faops size $BATS_TEST_DIRNAME/ufasta.fa | grep "\s0" | wc -l | xargs echo)
    res=$($BATS_TEST_DIRNAME/../faops interleave $BATS_TEST_DIRNAME/ufasta.fa | grep "^$" | wc -l | xargs echo)
    assert_equal "$exp" "$res"
}

@test "interleave: fq" {
    run bash -c "
        $BATS_TEST_DIRNAME/../faops interleave -q $BATS_TEST_DIRNAME/R1.fq.gz $BATS_TEST_DIRNAME/R2.fq.gz |
            grep '^!$' |
            wc -l
    "
    assert_equal 0 "${output}"
}

@test "interleave: fq (single)" {
    run bash -c "
        $BATS_TEST_DIRNAME/../faops interleave -q $BATS_TEST_DIRNAME/R1.fq.gz |
            grep '^!$' |
            wc -l
    "
    assert_equal 25 "${output}"
}
