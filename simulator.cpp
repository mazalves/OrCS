#include "./simulator.hpp"

orcs_engine_t orcs_engine;

// =============================================================================
static void display_use() {
    ORCS_PRINTF("**** OrCS - Ordinary Computer Simulator ****\n\n");
    ORCS_PRINTF("**** Options ****\n\n");
    ORCS_PRINTF("-t [trace file name]\n\n");
    ORCS_PRINTF("-f [output file name]\n\n");
    ORCS_PRINTF("-c [number of cores]\n\n");
    ORCS_PRINTF("Please provide -c <number of cores> [-t <trace_file_basename>] -f <output filename>\n");
}

// =============================================================================
static uint32_t process_argv(int argc, char **argv) {

    // Name, {no_argument, required_argument and optional_argument}, flag, value
    static struct option long_options[] = {
        {"help",        no_argument, 0, 'h'},
        {"core",        required_argument, 0, 'c'},
        {"trace",       required_argument, 0, 't'},
        {"output_filename",       optional_argument, 0, 'f'},
        {"use_pin", optional_argument, 0, 'p'},
        {NULL,          0, NULL, 0}
    };

    // Count number of traces
    int opt;
    int option_index = 0;
    uint32_t traces_informados = 0;
    
    while ((opt = getopt_long_only(argc, argv, "h:c:t:f:pw:",
                 long_options, &option_index)) != -1) {
        switch (opt) {
        case 0:
            printf ("Option %s", long_options[option_index].name);
            if (optarg)
                printf (" with arg %s", optarg);
            printf ("\n");
            break;
        case 'h':
            display_use();
            break;
        case 'c':
            orcs_engine.config_file = optarg;
            break;
        case 't':
            orcs_engine.arg_trace_file_name.push_back(optarg);
            traces_informados++;
            break;
        case 'f':
            orcs_engine.output_file_name = optarg;
            break;
        case 'p':
            orcs_engine.use_pin = true;
            break;
        case '?':
            break;

        default:
            ORCS_PRINTF(">> getopt returned character code 0%o ??\n", opt);
        }
    }

    if (optind < argc) {
        ORCS_PRINTF("Non-option ARGV-elements: ");
        while (optind < argc)
            ORCS_PRINTF("%s ", argv[optind++]);
        ORCS_PRINTF("\n");
    }

    orcs_engine.configuration = new configure_t();
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    uint32_t NUMBER_OF_PROCESSORS = cfg_root["PROCESSOR"].getLength();

    utils_t::process_mem_usage(&orcs_engine.stat_vm_start, &orcs_engine.stat_rss_start);
    if(orcs_engine.use_pin == false) {
        // ORCS_PRINTF ("traces_informados = %u, NUMBER_OF_PROCESSORS = %u\n", traces_informados, NUMBER_OF_PROCESSORS)
        ERROR_ASSERT_PRINTF(traces_informados==NUMBER_OF_PROCESSORS,"Erro, Numero de traces informados diferente do numero de cores\n")
        if (orcs_engine.arg_trace_file_name.empty()) {
            ORCS_PRINTF("Trace file not defined.\n");
            display_use();
        }
    } else {
        ERROR_ASSERT_PRINTF(NUMBER_OF_PROCESSORS == 1,"Error, Pin can only be used in a system with one processor\n")
    }

    return NUMBER_OF_PROCESSORS;
}

