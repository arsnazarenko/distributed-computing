#!/bin/bash
for i in {1..9}
do
  for j in {0..100}
  do
    ERROR=$(./build/pa1 -p $i 2>&1 1>/dev/null)
    if [ $? -eq 0 ]
    then
      STARTED=$(cat ./events.log | grep "has STARTED$" | wc -l)
      RECEIVE_STARTED=$(cat ./events.log | grep "STARTED messages$" | wc -l)
      DONE=$(cat ./events.log | grep "DONE its work$" | wc -l)
      RECEIVE_DONE=$(cat ./events.log | grep "DONE messages$" | wc -l)
      if [ $STARTED -eq $i ] && [ $RECEIVE_STARTED -eq $(( $i + 1 )) ] && [ $DONE -eq $i ] && [ $RECEIVE_DONE -eq $(( $i + 1 )) ];
      then
        echo "./build/pa1 -p $i: GOOD OUTPUT"
      else
        echo "./build/pa1 -p $i: BAD OUTPUT"
      fi
    else
      echo "./build/pa1 -p $i: $ERROR"
    fi
  done
done
