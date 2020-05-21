#!/usr/bin/env python3
import os
import sys
# ###############################################################################################################

# ***************************
# Parameters to be configured
# ***************************
SPEC_FOLDER = "none"
PIN_POINTS_HOME = "/home/war/OrCS/trace_generator"
BASE_CFG_FILE_NAME = PIN_POINTS_HOME + "/scripts/apps_benchmarks/pinplay.cfg"
# ###############################################################################################################

# **************
# Inner classes
# **************
class outputMessage:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

    def error(self, textMsg):
        print(self.FAIL + textMsg + self.ENDC)

    def print(self, textMsg):
        print(self.OKGREEN + textMsg + self.ENDC)
    
    def tip(self, textMsg):
        print(self.OKBLUE + textMsg + self.ENDC)

    def header(self, textMsg):
        print(self.HEADER + textMsg + self.ENDC)
# ###############################################################################################################

# Load spec 2006 files data
BENCHMARK_DIR = SPEC_FOLDER + "/benchspec/CPU2006"
content = []
data = open("apps_benchmarks/apps_spec_cpu2006.txt")
for l in data:
    # Comments
    if(l[0] == '#'):
        continue
    
    # Real app data
    l = l.replace("$BENCHMARK_DIR", BENCHMARK_DIR)
    content.append(l)
data.close()

# ###############################################################################################################

# ***********************************************************
# Verify if all the required variables are set
# ***********************************************************
if SPEC_FOLDER == "none":
    outputMessage().error("SPEC_FOLDER not defined!")
    exit(1)

# ******************************
# Check the number of parameters
# ******************************

if len(sys.argv) != 3:
    outputMessage().error("Missing/extra parameters")
    outputMessage().tip("Usage: python3 " + sys.argv[0] + " <spec 2006 program id> <trace suffix>")
    outputMessage().tip("Spec programs: ")

    id = 1
    for l in content:
        split_app_line = l.split(';')
        program_name = split_app_line[0]
        outputMessage().tip("    {}:".format(id) + program_name)
        id += 1

    exit(1)

# *************************
# Verify if its a valid app
# *************************
app_id = int(sys.argv[1])
TRACE_SUFIX = sys.argv[2]
if((app_id <= 0) or (app_id > len(content))):
    outputMessage().error("Invalid program id: " + sys.argv[1])
    outputMessage().tip("Spec programs: ")

    id = 1
    for l in content:
        split_app_line = l.split(';')
        program_name = split_app_line[0]
        outputMessage().tip("    {}:".format(id) + program_name)
        id += 1

    exit(1)
# ###############################################################################################################

# ***********************
# Prepare the SPEC files
# ***********************
WORKING_DIR = "Working"
TRACE_DIR = "Traces"

SELECTED_BENCHMARK = content[app_id - 1]
program_name = SELECTED_BENCHMARK.split(";")[0]

## **************************
## Select corresponding files
## **************************

ADDFILE = ""
if program_name == 'bwaves':
    ADDFILE = BENCHMARK_DIR + "/410.bwaves/data/ref/input/bwaves.in "
elif program_name == 'gamess':
    ADDFILE = BENCHMARK_DIR + "/416.gamess/data/ref/input/cytosine.2.config "\
                + BENCHMARK_DIR + "/416.gamess/data/ref/input/cytosine.2.inp "

elif program_name == 'zeusmp':
    ADDFILE = BENCHMARK_DIR + "/434.zeusmp/data/ref/input/zmp_inp "

elif program_name == 'gromacs':
    ADDFILE = BENCHMARK_DIR + "/435.gromacs/data/ref/input/gromacs.tpr "

elif program_name == 'gobmk':
    ADDFILE = BENCHMARK_DIR + "/445.gobmk/data/ref/input/* "\
                + BENCHMARK_DIR + "/445.gobmk/data/all/input/* -R "

elif program_name == 'cactusADM':
    ADDFILE = BENCHMARK_DIR + "/436.cactusADM/data/ref/input/benchADM.par "

elif program_name == 'calculix':
    ADDFILE = BENCHMARK_DIR + "/454.calculix/data/ref/input/hyperviscoplastic.inp "

elif program_name == 'GemsFDTD':
    ADDFILE = BENCHMARK_DIR + "/459.GemsFDTD/data/ref/input/ref.in "\
                + BENCHMARK_DIR + "/459.GemsFDTD/data/ref/input/sphere.pec "\
                + BENCHMARK_DIR + "/459.GemsFDTD/data/ref/input/yee.dat "

elif program_name == 'h264ref':
    ADDFILE = BENCHMARK_DIR + "/464.h264ref/data/all/input/foreman_qcif.yuv "\
                + BENCHMARK_DIR + "/464.h264ref/data/all/input/leakybucketrate.cfg "

elif program_name == 'tonto':
    ADDFILE = BENCHMARK_DIR + "/465.tonto/data/ref/input/stdin "

elif program_name == 'lbm':
    ADDFILE = BENCHMARK_DIR + "/470.lbm/data/ref/input/100_100_130_ldc.of "\
                + BENCHMARK_DIR + "/470.lbm/data/ref/input/lbm.in "

elif program_name == 'omnetpp':
    ADDFILE = BENCHMARK_DIR + "/471.omnetpp/data/ref/input/omnetpp.ini "

