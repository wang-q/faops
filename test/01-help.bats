#!/usr/bin/env bats

load test_helper

@test "run faops" {
    run $BATS_TEST_DIRNAME/../faops help
    assert_success
}

@test "help: contents" {
    run $BATS_TEST_DIRNAME/../faops help
    echo "${output}" | grep "Usage"
    assert_success
}

# xargs trims whitespaces
@test "help: lines of contents" {
    run $BATS_TEST_DIRNAME/../faops help
    run bash -c "echo \"${output}\" | wc -l | xargs echo "
    assert_equal "20" "${output}"
}
