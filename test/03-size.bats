#!/usr/bin/env bats

load test_helper

@test "individual sizes" {
    run bash -c "$BATS_TEST_DIRNAME/../faops size $BATS_TEST_DIRNAME/test1.fa | head -n 2"
    assert_equal "read0${tab}359" "${lines[0]}"
    assert_equal "read1${tab}106" "${lines[1]}"
}

@test "read from stdin" {
    run bash -c "cat $BATS_TEST_DIRNAME/test1.fa | $BATS_TEST_DIRNAME/../faops size stdin | head -n 2"
    assert_equal "read0${tab}359" "${lines[0]}"
    assert_equal "read1${tab}106" "${lines[1]}"
}

@test "lines of result" {
    run $BATS_TEST_DIRNAME/../faops size $BATS_TEST_DIRNAME/test1.fa
    run bash -c "echo \"${output}\" | wc -l | xargs echo "
    assert_equal 50 "${output}"
}

@test "mixture of stdin and actual file" {
    run bash -c "cat $BATS_TEST_DIRNAME/test1.fa | $BATS_TEST_DIRNAME/../faops size stdin $BATS_TEST_DIRNAME/test1.fa"
    run bash -c "echo \"${output}\" | wc -l | xargs echo "
    assert_equal 100 "${output}"
}

@test "sum of sizes" {
    run bash -c "
        $BATS_TEST_DIRNAME/../faops size $BATS_TEST_DIRNAME/test1.fa \
            | perl -ane '\$c += \$F[1]; END { print qq{\$c\n} }'
    "
    assert_equal 9317 "${output}"
}
