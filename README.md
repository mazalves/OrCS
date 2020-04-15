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
rm -f simulator.o orcs_engine.o trace_reader.o package/opcode_package.o package/uop_package.o processor/memory_disambiguation/disambiguation_hashed.o processor/processor.o processor/reorder_buffer_line.o processor/memory_order_buffer_line.o utils/utils.o utils/enumerations.o branch_predictor/branch_predictor.o branch_predictor/piecewise.o cache/cache.o cache/cache_manager.o cache/mshr_entry.o directory/directory.o prefetcher/prefetcher.o prefetcher/stride_prefetcher.o config/config.o main_memory/memory_channel.o main_memory/memory_controller.o
rm -f orcs
OrCS cleaned\!

g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic simulator.cpp -o simulator.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic orcs_engine.cpp -o orcs_engine.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic trace_reader.cpp -o trace_reader.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic package/opcode_package.cpp -o package/opcode_package.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic package/uop_package.cpp -o package/uop_package.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic processor/memory_disambiguation/disambiguation_hashed.cpp -o processor/memory_disambiguation/disambiguation_hashed.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic processor/processor.cpp -o processor/processor.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic processor/reorder_buffer_line.cpp -o processor/reorder_buffer_line.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic processor/memory_order_buffer_line.cpp -o processor/memory_order_buffer_line.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic utils/utils.cpp -o utils/utils.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic utils/enumerations.cpp -o utils/enumerations.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic branch_predictor/branch_predictor.cpp -o branch_predictor/branch_predictor.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic branch_predictor/piecewise.cpp -o branch_predictor/piecewise.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic cache/cache.cpp -o cache/cache.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic cache/cache_manager.cpp -o cache/cache_manager.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic cache/mshr_entry.cpp -o cache/mshr_entry.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic directory/directory.cpp -o directory/directory.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic prefetcher/prefetcher.cpp -o prefetcher/prefetcher.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic prefetcher/stride_prefetcher.cpp -o prefetcher/stride_prefetcher.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic config/config.cpp -o config/config.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic main_memory/memory_channel.cpp -o main_memory/memory_channel.o
g++ -c -ggdb3 -g -Wall -Wextra -Werror -std=c++0x -lefence -O3 -pedantic main_memory/memory_controller.cpp -o main_memory/memory_controller.o
g++ -ggdb3 -o orcs simulator.o orcs_engine.o trace_reader.o package/opcode_package.o package/uop_package.o processor/memory_disambiguation/disambiguation_hashed.o processor/processor.o processor/reorder_buffer_line.o processor/memory_order_buffer_line.o utils/utils.o utils/enumerations.o branch_predictor/branch_predictor.o branch_predictor/piecewise.o cache/cache.o cache/cache_manager.o cache/mshr_entry.o directory/directory.o prefetcher/prefetcher.o prefetcher/stride_prefetcher.o config/config.o main_memory/memory_channel.o main_memory/memory_controller.o -lz -lconfig++ 
```

### Install OrCS + Tracer
Video tutorials showing OrCS+Sinuca_tracer download and configuration:
[Fast installation](https://asciinema.org/a/78yLzIg0GlXvUngoA8f0lx4Vb)
[Manual installation](https://asciinema.org/a/mn63H4OW4SY542Xs1p7seEL3M)

## Running OrCS
Before executing OrCS we must download a simulation trace.

### Simulation traces
Download at least one [simulation trace](https://drive.google.com/drive/folders/1wJIxye5Cm4cRu3pTYrKkWfPF94_9h5uN) because it will be needed during OrCS execution. Simulation traces replicate the computational behavior of a specific application, so these traces will be evaluated in OrCS similarly to the real application execution on a conventional computer. Each application directory has three different compressed files containing the simulation traces:

```
* application_name
    * application_name.tid0.dyn.out.gz
    * application_name.tid0.mem.out.gz
    * application_name.tid0.stat.out.gz
