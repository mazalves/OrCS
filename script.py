import os
#python script.py vima traces_microbenchmarks.txt configuration_files/sandy_vima.cfg _outro

import sys
filepath = sys.argv[2]
path = '~/Experiment/benchmarks/traces/'
benchmark = sys.argv[1]
config = sys.argv[3]
output = ""
folder = "resultados"
suffix = ""

if os.path.exists(folder) == False: os.mkdir (folder)
#if os.path.exists(folder+"/"+benchmark) == False: os.mkdir (folder+"/"+benchmark)
if len(sys.argv) > 4: 
    suffix = sys.argv[4]
    if os.path.exists(folder+"/"+benchmark+suffix) == False: os.mkdir (folder+"/"+benchmark+suffix)
else:
	if os.path.exists(folder+"/"+benchmark) == False: os.mkdir (folder+"/"+benchmark)


with open(filepath) as fp:
   for cnt, line in enumerate(fp):
    output += ("./orcs -t {}{}/{} -c {} > {}/{}{}/{}.txt; ".format (path, benchmark, line.rstrip(), config, folder, benchmark, suffix, line.rstrip()))

print (output)
os.system (output)