std::string get_status_execution(uint32_t NUMBER_OF_PROCESSORS){   
    std::string final_report;
    char report[1000];
    // Data - Atual,total, active cores
    // ================================================================
    uint64_t ActualLength = 0;
    uint64_t FullLength = 0;
    uint32_t active_cores = 0;
    // ================================================================
    // Get infos about total execution
    snprintf(report,sizeof(report),"%s","==========================================================================\n");
    final_report+=report;
    gettimeofday(&orcs_engine.stat_timer_end, NULL);
    double seconds_spent = orcs_engine.stat_timer_end.tv_sec - orcs_engine.stat_timer_start.tv_sec;
    
    utils_t::process_mem_usage(&orcs_engine.stat_vm_allocate, &orcs_engine.stat_rss_allocate);
    
    /// Get global statistics from all the cores
    for (uint32_t cpu = 0 ; cpu < NUMBER_OF_PROCESSORS ; cpu++) {
        ActualLength += orcs_engine.trace_reader[cpu].get_fetch_instructions();
        FullLength += orcs_engine.trace_reader[cpu].get_trace_opcode_max() + 1;
        active_cores += orcs_engine.processor[cpu].isBusy();
    }
    // compute status of full simulation
    snprintf(report,sizeof(report),"Active Cores: %u of %u\n",active_cores,NUMBER_OF_PROCESSORS);
    final_report+=report;
    snprintf(report,sizeof(report),"Opcodes Processed: %lu of %lu\n",ActualLength,FullLength);
    final_report+=report;
    // Setting progress
    double percentage_complete = 100.0 * (static_cast<double>(ActualLength) / static_cast<double>(FullLength));
    snprintf(report,sizeof(report),"Total Progress %8.4lf%%\n",percentage_complete);
    final_report+=report;
    // IPC parcial
    snprintf(report,sizeof(report),"Global IPC(%1.6lf)\n", static_cast<double>(ActualLength) / static_cast<double>(orcs_engine.get_global_cycle()));
    final_report+=report;    
    //    
    double seconds_remaining = (100*(seconds_spent / percentage_complete)) - seconds_spent;
        snprintf(report,sizeof(report), "Global ETC(%02.0f:%02.0f:%02.0f)\n",
                                                floor(seconds_remaining / 3600.0),
                                                floor(fmod(seconds_remaining, 3600.0) / 60.0),
                                                fmod(seconds_remaining, 60.0));
    final_report+=report;
    // End of card
    snprintf(report,sizeof(report),"%s","==========================================================================\n");
    final_report+=report;
    // Get statistics from each core

    double kilo_instructions_simulated = 0;
    for (uint32_t cpu = 0 ; cpu < NUMBER_OF_PROCESSORS ; cpu++) {
        snprintf(report,sizeof(report),"======================Processor %u of %u===============================\n",cpu+1, NUMBER_OF_PROCESSORS);
        final_report+=report;
        // Get benchmark name
        snprintf(report,sizeof(report),"Benchmark %s\n",(orcs_engine.use_pin) 
                                                        ? "PIN" : orcs_engine.arg_trace_file_name[cpu].c_str());
        final_report+=report;

        kilo_instructions_simulated += orcs_engine.trace_reader[cpu].get_fetch_instructions() / 1000.0;
        ActualLength = orcs_engine.trace_reader[cpu].get_fetch_instructions();
        FullLength = orcs_engine.trace_reader[cpu].get_trace_opcode_max() + 1;
        // get actual cicle
        snprintf(report,sizeof(report),"Actual Cycle %lu\n",orcs_engine.get_global_cycle());
        final_report+=report;
        snprintf(report,sizeof(report),"Opcodes Processed: %lu of %lu\n",ActualLength,FullLength);
        final_report+=report;
        // Get  status opcodes total, executed -> calculate percent / CORE
        uint64_t total_opcodes = orcs_engine.trace_reader[cpu].get_trace_opcode_max();
        uint64_t fetched_opcodes = orcs_engine.trace_reader[cpu].get_fetch_instructions();

        snprintf(report,sizeof(report),"Uops Fetched/Uops Total: %lu of %lu\n",fetched_opcodes,total_opcodes);
        final_report+=report;
        // Get total uops decoded, uops coppleted
        uint64_t uops_decoded = orcs_engine.processor[cpu].renameCounter;
        uint64_t uop_completed = orcs_engine.processor[cpu].commit_uop_counter;
        snprintf(report,sizeof(report),"Uops Completed/uops decoded: %lu of %lu\n",uop_completed,uops_decoded);
        final_report+=report;
        ////////
        double percentage_complete = 100.0 * (static_cast<double>(fetched_opcodes) / static_cast<double>(total_opcodes));
        snprintf(report,sizeof(report),"Total Progress %8.4lf%%\n",percentage_complete);
        final_report+=report;
        // IPC parcial
        snprintf(report,sizeof(report),"IPC(%1.6lf)\n", static_cast<double>(fetched_opcodes) / static_cast<double>(orcs_engine.get_global_cycle()));
        final_report+=report;
        double seconds_remaining = (100*(seconds_spent / percentage_complete)) - seconds_spent;
        snprintf(report,sizeof(report), "ETC(%02.0f:%02.0f:%02.0f)\n",
                                                floor(seconds_remaining / 3600.0),
                                                floor(fmod(seconds_remaining, 3600.0) / 60.0),
                                                fmod(seconds_remaining, 60.0));
        final_report+=report;
    }
    snprintf (report, sizeof(report), "KIPS(%lf)\n", static_cast<double> (kilo_instructions_simulated/seconds_spent));
    final_report+=report;
    snprintf(report,sizeof(report), "Elapsed Time (%02.0f:%02.0f:%02.0f)\n",
                                                floor(seconds_spent / 3600.0),
                                                floor(fmod(seconds_spent, 3600.0) / 60.0),
                                                fmod(seconds_spent, 60.0));
    final_report+=report;
    snprintf(report,sizeof(report),"%s","==========================================================================\n");
    final_report+=report;
    return final_report;
}

