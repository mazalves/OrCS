#ifndef __ORCS_TRACER_DEFINES_hpp__
#define __ORCS_TRACER_DEFINES_hpp__
// ============================================================================

#define ERROR_INFORMATION() {\
                                printf("ERROR INFORMATION\n");\
                                printf("ERROR: File: %s at Line: %u\n", __FILE__, __LINE__);\
                                printf("ERROR: Function: %s\n", __PRETTY_FUNCTION__);\
                            }
// ============================================================================

#define ERROR_ASSERT_PRINTF(v, ...) if (!(v)) {\
                                        ERROR_INFORMATION();\
                                        printf("ERROR_ASSERT: %s\n", #v);\
                                        printf("\nERROR: ");\
                                        printf(__VA_ARGS__);\
                                        exit(EXIT_FAILURE);\
                                    }
// ============================================================================

#define ERROR_PRINTF(...) {\
                              ERROR_INFORMATION();\
                              printf("\nERROR: ");\
                              printf(__VA_ARGS__);\
                              exit(EXIT_FAILURE);\
                          }
// ============================================================================

#endif