#!/bin/bash

if ps aux | grep -v grep | grep -q '/server/Server'; then
	sudo kill -9 $(pgrep -l "Server" | grep -oE '[0-9]*')
	echo 'Killed server successfully'
else
	echo 'Server not running'
fi

