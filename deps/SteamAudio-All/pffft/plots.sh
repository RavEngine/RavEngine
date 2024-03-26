#!/bin/bash

OUTPNG="1"
W="1024"
H="768"
PTS="20"
LWS="20"

for f in $(ls -1 *-4-*.csv *-6-*.csv); do
  b=$(basename "$f" ".csv")
  #echo $b
  LASTCOL="$(head -n 1 $f |sed 's/,/,\n/g' |grep -c ',')"
  echo "${b}: last column is $LASTCOL"
  if [ $(echo "$b" |grep -c -- "-1-") -gt 0 ]; then
    YL="duration in ms; less is better"
  elif [ $(echo "$b" |grep -c -- "-4-") -gt 0 ]; then
    YL="duration relative to pffft; less is better"
  else
    YL=""
  fi

  E=""
  if [ "${OUTPNG}" = "1" ]; then
    E="set terminal png size $W,$H"
    E="${E} ; set output '${b}.png'"
  fi
  if [ -z "${E}" ]; then
    E="set key outside"
  else
    E="${E} ; set key outside"
  fi
  E="${E} ; set datafile separator ','"
  E="${E} ; set title '${b}'"
  E="${E} ; set xlabel 'fft order: fft size N = 2\\^order'"
  if [ ! -z "${YL}" ]; then
    #echo "  setting  Y label to ${YL}"
    E="${E} ; set ylabel '${YL}'"
  fi
  # unfortunately no effect for 
  #for LNO in $(seq 1 ${LASTCOL}) ; do
  #  E="${E} ; set style line ${LNO} ps ${PTS} lw ${LWS}"
  #done
  E="${E} ; plot for [col=3:${LASTCOL}] '${f}' using 2:col with lines title columnhead"

  if [ "${OUTPNG}" = "1" ]; then
    gnuplot -e "${E}"
  else
    gnuplot -e "${E}" --persist
  fi
done
