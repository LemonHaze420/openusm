#!/usr/bin/env bash
set -u

cd "$(dirname "$0")/.."

IMAGE="${CLANG_FORMAT_IMAGE:-silkeh/clang:latest}"

FILES=$(find src include test -name '*.cpp' -o -name '*.h' -o -name '*.hpp')

exec docker run --rm -v "$PWD":/app -w /app \
    --entrypoint clang-format "$IMAGE" \
    --dry-run --Werror $FILES main.cpp
