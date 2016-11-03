#!/usr/bin/env bats

load test_helper

@test "count: read from file" {
    run bash -c "$BATS_TEST_DIRNAME/../faops count $BATS_TEST_DIRNAME/ufasta.fa | head -n 2"
    assert_equal "#seq${tab}len${tab}A${tab}C${tab}G${tab}T${tab}N" "${lines[0]}"
    assert_equal "read0${tab}359${tab}99${tab}89${tab}92${tab}79${tab}0" "${lines[1]}"
}

@test "count: read from gzipped file" {
    run bash -c "$BATS_TEST_DIRNAME/../faops count $BATS_TEST_DIRNAME/ufasta.fa.gz | head -n 2"
    assert_equal "#seq${tab}len${tab}A${tab}C${tab}G${tab}T${tab}N" "${lines[0]}"
    assert_equal "read0${tab}359${tab}99${tab}89${tab}92${tab}79${tab}0" "${lines[1]}"
}

@test "count: read from stdin" {
    run bash -c "cat $BATS_TEST_DIRNAME/ufasta.fa | $BATS_TEST_DIRNAME/../faops count stdin | head -n 2"
    assert_equal "#seq${tab}len${tab}A${tab}C${tab}G${tab}T${tab}N" "${lines[0]}"
    assert_equal "read0${tab}359${tab}99${tab}89${tab}92${tab}79${tab}0" "${lines[1]}"
}

@test "count: lines of result" {
    run $BATS_TEST_DIRNAME/../faops count $BATS_TEST_DIRNAME/ufasta.fa
    run bash -c "echo \"${output}\" | wc -l | xargs echo "
    assert_equal 52 "${output}"
}

@test "count: mixture of stdin and actual file" {
    run bash -c "cat $BATS_TEST_DIRNAME/ufasta.fa | $BATS_TEST_DIRNAME/../faops count stdin $BATS_TEST_DIRNAME/ufasta.fa"
    run bash -c "echo \"${output}\" | wc -l | xargs echo "
    assert_equal 102 "${output}"
}

@test "count: sum of sizes" {
    run bash -c "
        $BATS_TEST_DIRNAME/../faops count $BATS_TEST_DIRNAME/ufasta.fa \
            | perl -ne '/^total\t(\d+)/ and print \$1'
    "
    assert_equal 9317 "${output}"
}
