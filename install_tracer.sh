#!/bin/bash

# Change this when become necessary
PINPLAY_IDENTIFIER="pinplay-"

# Navigate to the tracer directory
cd trace_generator

# Download pin/pinpoints 3.11
# wget https://software.intel.com/system/files/pinplay-dcfg-3.11-pin-3.11-97998-g7ecce2dac-gcc-linux.tar.bz2

# Extract the files
#https://www.marksanborn.net/linux/extract-without-first-directory/
tar -xvjf $PINPLAY_IDENTIFIER*.tar.bz2 --strip 1
rm $PINPLAY_IDENTIFIER*.tar.bz2 -f

# Copy required files
srcCPP="extras/pinplay/examples/pinplay-debugger-shell.cpp"
srcH="extras/pinplay/examples/pinplay-debugger-shell.H"
dest="extras/pinplay/sinuca_tracer/"
cp $srcCPP $srcH $dest

# Add necessary flags into the makefile
src="./source/tools/Config/makefile.unix.config"
temp="make.temp"
additionalFlags=" -Wno-deprecated-declarations -faligned-new"
sed -r 's/(TOOL_CXXFLAGS_NOOPT :=)(.*)/\1 '"$additionalFlags"' \2/g' $src > $temp
mv $temp $src