import sys
import os


if len(sys.argv) != 3:
    print("Correct use: python3 {} <Command to be traced> <Resultant traces name>".format(sys.argv[0]))
    exit(1)

# Obtaining arguments
command = sys.argv[1]
destination = sys.argv[2]

cmd = '../../../pin -t ../bin/intel64/sinuca_tracer.so -trace x86 -output "{}" -- "{}"'.format(destination, command)
tracerFolder = "cd extras/pinplay/sinuca_tracer"
commandReturnFile = "mv {}.tid*.out.gz ../../../../../".format(destination)
writeInfo = "cd ../../../../../ && pwd"
complete = tracerFolder + " && {} && {} && {}".format(cmd, commandReturnFile, writeInfo)
print("Executing: {}".format(complete))
os.system(complete)