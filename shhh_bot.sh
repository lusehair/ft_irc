#!/bin/bash

input=".botchatlogs"

HOST="$1"
PORT="$2"
PASS="$3"
NAME="Oh_My_God_What_A_Shhh_Bot"
CHAN="#BotHouse"

if [ "$#" -ne 3 ]; then
    echo "Please launch the bot using: 'sh $0 <host> <port> <password>'"
    exit 1
fi

log_n_join()
{
    printf "PASS ${PASS}\r\n"
    printf "NICK ${NAME}\r\n"
    printf "USER ${NAME} * * :${NAME}\r\n"
    printf "JOIN ${CHAN}\r\n"
}

log_n_join > $input
trap "rm -f $input;exit 0" INT TERM EXIT
tail -f $input | nc $HOST $PORT | while read line; do
    if ( echo "$line" | grep "PRIVMSG ${CHAN}"); then
        printf "NOTICE ${CHAN} :Shhhhhhh\r\n" >> $input
    fi
done
