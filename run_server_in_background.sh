#!/bin/bash

if [[ $1 =~ (--build|-b) ]]; then
	./perform_build.sh
fi

./kill_server.sh
./build/server/Server &
