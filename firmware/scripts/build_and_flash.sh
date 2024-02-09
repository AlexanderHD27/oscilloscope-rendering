#!/usr/bin/bash

if ! scripts/build.sh; then
    exit
else
    scripts/flash.exp main.elf
fi

