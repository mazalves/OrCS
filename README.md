# Ordinary Computer Simulator (OrCS)
**OrCS** is a cycle-accurate and trace-driven simulator developed to Computer Architecture researches. It simulates a simple architecture containing at least a processor, a main memory and a cache-hierarchy. It is being improved by the researchers every day.

## Getting started
These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites
To run OrCS, it is necessary to install the following libraries:

#### libconfig
Download the [libconfig](http://hyperrealm.github.io/libconfig/) library from the website and run the following commands on terminal:
```bash
$ cd libconfig-x.x.x.tar.gz
$ ./configure
$ make; make check
$ make install
$ make clean
```

#### zlib
Download the [zlib](https://zlib.net/) library by terminal commands only:
```bash
$ sudo apt install zlib1g-dev
```



### Install OrCS
Clone or download OrCS
```bash
$ git clone git@github.com:mazalves/OrCS.git
$ cd OrCS
$ make clean; make orcs
```

Output:
```
rm -f simulator.o orcs_engine.o trace_reader.o package/opcode_package.o package/uop_package.o package/memory_package.o processor/memory_disambiguation/disambiguation_hashed.o processor/processor.o processor/reorder_buffer_line.o processor/memory_order_buffer_line.o processor/instruction_set.o utils/utils.o utils/enumerations.o branch_predictor/branch_predictor.o branch_predictor/piecewise.o cache/cache.o cache/cache_manager.o hive/hive_controller.o vima/vima_controller.o vima/vima_vector.o prefetcher/prefetcher.o prefetcher/stride_prefetcher.o config/config.o main_memory/memory_channel.o main_memory/memory_controller.o main_memory/memory_request_client.o
rm -f orcs
OrCS cleaned!

g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence simulator.cpp -o simulator.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence orcs_engine.cpp -o orcs_engine.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence trace_reader.cpp -o trace_reader.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence package/opcode_package.cpp -o package/opcode_package.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence package/uop_package.cpp -o package/uop_package.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence package/memory_package.cpp -o package/memory_package.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence processor/memory_disambiguation/disambiguation_hashed.cpp -o processor/memory_disambiguation/disambiguation_hashed.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence processor/processor.cpp -o processor/processor.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence processor/reorder_buffer_line.cpp -o processor/reorder_buffer_line.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence processor/memory_order_buffer_line.cpp -o processor/memory_order_buffer_line.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence processor/instruction_set.cpp -o processor/instruction_set.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence utils/utils.cpp -o utils/utils.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence utils/enumerations.cpp -o utils/enumerations.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence branch_predictor/branch_predictor.cpp -o branch_predictor/branch_predictor.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence branch_predictor/piecewise.cpp -o branch_predictor/piecewise.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence cache/cache.cpp -o cache/cache.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence cache/cache_manager.cpp -o cache/cache_manager.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence hive/hive_controller.cpp -o hive/hive_controller.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence vima/vima_controller.cpp -o vima/vima_controller.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence vima/vima_vector.cpp -o vima/vima_vector.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence prefetcher/prefetcher.cpp -o prefetcher/prefetcher.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence prefetcher/stride_prefetcher.cpp -o prefetcher/stride_prefetcher.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence config/config.cpp -o config/config.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence main_memory/memory_channel.cpp -o main_memory/memory_channel.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence main_memory/memory_controller.cpp -o main_memory/memory_controller.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O1 -pedantic -fsanitize=leak -Wno-stringop-truncation -lefence main_memory/memory_request_client.cpp -o main_memory/memory_request_client.o
g++ -ggdb3 -o orcs simulator.o orcs_engine.o trace_reader.o package/opcode_package.o package/uop_package.o package/memory_package.o processor/memory_disambiguation/disambiguation_hashed.o processor/processor.o processor/reorder_buffer_line.o processor/memory_order_buffer_line.o processor/instruction_set.o utils/utils.o utils/enumerations.o branch_predictor/branch_predictor.o branch_predictor/piecewise.o cache/cache.o cache/cache_manager.o hive/hive_controller.o vima/vima_controller.o vima/vima_vector.o prefetcher/prefetcher.o prefetcher/stride_prefetcher.o config/config.o main_memory/memory_channel.o main_memory/memory_controller.o main_memory/memory_request_client.o -lz -lconfig++
```

### Creating traces
In order to create OrCS traces, its necessary to install the correct Pin tool version inside the OrCS directory. The Pin tool is a dynamic binary instrumentation framework that allows the creation of traces.
The correct Pin version is available [here](https://drive.google.com/file/d/1UEsOS00owGh-W0f4nZWRwlkaY4AKK9eE/view?usp=drive_link).

After downloading this file, it must be placed inside the **trace_generator** folder.
Then, run the following command, in order to extract the Pin tool and compile the tracer files:

```bash
$ ./install_tracer.sh
```

With the Pin tool installed, we can now create the traces. To do this, navigate to the folder: **trace_generator/extras/pinplay/sinuca_tracer** and run the following command:

```bash 
$ ../../../pin -t ../bin/intel64/sinuca_tracer.so -trace x86 -- ​<<Caminho até o executável>>
```

For example:
```bash
$ ../../../pin -t ../bin/intel64/sinuca_tracer.so -trace x86 -- /bin/ls
```

This command will generate three files in the same directory as the executable. These files are the traces that will be used by OrCS.
```bash
output_trace.out.tid0.dyn.out.gz
output_trace.out.tid0.mem.out.gz
output_trace.out.tid0.stat.out.gz
```

For more advanced traces, check the **docs** folder tutorials.

## Running OrCS
After creating a simulation trace, move the compressed files into the OrCS directory:
```bash
$ mv path/to_the/trace/application_name.tid0.dyn.out.gz path/to_orcs/orcs/
$ mv path/to_the/trace/application_name.tid0.mem.out.gz path/to_orcs/orcs/
$ mv path/to_the/trace/application_name.tid0.stat.out.gz path/to_orcs/orcs/
```

Then execute OrCS in terminal:
```bash
$ ./orcs -t <application_name> -c sandy_bridge/sandy_bridge.cfg
```

For example:
```bash
$ ./orcs -c configuration_files/skylakeServer.cfg -t output_trace.out
```

The output will contain statistics about the application's execution in OrCS which can be used for architecture evaluation:

```
Global_Statistics
#===============================================#
Global_Cycle: 586522
Global_IPC: 0.965122
Elapsed Time (00:00:10)
KIPS: 56.606400
#===============================================#
#===============================================#
Statistics of Core 0
#========================================================================#
trace_reader_t
fetch_instructions: 566064
#========================================================================#
Escritas enviadas: 57882
Reads MSHR stall: 113
Write MSHR stall: 611
Inst MSHR stall: 102
#========================================================================#
Total_Cycle:  586521
#===============================================#
Stage_Opcode_and_Uop_Counters
#===============================================#
Stage_Fetch:  566065
Stage_Decode: 566065
Stage_Rename: 565211
Stage_Commit: 565210
#========================================================================#
Instruction_Per_Cycle:            0.965123
MPKI:                             9.601371
Average_wait_cycles_wait_mem_req: 96.234688
Core_Request_RAM_AVG_Cycle:       -nan
Total_Load_Requests:              139497
Total_Store_Requests:             57882
Total_HIVE_Instructions:          0
Total_VIMA_Instructions:          0
#========================================================================#
Stalls Fetch:          197197
Stalls Decode:          141268
Stalls Rename:          114014
#========================================================================#
Dependencies created:          711142
Calls for dependencies creation:          565210
#========================================================================#
Total_OP_NOP   _Instructions:         7698
Total_OP_NOP   _Instructions_Latency: 897833
Avg._OP_NOP   _Instructions_Latency:  116 (897833/7698)
Max_OP_NOP   _Instructions_Latency:   665
Min_OP_NOP   _Instructions_Latency:   9
Total_OP_IN_ALU_Instructions:         254737
Total_OP_IN_ALU_Instructions_Latency: 43483393
Avg._OP_IN_ALU_Instructions_Latency:  170 (43483393/254737)
Max_OP_IN_ALU_Instructions_Latency:   850
Min_OP_IN_ALU_Instructions_Latency:   9
Total_OP_IN_MUL_Instructions:         889
Total_OP_IN_MUL_Instructions_Latency: 24596
Avg._OP_IN_MUL_Instructions_Latency:  27 (24596/889)
Max_OP_IN_MUL_Instructions_Latency:   264
Min_OP_IN_MUL_Instructions_Latency:   11
Total_OP_IN_DIV_Instructions:         8615
Total_OP_IN_DIV_Instructions_Latency: 1681771
Avg._OP_IN_DIV_Instructions_Latency:  195 (1681771/8615)
Max_OP_IN_DIV_Instructions_Latency:   643
Min_OP_IN_DIV_Instructions_Latency:   9
Total_OP_FP_ALU_Instructions:         7238
Total_OP_FP_ALU_Instructions_Latency: 1388873
Avg._OP_FP_ALU_Instructions_Latency:  191 (1388873/7238)
Max_OP_FP_ALU_Instructions_Latency:   598
Min_OP_FP_ALU_Instructions_Latency:   9
Total_OP_FP_DIV_Instructions:         85
Total_OP_FP_DIV_Instructions_Latency: 5812
Avg._OP_FP_DIV_Instructions_Latency:  68 (5812/85)
Max_OP_FP_DIV_Instructions_Latency:   372
Min_OP_FP_DIV_Instructions_Latency:   9
Total_OP_BRANCH_Instructions:         125536
Total_OP_BRANCH_Instructions_Latency: 19937960
Avg._OP_BRANCH_Instructions_Latency:  158 (19937960/125536)
Max_OP_BRANCH_Instructions_Latency:   849
Min_OP_BRANCH_Instructions_Latency:   9
Total_OP_LOAD  _Instructions:         103961
Total_OP_LOAD  _Instructions_Latency: 19748464
Avg._OP_LOAD  _Instructions_Latency:  189 (19748464/103961)
Max_OP_LOAD  _Instructions_Latency:   849
Min_OP_LOAD  _Instructions_Latency:   10
Total_OP_STORE _Instructions:         53631
Total_OP_STORE _Instructions_Latency: 9872078
Avg._OP_STORE _Instructions_Latency:  184 (9872078/53631)
Max_OP_STORE _Instructions_Latency:   850
Min_OP_STORE _Instructions_Latency:   9
Total_OP_OTHER _Instructions:         2820
Total_OP_OTHER _Instructions_Latency: 140377
Avg._OP_OTHER _Instructions_Latency:  49 (140377/2820)
Max_OP_OTHER _Instructions_Latency:   431
Min_OP_OTHER _Instructions_Latency:   9
#===============================================#
#Memory Disambiguation
#===============================================#
Total_Read_false_Positives:       16314
Total_Write_false_Positives:      15194
Total_Resolve_Address_to_Address: 9830
#===============================================#
Inst_requests: 566064
#========================================================================#
#Branch Prodiction
#========================================================================#
BTB Hits:                    111100
BTB Miss:                    3801
Total Branchs:               114901
Total Branchs Taken:         39354
Total Branchs Not Taken:     75547
Correct Branchs Taken:       36264
Incorrect Branchs Taken:     3090
Correct Branchs Not Taken:   38180
Incorrect Branchs Not Taken: 37367
#========================================================================#
#========================================================================#
#Cache Manager
#========================================================================#
Total Reads:                       129667
Total Writes:                      57882
Total RAM requests:                5435
Total RAM request latency cycles:  894567
Avg. wait for RAM requests:        164
Min. wait for RAM requests:        125
Max. wait for RAM requests:        375
#========================================================================#
READ _Tot._Latency_Hierarchy:  860255
READ _Avg._Latency_Hierarchy:  9 (860255/90351)
READ _Min._Latency_Hierarchy:  5
READ _Max._Latency_Hierarchy:  285
WRITE_Tot._Latency_Hierarchy:  313656
WRITE_Avg._Latency_Hierarchy:  11 (313656/27094)
WRITE_Min._Latency_Hierarchy:  5
WRITE_Max._Latency_Hierarchy:  375
INST _Tot._Latency_Hierarchy:  824271
INST _Avg._Latency_Hierarchy:  7 (824271/105329)
INST _Min._Latency_Hierarchy:  5
INST _Max._Latency_Hierarchy:  267
Already there (INST): 460735
Already there (READ): 39316
Already there (WRITE): 30788
#========================================================================#
#L1 INSTRUCTION Cache
#========================================================================#
0_Cache_Access:       105329
0_Cache_Hits:         103363
0_Cache_Miss:         1966
0_Cache_Eviction:     1966
0_Cache_Read:         0
0_Cache_Write:        0
0_Cache_Inst:         105329
0_Cache_Inst_Hit:     103363
0_Cache_Inst_Miss:    1966
0_Avg._INST _Latency: 6
0_Min._INST _Latency: 4
0_Max._INST _Latency: 266
0_MSHR_Max_Reached:   10
#========================================================================#
#L1 DATA Cache
#========================================================================#
0_Cache_Access:       117445
0_Cache_Hits:         112566
0_Cache_Miss:         4879
0_Cache_Eviction:     4879
0_Cache_Read:         90351
0_Cache_Read_Hit:     86632
0_Cache_Read_Miss:    3719
0_Cache_Write:        27094
0_Cache_Write_Hit:    25934
0_Cache_Write_Miss:   1160
0_Cache_Inst:         0
0_Avg._READ _Latency: 8
0_Min._READ _Latency: 4
0_Max._READ _Latency: 284
0_Avg._WRITE_Latency: 10
0_Min._WRITE_Latency: 4
0_Max._WRITE_Latency: 374
0_Cache_WriteBack:    1272
0_MSHR_Max_Reached:   20
#========================================================================#
#L2 DATA Cache
#========================================================================#
1_Cache_Access:       6845
1_Cache_Hits:         1410
1_Cache_Miss:         5435
1_Cache_Eviction:     5435
1_Cache_Read:         3719
1_Cache_Read_Hit:     1215
1_Cache_Read_Miss:    2504
1_Cache_Write:        1160
1_Cache_Write_Hit:    73
1_Cache_Write_Miss:   1087
1_Cache_Inst:         1966
1_Cache_Inst_Hit:     122
1_Cache_Inst_Miss:    1844
1_Avg._READ _Latency: 109
1_Min._READ _Latency: 12
1_Max._READ _Latency: 280
1_Avg._WRITE_Latency: 153
1_Min._WRITE_Latency: 12
1_Max._WRITE_Latency: 370
1_Avg._INST _Latency: 151
1_Min._INST _Latency: 12
1_Max._INST _Latency: 262
1_MSHR_Max_Reached:   24
#========================================================================#
#L3 DATA Cache
#========================================================================#
2_Cache_Access:       5435
2_Cache_Hits:         0
2_Cache_Miss:         5435
2_Cache_Eviction:     5435
2_Cache_Read:         2504
2_Cache_Read_Miss:    2504
2_Cache_Write:        1087
2_Cache_Write_Miss:   1087
2_Cache_Inst:         1844
2_Cache_Inst_Miss:    1844
2_Avg._READ _Latency: 145
2_Min._READ _Latency: 108
2_Max._READ _Latency: 268
2_Avg._WRITE_Latency: 151
2_Min._WRITE_Latency: 109
2_Max._WRITE_Latency: 358
2_Avg._INST _Latency: 148
2_Min._INST _Latency: 114
2_Max._INST _Latency: 250
2_Cache_WriteBack:    13
2_MSHR_Max_Reached:   23
#===============================================#
#========================================================================#
#Memory Controller
#========================================================================#
Requests_Made:               5448
Requests_from_LLC:           5435
Row_Buffer_Hit,  Channel  0: 46
Row_Buffer_Miss, Channel  0: 132
Row_Buffer_Hit,  Channel  1: 62
Row_Buffer_Miss, Channel  1: 131
Row_Buffer_Hit,  Channel  2: 60
Row_Buffer_Miss, Channel  2: 140
Row_Buffer_Hit,  Channel  3: 51
Row_Buffer_Miss, Channel  3: 150
Row_Buffer_Hit,  Channel  4: 45
Row_Buffer_Miss, Channel  4: 162
Row_Buffer_Hit,  Channel  5: 45
Row_Buffer_Miss, Channel  5: 154
Row_Buffer_Hit,  Channel  6: 40
Row_Buffer_Miss, Channel  6: 146
Row_Buffer_Hit,  Channel  7: 47
Row_Buffer_Miss, Channel  7: 126
Row_Buffer_Hit,  Channel  8: 49
Row_Buffer_Miss, Channel  8: 121
Row_Buffer_Hit,  Channel  9: 38
Row_Buffer_Miss, Channel  9: 136
Row_Buffer_Hit,  Channel 10: 51
Row_Buffer_Miss, Channel 10: 145
Row_Buffer_Hit,  Channel 11: 33
Row_Buffer_Miss, Channel 11: 129
Row_Buffer_Hit,  Channel 12: 36
Row_Buffer_Miss, Channel 12: 119
Row_Buffer_Hit,  Channel 13: 44
Row_Buffer_Miss, Channel 13: 122
Row_Buffer_Hit,  Channel 14: 33
Row_Buffer_Miss, Channel 14: 113
Row_Buffer_Hit,  Channel 15: 33
Row_Buffer_Miss, Channel 15: 105
Row_Buffer_Hit,  Channel 16: 41
Row_Buffer_Miss, Channel 16: 120
Row_Buffer_Hit,  Channel 17: 26
Row_Buffer_Miss, Channel 17: 126
Row_Buffer_Hit,  Channel 18: 46
Row_Buffer_Miss, Channel 18: 126
Row_Buffer_Hit,  Channel 19: 34
Row_Buffer_Miss, Channel 19: 123
Row_Buffer_Hit,  Channel 20: 37
Row_Buffer_Miss, Channel 20: 122
Row_Buffer_Hit,  Channel 21: 29
Row_Buffer_Miss, Channel 21: 117
Row_Buffer_Hit,  Channel 22: 27
Row_Buffer_Miss, Channel 22: 129
Row_Buffer_Hit,  Channel 23: 33
Row_Buffer_Miss, Channel 23: 125
Row_Buffer_Hit,  Channel 24: 31
Row_Buffer_Miss, Channel 24: 127
Row_Buffer_Hit,  Channel 25: 37
Row_Buffer_Miss, Channel 25: 131
Row_Buffer_Hit,  Channel 26: 36
Row_Buffer_Miss, Channel 26: 131
Row_Buffer_Hit,  Channel 27: 43
Row_Buffer_Miss, Channel 27: 139
Row_Buffer_Hit,  Channel 28: 42
Row_Buffer_Miss, Channel 28: 116
Row_Buffer_Hit,  Channel 29: 49
Row_Buffer_Miss, Channel 29: 122
Row_Buffer_Hit,  Channel 30: 42
Row_Buffer_Miss, Channel 30: 129
Row_Buffer_Hit,  Channel 31: 54
Row_Buffer_Miss, Channel 31: 118
Row_Buffer Total_Hits:       1320
Row_Buffer_Total_Misses:     4132
Row_Buffer_Miss_Ratio:       0.758443
READ _Tot._Latency:          248687
READ _Avg._Latency:          99
READ _Min._Latency:          62
READ _Max._Latency:          92
WRITE_Tot._Latency:          115413
WRITE_Avg._Latency:          104
WRITE_Min._Latency:          63
WRITE_Max._Latency:          91
INST _Tot._Latency:          189207
INST _Avg._Latency:          102
INST _Min._Latency:          68
INST _Max._Latency:          128
#========================================================================#
Deleting Trace Reader
Instructions by reads:
0 reads -- 19660 instructions
1 reads -- 6178 instructions
Deleting Branch predictor
Deleting Cache manager
Deleting HIVE Controller
Deleting VIMA Controller
Deleting Memory Controller
Deleting Processor
```

## Development
To keep all branches correct and a functional master, we must follow some versioning rules.

### For direct contributors
* The branch `master` must have the stable version;
* The branch `develop` is the intermediary branch. It must keep a functional version that will be sent to the `master` only when it is stable;
* The other branches are "personal" branches, they will be used to the contributors individual features but must be aligned with `develop`;
* Remember: only merge your branch in `develop` when you already merged the `develop` branch in yours and ensure the results are stable;
* Keep verifying `develop` branch to keep your branch updated.

#### Creating a new branch
* To create a new personal branch, you may start from `develop` branch:
```bash
$ git checkout develop
$ git pull
$ git checkout -b your_branch_name develop
```

#### Incorporating a finished feature in `develop`
Every contributor must keep its own branch updated according to the `develop` branch. It is therefore necessary that every contributor updates the `develop` branch frequently following the steps below:

* After modifying the code in your local branch with success (without compiling and execution errors) you may commit the modified files:
```bash
$ git add <files>
$ git commit -m "modifications in my branch"
```

* With the modifications of your local branch commited, switch to the local `develop` branch and fetch the remote version:
```bash 
$ git checkout develop
$ git pull
```

* Switch back to your local branch and merge it with the updated content in `develop`:
```bash
$ git checkout your_branch
$ git merge develop
```

* Fix the merge conflicts and test your code in local branch. When it is flawless (without compiling, execution, and behavior errors) you can update your remote branch:
```bash
$ git add <files>
$ git commit
$ git push
```
* Again switch to the local `develop` branch to merge it with your branch modifications:
```bash
$ git checkout develop
$ git merge --no-ff your_branch
```

* Finally update the `develop` remote branch:
```bash
$ git add <files>
$ git commit -m "develop merged with your_branch"
$ git push
```
#### After incorporating a feature
The contributor can delete the branch related to the incorporated feature:
```bash
$ git branch -d your_branch
```

## Authors
OrCS was developed in HiPES Lab (High Performance and Efficient Systems) in Federal University of Paraná:
* [Aline Santana Cordeiro](https://github.com/ascordeiro)
* [Marco Antonio Zanata Alves](https://github.com/mazalves/)
* [Ricardo Kohler](https://github.com/kohlerricardo)
* [Sairo Raoní dos Santos](https://github.com/sairosantos)

## Acknowledgments
OrCS is based on [SiNUCA](https://github.com/mazalves/sinuca) (Simulator of Non Uniform Caches) and was developed with the need of a simple and faster simulator which can be easily modified by the researchers.
