#!/usr/bin/env sh

set -x

bundle exec puma -w 1 -t 5:5