elif program_name == 'astar':
    ADDFILE = BENCHMARK_DIR + "/473.astar/data/ref/input/BigLakes2048.bin "

elif program_name == 'wrf':
    ADDFILE = BENCHMARK_DIR + "/481.wrf/data/ref/input/* "\
                + BENCHMARK_DIR + "/481.wrf/data/all/input/* "\
                + BENCHMARK_DIR + "/481.wrf/data/all/input/le/32/* "

elif program_name == 'sphinx3':
    ADDFILE = BENCHMARK_DIR + "/482.sphinx3/data/ref/input/beams.dat "\
                + BENCHMARK_DIR + "/482.sphinx3/data/ref/input/args.an4 "\
                + BENCHMARK_DIR + "/482.sphinx3/data/all/input/model -R"
    os.system("cp " + BENCHMARK_DIR + "/482.sphinx3/data/ref/input/*.le.raw " + WORKING_DIR)
    os.system("cd " + WORKING_DIR + " ; for i in `ls *.le.raw` ; do echo \"$i > ${i%.le.raw}.raw\" ; mv $i ${i%.le.raw}.raw  ; done")
    os.system("cd " + WORKING_DIR + " ; echo "" > ctlfile")
    os.system("cd " + WORKING_DIR + " ; for i in `ls *raw`; do ls -ltr $i | nawk '{print substr(\"'\"${i%.raw}\"'\",0) \" \" $5}' >> ctlfile ; done")
else:
    outputMessage().error("Invalid program name: " + program_name)
    exit(1)


if ADDFILE != "":
    os.system("cp "+ADDFILE+" "+WORKING_DIR)
    outputMessage().print("Prepared: "+program_name+" =>"+WORKING_DIR)
else :
    outputMessage().print("No Action: "+program_name)


# ###############################################################################################################

# *************************
# Adjusting the config file
# *************************


split_app_line = SELECTED_BENCHMARK.split(';')

PROGRAM = split_app_line[0]
outputMessage().print(str(app_id) + "-PROGRAM: " + PROGRAM)

INPUT = split_app_line[1]
outputMessage().print("INPUT: " + INPUT)

COMMAND = split_app_line[4]
outputMessage().print("COMMAND: " + COMMAND)

PROGRAM_NAME = PROGRAM + "." + INPUT

CUSTOM_CFG_NAME = PROGRAM_NAME + ".pp.cfg"
CUSTOM_CFG = WORKING_DIR + "/" + CUSTOM_CFG_NAME

outputMessage().print("Adjusting the config file "+CUSTOM_CFG+" for " + program_name)

os.system("cat "+BASE_CFG_FILE_NAME+" > " + CUSTOM_CFG)
os.system("echo program_name: \"" + INPUT      + "\" >> " + CUSTOM_CFG)
os.system("echo input_name:   \"" + PROGRAM    + "\" >> " + CUSTOM_CFG)
os.system("echo command:      \"" + COMMAND    + " >> " + WORKING_DIR + "/OutPut." + PROGRAM_NAME + ".tmp 2>&1 \" >> " + CUSTOM_CFG)
os.system("echo pinplayhome:  \"" + PIN_POINTS_HOME + "\" >> " + CUSTOM_CFG)

os.system("echo mode: \"st\" >> " + CUSTOM_CFG)


os.system("echo sinuca_tracer_output:           \"" + TRACE_DIR +"/"+ PROGRAM_NAME + TRACE_SUFIX + "\" >> " + CUSTOM_CFG)
os.system("echo sinuca_tracer_threads:          \"1\" >> " + CUSTOM_CFG)

os.system("echo sinuca_tracer_parallel_start:   \"" + split_app_line[2] + "\" >> " + CUSTOM_CFG)
os.system("echo sinuca_tracer_parallel_end:     \"" + split_app_line[3] + "\" >> " + CUSTOM_CFG)
# ###############################################################################################################

# ***********************************
# Generating base files for PinPoints
# ***********************************
outputMessage().print("Generating base files for " + program_name)
COMMAND = "cd "+ WORKING_DIR + "; date; " \
            + "time python3 " + PIN_POINTS_HOME + "/extras/pinplay/PinPoints/scripts/sinuca_pinpoints.py " \
            + " --cfg " + CUSTOM_CFG_NAME \
            + " --delete -lbsp "
outputMessage().print("COMMAND: " + COMMAND)
os.system(COMMAND)
# ###############################################################################################################

# ***********************************************
# Generating PinPoints most representative result
# ***********************************************
outputMessage().print("Generating most representative PinPoints result for " + program_name)
COMMAND = "cd "+ WORKING_DIR + "; date; " \
            + "time python3 " + PIN_POINTS_HOME + "/extras/pinplay/PinPoints/scripts/sinuca_pinpoints.py " \
            + " --cfg " + CUSTOM_CFG_NAME \
            + " -T "
outputMessage().print("COMMAND: " + COMMAND)
os.system(COMMAND)

# ###############################################################################################################

# *************
# Final message
# *************
outputMessage().header("\n##################################################################")
outputMessage().header("\nProcess finished!\n The output files are located in the directory " + outputMessage.OKGREEN + WORKING_DIR + outputMessage.ENDC)
outputMessage().header("#################################################################\n")
