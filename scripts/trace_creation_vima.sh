#!/bin/bash

cd ~/Experiment/OrCS/trace_generator/extras/pinplay/sinuca_tracer

while IFS= read -r opt
do 
    $opt
done < ~/Experiment/OrCS/traces_vima.txt