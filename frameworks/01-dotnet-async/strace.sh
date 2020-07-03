#!/usr/bin/env sh

set -x

strace -qqq \
  -Tfe trace=network,epoll_ctl \
  dotnet run bin/Debug/netcoreapp3.1/01-dotnet-async.dll
