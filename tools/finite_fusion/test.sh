#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
test_dir="$(mktemp -d)"
# No repository files are removed by this cleanup.
trap 'rm -f "$test_dir/test_stats" "$test_dir/test_lifecycle"; rmdir "$test_dir"' EXIT
"${CC:-cc}" -std=c99 -Wall -Wextra -Werror -pedantic \
    -I"$project_root/include" \
    "$project_root/src/finite_fusion_stats.c" \
    "$project_root/tools/finite_fusion/test_stats.c" \
    -o "$test_dir/test_stats"
"$test_dir/test_stats"
"${CC:-cc}" -std=c99 -Wall -Wextra -Werror -pedantic \
    -I"$project_root/include" \
    "$project_root/src/finite_fusion_stats.c" \
    "$project_root/src/finite_fusion_lifecycle.c" \
    "$project_root/tools/finite_fusion/test_lifecycle.c" \
    -o "$test_dir/test_lifecycle"
"$test_dir/test_lifecycle"
