#!/usr/bin/env bash
set -e

./scripts/build.sh "$1" && ./scripts/run.sh
