#!/usr/bin/env sh

set -x

bundle exec unicorn -c config/unicorn.rb
