#!/bin/bash

function brew_install
{
#    (brew outdated "$1" || brew install "$1") || (echo "Error installing $1"; return 1)
    (brew install "$1") || (echo "Error installing $1"; return 1)
}

brew update
#brew upgrade
brew_install valgrind
brew link --force valgrind
