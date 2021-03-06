#!/bin/bash

SPEED=1
DELAY=1
LOSS=0
CORRUPT=10
#SOURCE_FILE=$1
#DESTINATION_FILE=$2
SOURCE_FILE='file.in'
DESTINATION_FILE='file.out'

killall link 2> /dev/null
killall receiver 2> /dev/null
killall sender 2> /dev/null

./link_emulator/link speed=$SPEED delay=$DELAY loss=$LOSS corrupt=$CORRUPT &> /dev/null &
sleep 1
./receiver $DESTINATION_FILE &
sleep 1

./sender $SOURCE_FILE
