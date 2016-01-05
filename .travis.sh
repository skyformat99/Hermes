#! /bin/bash

BLACK=$(tput setaf 0)
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
YELLOW=$(tput setaf 3)
BLUE=$(tput setaf 4)
MAGENTA=$(tput setaf 5)
ORANGE=$(tput setaf 172)
WHITE=$(tput setaf 7)
BRIGHT=$(tput bold)
NORMAL=$(tput sgr0)
BOLD=$(tput bold)

ROOT=$PWD

for d in */ ; do
    printf "${BLUE}**********************************************************************************************\n\n"
    printf "                                        ${BOLD}${d}${NORMAL}${BLUE}                                                  \n\n"
    printf "**********************************************************************************************${NORMAL}\n\n"

    cd $d
    if [ ! -f CMakeLists.txt ]; then
	    printf "${RED}CMake file not found for ${d}${NORMAL}\n"
    else
    	if [ -d build ]; then
    	    rm -r build
    	fi
    	mkdir build
    	cd build
    	printf "${GREEN}***************** Building ********************${NORMAL}\n\n"
    	cmake ../
    	make
    	printf "\n${GREEN}***************** Testing *********************${NORMAL}\n\n"
        make test
    fi
    cd $ROOT
    printf "\n"
done
