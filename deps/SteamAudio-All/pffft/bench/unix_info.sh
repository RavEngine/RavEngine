#!/bin/bash

lscpu > unix_lscpu.txt
cat /proc/cpuinfo > unix_cpuinfo.txt
lsb_release -a  > unix_lsb_release.txt
FILES=$(ls -1 /etc/*-release)
if [ ! -z "$FILES" ]; then
  cp /etc/*-release ./
fi