```

Move only the compressed files into the OrCS directory:
```bash
$ mv path/to_the/trace/application_name/application_name.tid0.dyn.out.gz path/to_orcs/orcs/
$ mv path/to_the/trace/application_name/application_name.tid0.mem.out.gz path/to_orcs/orcs/
$ mv path/to_the/trace/application_name/application_name.tid0.stat.out.gz path/to_orcs/orcs/
```

To execute OrCS in terminal:
```bash
$ ./orcs -t <application_name> -c sandy_bridge/sandy_bridge.cfg
```

The output will contain statistics about the application's execution in OrCS which can be used for architecture evaluation:

```
Global_Statistics
#===============================================#
Global_Cycle: 841359
Global_IPC: 0.653862
#===============================================#
#===============================================#
Statistics of Core 0
#========================================================================#
trace_reader_t
fetch_instructions: 550132
#========================================================================#
#========================================================================#
Total_Cycle: 841358
#===============================================#
Stage_Opcode_and_Uop_Counters
#===============================================#
Stage_Fetch: 550133
Stage_Decode: 550133
Stage_Rename: 587797
Stage_Commit: 587796
#========================================================================#
Times_Reach_MAX_PARALLEL_REQUESTS_CORE_READ: 7633
Times_Reach_MAX_PARALLEL_REQUESTS_CORE_WRITE: 2867
#========================================================================#
Instruction_Per_Cycle: 0.653863
MPKI: 10.859192
Average_wait_cycles_wait_mem_req: 16.002829
Core_Request_RAM_AVG_Cycle: 42.082443
#========================================================================#
#===============================================#
Total_Read_false_Positives: 0
Total_Write_false_Positives: 0
Total_Resolve_Address_to_Address: 0
#===============================================#
#========================================================================#
BTB Hits: 114741
BTB Miss: 3486
Total Branchs: 118227
Total Branchs Taken: 40416
Total Branchs Not Taken: 77811
Correct Branchs Taken: 36297
Incorrect Branchs Taken: 4119
Correct Branchs Not Taken: 38437
Incorrect Branchs Not Taken: 39374
#========================================================================#
#========================================================================#
##############  Cache Memories ##################
#========================================================================#
#===============================================#
Cache_Level: 0 - Cache_Type: 0
0_Cache_Access: 550132
0_Cache_Hits: 537243
0_Cache_Miss: 12889
0_Cache_Eviction: 1714
0_Cache_Read: 537243
0_Cache_Write: 0
#===============================================#
#===============================================#
Cache_Level: 0 - Cache_Type: 1
0_Cache_Access: 178499
0_Cache_Hits: 171621
0_Cache_Miss: 6878
0_Cache_Eviction: 19123
0_Cache_Read: 133957
0_Cache_Write: 44542
0_Cache_WriteBack: 2420
#===============================================#
#===============================================#
Cache_Level: 1 - Cache_Type: 1
1_Cache_Access: 19767
1_Cache_Hits: 13729
1_Cache_Miss: 6038
1_Cache_Eviction: 7108
1_Cache_Read: 18438
1_Cache_Write: 14604
1_Cache_WriteBack: 347
#===============================================#
#===============================================#
Cache_Level: 2 - Cache_Type: 1
2_Cache_Access: 6038
2_Cache_Hits: 64
2_Cache_Miss: 5974as we executed
2_Cache_Eviction: as we executed
2_Cache_Read: 4963as we executed
2_Cache_Write: 1129
2_Cache_WriteBack: 752
#===============================================#
#===============================================#
#========================================================================#
#Memory Controller
#========================================================================#
Requests_Made:              5974
Requests_from_Prefetcher:   0
Requests_from_LLC:          5974
Row_Buffer_Hit, Channel 0:  1186
Row_Buffer_Miss, Channel 0: 277
Row_Buffer_Hit, Channel 1:  1176
Row_Buffer_Miss, Channel 1: 261
Row_Buffer_Hit, Channel 2:  1213
Row_Buffer_Miss, Channel 2: 253
Row_Buffer_Hit, Channel 3:  1198
Row_Buffer_Miss, Channel 3: 270
#========================================================================#
Writed FILE
Deleting Trace Reader
Deleting Processor
Deleting Branch predictor
Deleting Cache manager
Deleting Memory Controller
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
