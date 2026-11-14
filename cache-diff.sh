#!/usr/bin/env bash

if [[ $# -ne 1 || ! $1 =~ ^[0-9]+$ ]]; then
    echo "Usage: $0 <session-number>" >&2
    exit 2
fi

session=$1

if ! command -v sleephistory >/dev/null 2>&1; then
    echo "Error: sleephistory was not found in PATH." >&2
    exit 127
fi

if ! command -v diff >/dev/null 2>&1; then
    echo "Error: diff was not found." >&2
    exit 127
fi

tmpdir=$(mktemp -d) || {
    echo "Error: could not create a temporary directory." >&2
    exit 1
}
trap 'rm -rf "$tmpdir"' EXIT

# No Cache
if ! sleephistory --no-cache -md "$session" >"$tmpdir/no-cache.md" 2>/dev/null; then
    echo "Error: sleephistory failed with --no-cache for session $session." >&2
    exit 1
fi

# Yes Cache
if ! sleephistory -md "$session" >"$tmpdir/cache.md" 2>/dev/null; then
    echo "Error: sleephistory failed while reading the cache for session $session." >&2
    exit 1
fi

diff_file="$tmpdir/diff.txt"

if diff -u --color=always \
    --label "No Cache (Session $session)" \
    --label "With Cache (Session $session)" \
    "$tmpdir/no-cache.md" "$tmpdir/cache.md" >"$diff_file"; then
    printf '\033[1;32mThe Output is identical – no Diff for Session %s.\033[0m\n' "$session"
else
    diff_status=$?
    if [[ $diff_status -eq 1 ]]; then
        less -R "$diff_file"
    else
        echo "Error: diff failed with exit code $diff_status." >&2
        exit "$diff_status"
    fi
fi
