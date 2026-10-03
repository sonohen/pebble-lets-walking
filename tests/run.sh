#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
test_binary=$(mktemp "${TMPDIR:-/tmp}/letswalking-test.XXXXXX")
trap 'rm -f "$test_binary"' EXIT HUP INT TERM
cc -std=c99 -Wall -Wextra -Werror -fsanitize=address,undefined src/c/walk.c tests/walk_test.c -o "$test_binary"
"$test_binary"
for variant in '-DPBL_COLOR' '-DPBL_COLOR -DPBL_ROUND' '-DPBL_COLOR -DTEST_WIDTH=200 -DTEST_HEIGHT=228' '-DPBL_COLOR -DPBL_ROUND -DTEST_WIDTH=260 -DTEST_HEIGHT=260'; do
  cc -std=c99 -Wall -Wextra -Werror -Wno-unused-function -Wno-unused-parameter -Wno-return-type -fsanitize=address,undefined -Itests/stubs $variant src/c/walk.c tests/main_test.c -o "$test_binary"
  "$test_binary"
done
cc -std=c99 -Wall -Wextra -Werror -Wno-unused-function -Wno-unused-parameter -Wno-return-type -fsanitize=address,undefined -Itests/stubs src/c/walk.c tests/main_test.c -o "$test_binary"
"$test_binary"
