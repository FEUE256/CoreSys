#!/bin/sh

sudo apt-get install build-essential cmake pkg-config libicu-dev zlib1g-dev libcurl4-openssl-dev libssl-dev ruby-dev ruby libgit2-dev ruby-github-linguist

gem install github-linguist

echo "alias gh-lang='github-linguist'" >> ~/.bashrc
source ~/.bashrc
