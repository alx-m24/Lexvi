#!/usr/bin/env bash
set -e

./scripts/build.sh "$1" "$2" && ./scripts/run.sh
