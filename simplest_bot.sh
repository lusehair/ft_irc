#!/bin/bash

# NICK="quotes_bot"
# USER="irccat 8 x : irccat"
# SLEEP_TIME=3.4

send_message()
{
	printf "%s\r\n" "$@"
}

# get_random_quote()
# {
# 	local response=$(curl -s 'https://api.quotable.io/random')
# 	local formatted=$(echo "$response" | tr '"' '\n')
# 	local content=$(echo "$formatted" | grep -A2 content | sed '3q;d')
# 	local author=$(echo "$formatted" | grep -A2 author | sed '3q;d')
# 	echo "$content - $author"
# }

if [ "$#" -ne 4 ]; then
    echo "Please launch the bot using: 'sh $0 <host> <port> <password> <channel>'"
    exit 1
fi

# nc -q 0 "$1" "$2"
bot()
{
    if ! (echo "$4" | grep -q '#'); then
        channel="#$4"
    fi

    send_message "PASS $3"
    send_message "NICK Oh_My_God_What_A_Shhh_Bot"
    send_message "USER ircdoog 8 * :I'm a bot"
    send_message "JOIN $channel"
    while read line; do
        if ( echo "$line" | grep 'PRIVMSG #'); then
            send_message "NOTICE $(echo $line | awk '{print $2}') :Shhhhhhh"
        fi
    done
}

# log $@ | nc -q 0 "$1" "$2"
# read line
# if ( echo "$line" | grep 'ERR' ); then
#     echo "Cannot connect to the server. Probably used nickname or wrong parameters"
#     exit 1
# fi

while true;do
    bot $@ | nc -q 0 "$1" "$2"
done

#!/bin/bash