// =============================================================================
int main(int argc, char **argv) {

    // process args
    uint32_t NUMBER_OF_PROCESSORS = process_argv(argc, argv);

    /// Call all the allocate's
    orcs_engine.allocate(NUMBER_OF_PROCESSORS);

    //==================
    //Cache Manager
    //==================
    orcs_engine.cacheManager->allocate(NUMBER_OF_PROCESSORS);
    //==================
    //Memory Controller
    //==================
    orcs_engine.memory_controller->allocate();

    for (uint32_t i = 0; i < NUMBER_OF_PROCESSORS; i++){
        //==================
        //trace_reader
        //==================
        orcs_engine.trace_reader[i].set_processor_id(i);
        if(orcs_engine.use_pin == false)
        {
            orcs_engine.trace_reader[i].allocate((char*)orcs_engine.arg_trace_file_name[i].c_str());
        }else {
            orcs_engine.trace_reader[i].allocate(NULL);
        }
        // Allocate structures to all cores
        //==================
        //Processor
        //==================
        orcs_engine.processor[i].allocate();
        orcs_engine.processor[i].set_processor_id(i);        
        //==================
        //Branch Predictor
        //==================
        orcs_engine.branchPredictor[i].allocate (i);
    }

    if (orcs_engine.processor->get_HAS_HIVE()) orcs_engine.hive_controller->allocate();
    if (orcs_engine.processor->get_HAS_VIMA()) orcs_engine.vima_controller->allocate();
    //initializate simulator
    orcs_engine.simulator_alive = true;

    /// Start CLOCK for all the components
    while (orcs_engine.get_simulation_alive(NUMBER_OF_PROCESSORS)) {
        #if HEARTBEAT
            if(orcs_engine.get_global_cycle()%HEARTBEAT_CLOCKS==0){
                gettimeofday(&orcs_engine.stat_timer_end, NULL);
                ORCS_PRINTF("%s\n",get_status_execution(NUMBER_OF_PROCESSORS).c_str())
            }
        #endif
        orcs_engine.memory_controller->clock();
        for (uint32_t i = 0; i < NUMBER_OF_PROCESSORS; i++)
        {
            orcs_engine.processor[i].clock();
        }
        
        orcs_engine.global_cycle++;
    }
    // *****************************************************************************************
	ORCS_PRINTF("End of Simulation\n")
	ORCS_PRINTF("Writting FILE\n")
    uint64_t FullLength = 0;
    gettimeofday(&orcs_engine.stat_timer_end, NULL);
    bool memory_leak_warning = false;
    double kilo_instructions_simulated = 0;
    double seconds_spent = orcs_engine.stat_timer_end.tv_sec - orcs_engine.stat_timer_start.tv_sec;
    utils_t::process_mem_usage(&orcs_engine.stat_vm_end, &orcs_engine.stat_rss_end);
    if (orcs_engine.stat_vm_end > orcs_engine.stat_vm_allocate + 10) {
        memory_leak_warning = true;
    }
    for (uint32_t cpu = 0; cpu < NUMBER_OF_PROCESSORS; cpu++){
        FullLength += orcs_engine.trace_reader[cpu].get_trace_opcode_max() + 1; 
        kilo_instructions_simulated += orcs_engine.trace_reader[cpu].get_fetch_instructions() / 1000.0;
    }
    
    if(orcs_engine.use_pin)
    {
        FullLength += orcs_engine.trace_reader[0].get_fetch_instructions(); 

    }

    FILE *output = stdout;
    bool close = false;
    if(orcs_engine.output_file_name != NULL){
        output = fopen(orcs_engine.output_file_name,"a+");
        close=true;
    }
    if (output != NULL){
        fprintf(output,"Global_Statistics\n");
        utils_t::largeSeparator(output);
        fprintf(output,"Global_Cycle: %lu\n",orcs_engine.get_global_cycle());
        fprintf(output,"Global_IPC: %2.6lf\n", static_cast<double>(FullLength) / static_cast<double>(orcs_engine.get_global_cycle()));
        fprintf(output,"Elapsed Time (%02.0f:%02.0f:%02.0f)\n",
                                                floor(seconds_spent / 3600.0),
                                                floor(fmod(seconds_spent, 3600.0) / 60.0),
                                                fmod(seconds_spent, 60.0));
        fprintf(output,"KIPS: %lf\n", static_cast<double> (kilo_instructions_simulated/seconds_spent));   
        if (memory_leak_warning) fprintf(output,"Check for Memory Leak!\n");   
        utils_t::largeSeparator(output);
    }
    if(close) fclose(output);
    for (uint32_t i = 0; i < NUMBER_OF_PROCESSORS; i++)
    {
        bool close = false;
        if(orcs_engine.output_file_name != NULL){
            output = fopen(orcs_engine.output_file_name,"a+");
            close=true;
        }
        if (output != NULL){
            utils_t::largeSeparator(output);
            fprintf(output,"Statistics of Core %d\n",i);
            if(close)fclose(output);
            orcs_engine.trace_reader[i].statistics();
            orcs_engine.processor[i].statistics();
            orcs_engine.branchPredictor[i].statistics();
            orcs_engine.cacheManager->statistics(i);
            utils_t::largeSeparator(output);
        }
    }
    orcs_engine.memory_controller->statistics();    
    ORCS_PRINTF("Writed FILE\n")
    // *****************************************************************************************

    ORCS_PRINTF("Deleting Trace Reader\n")
    delete[] orcs_engine.trace_reader;
    delete orcs_engine.configuration;
    ORCS_PRINTF("Deleting Memory Controller\n")
    delete orcs_engine.memory_controller;
    ORCS_PRINTF("Deleting Branch predictor\n")
    delete[] orcs_engine.branchPredictor;
    ORCS_PRINTF("Deleting Cache manager\n")
    delete orcs_engine.cacheManager;
    ORCS_PRINTF("Deleting HIVE Controller\n")
    delete orcs_engine.hive_controller;
    ORCS_PRINTF ("Deleting VIMA Controller\n")
    delete orcs_engine.vima_controller;
    ORCS_PRINTF("Deleting Processor\n")
    delete[] orcs_engine.processor;
}
