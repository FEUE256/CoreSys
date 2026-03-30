#!/bin/bash

while true; do
    # Check for index.lock file
    lock_path="../../.git/index.lock"
    if [ -f "$lock_path" ]; then
        rm -f "$lock_path"
        echo "index.lock removed."
    else
        echo "No lock file found."
    fi

    sleep 1  # Wait one second before checking again
done
