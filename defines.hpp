#ifndef DEFINES_H
#define DEFINES_H

// #define NUMBER_OF_PROCESSORS 1

#define SANITY_CHECK 0
#define HEARTBEAT 1
#define HEARTBEAT_CLOCKS 10000000

#define KILO 1024
#define MEGA KILO*KILO
// ============================================================================
/// Definitions for Log, Debug, Warning, Error and Statistics
// ============================================================================
#define POSITION_FAIL -1        /// FAIL when return is int32_t
#define FAIL 0                  /// FAIL when return is int32_t or uint32_t
#define OK 1                    /// OK when return is int32_t or uint32_t
#define NOT_ALL_REGS 2
#define TRACE_LINE_SIZE 512

// ========================
// Defines Simulators Caracteristics
// ========================
#define MAX_UOP_DECODED 5
#define MAX_REGISTERS 32         /// opcode_package_t uop_package_t  (Max number of register (read or write) for one opcode/uop)
#define MAX_ASSEMBLY_SIZE 32    /// In general 20 is enough
#define MAX_REGISTER_NUMBER 256
// ========================

#endif // DEFINES_H