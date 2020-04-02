//
// This tool prints a trace of image load and unload events
//
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pin.H"

FILE * trace;
PIN_LOCK lock;

KNOB<std::string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "output_library_call.out", "specify the trace base file name.");


//==============================================================================
VOID called_rtn(char *sync_str, THREADID threadid) {
    PIN_GetLock(&lock, threadid);
    fprintf(trace, "#CALLED:%s THREADID:%d \n", sync_str, threadid);
    PIN_ReleaseLock(&lock);
};

// Pin calls this function every time a new img is loaded
// It can instrument the image, but this example does not
// Note that imgs (including shared libraries) are loaded lazily
VOID ImageLoad(IMG img, VOID *v)
{
    fprintf(trace, "Loading %s, Image id = %d\n", IMG_Name(img).c_str(), IMG_Id(img));
    std::string rtn_name;

    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec)) {
        // RTN_InsertCall() and INS_InsertCall() are executed in order of
        // appearance.  The IPOINT_AFTER may be executed before the IPOINT_BEFORE.
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn)) {
            rtn_name = RTN_Name(rtn);
            RTN_Open(rtn);

            // ~ fprintf(trace, "#MAPPED:\"%s\"\n", rtn_name.c_str());

            // ~ char *sync_str = new char[200];
            // ~ sprintf(sync_str, "RTN\"%s\"", rtn_name.c_str());
            // ~ RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(called_rtn), IARG_PTR, sync_str, IARG_THREAD_ID, IARG_END);


            // Examine each instruction in the routine.
            for(INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins) )
            {
                if (strcmp(INS_Mnemonic(ins).c_str(), "PAUSE") != 0) continue;

                char *sync_str = new char[200];
                sprintf(sync_str, "RTN\"%s\" - INS\"%s\"", rtn_name.c_str(), INS_Mnemonic(ins).c_str());
                INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(called_rtn), IARG_PTR, sync_str, IARG_THREAD_ID, IARG_END);
            }

            RTN_Close(rtn);
        }
    }
}

// Pin calls this function every time a new img is unloaded
// You can't instrument an image that is about to be unloaded
VOID ImageUnload(IMG img, VOID *v)
{
    fprintf(trace, "Unloading %s\n", IMG_Name(img).c_str());
}

// This function is called when the application exits
// It closes the output file.
VOID Fini(INT32 code, VOID *v)
{
    fclose(trace);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    PIN_ERROR("This tool prints a log of image load and unload events\n"
             + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    PIN_InitLock(&lock);
    trace = fopen(KnobOutputFile.Value().c_str(), "w");

    // Initialize symbol processing
    PIN_InitSymbols();

    // Register ImageLoad to be called when an image is loaded
    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Register ImageUnload to be called when an image is unloaded
    IMG_AddUnloadFunction(ImageUnload, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return EXIT_SUCCESS;
}
