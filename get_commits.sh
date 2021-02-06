#!/bin/bash

DEFINES_COMMITS=""; \
for file in $*; do
	DEFINES_COMMITS="${DEFINES_COMMITS},`echo ${file} | cut -d '/' -f 2`=`git log ${file} | head -1 | cut -d ' ' -f 2`";
done; 
DEFINES_COMMITS="${DEFINES_COMMITS:1}";
echo "${DEFINES_COMMITS}";
