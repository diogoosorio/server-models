#!/usr/bin/env bash

while true; do 
  ss -tap -o state established | egrep $(dig marleyspoon.com +short | paste -s -d "|");
done
