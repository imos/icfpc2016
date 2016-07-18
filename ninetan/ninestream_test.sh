#!/bin/bash

TARGET='ninetan/ninestream'
PID=''

setup() {
  local stdin="$(sub::tmpfile)"
  local stdout="$(sub::tmpfile)"
  mkfifo "${stdin}"
  mkfifo "${stdout}"
  "${TARGET}" <"${stdin}" >"${stdout}" &
  PID="$!"
  exec 4>"${stdin}"
  exec 3<"${stdout}"
}

call() {
  local line=''
  echo "$1" >&4
  IFS='' read line <&3
  echo "${line}"
}

test::ninestream_read() {
  setup
  ASSERT_STREQ 'OK 1' "$(call 'run 1 bash')"
  # bash output nothing, so it should exceed deadline.
  ASSERT_STREQ 'DEADLINE_EXCEEDED No ready stream.' "$(call 'read 1 10')"
  ASSERT_STREQ 'OK' "$(call 'write 1 echo foo')"
  # "echo foo" should output "foo".
  ASSERT_STREQ 'OK 1 foo' "$(call 'read 1')"
  ASSERT_STREQ 'OK' "$(call 'write 1 exit')"
  ASSERT_STREQ 'DEADLINE_EXCEEDED No ready stream.' "$(call 'read 1')"
  ASSERT_STREQ 'OK' "$(call 'exit')"
  wait "${PID}"
}

test::ninestream_readall() {
  setup
  ASSERT_STREQ 'OK 1 2 3' "$(call 'run 3 bash')"
  ASSERT_STREQ 'DEADLINE_EXCEEDED No ready stream.' "$(call 'read -1 10')"
  ASSERT_STREQ 'OK' "$(call 'write 2 echo Stream 2')"
  ASSERT_STREQ 'OK 2 Stream 2' "$(call 'read -1')"
  ASSERT_STREQ 'DEADLINE_EXCEEDED No ready stream.' "$(call 'read -1 10')"
  ASSERT_STREQ 'OK' "$(call 'write 1 echo Stream 1')"
  ASSERT_STREQ 'OK 1 Stream 1' "$(call 'read -1')"
  ASSERT_STREQ 'DEADLINE_EXCEEDED No ready stream.' "$(call 'read -1 10')"
  ASSERT_STREQ 'OK' "$(call 'write -1 exit')"
  ASSERT_STREQ 'DEADLINE_EXCEEDED No ready stream.' "$(call 'read -1')"
  ASSERT_STREQ 'OK' "$(call 'exit')"
  wait "${PID}"
}
