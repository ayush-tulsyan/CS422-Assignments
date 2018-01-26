/*
 * Constants used
 */

const int NUM_INS_TYPE = 17;
const int NUM_FOOTPRINT_TYPE = 2;
const int NUM_MEMORY_BLOCKS = (1<<27);
const int MAX_INS_SIZE = 20;
const int MAX_NUM_OPERANDS = 10;
const int MAX_NUM_MEMOPS = 4;

enum InsType{
    LOADS,
    STORES,
	NOPS,
	DIRECT_CALLS,
	INDIRECT_CALLS,
	RETURNS,
	UNCONDITIONAL_BRANCHES,
	CONDITIONAL_BRANCHES,
	LOGICAL_OPERATIONS,
	ROTATE_AND_SHIFT,
	FLAG_OPERATIONS,
	VECTOR_INSTRUCTIONS,
	CONDITIONAL_MOVES,
	MMX_AND_SSE_INSTRUCTIONS,
	SYSTEM_CALLS,
	FLOATING_POINT,
	THE_REST
};

string InsTypeLiterals[] = {
	"Loads",
	"Stores",
	"NOPs",
	"Direct calls",
	"Indirect calls",
	"Returns",
	"Unconditional branches",
	"Conditional branches",
	"Logical operations",
	"Rotate and Shift",
	"Flag operations",
	"Vector instructions",
	"Conditional moves",
	"MMX and SSE instructions",
	"System calls",
	"Floating point instructions",
	"The rest"
};

enum FootprintType{
    INSTRUCTION_FOOTPRINT,
    DATA_FOOTPRINT
};

string FootprintTypeLiterals[] = {
    "Instruction Blocks Accesses",
    "Data Blocks Accesses"
};

/*
 * Counters used across the tool
 */

// The running count of instructions is kept here
// make it static to help the compiler optimize docount
static UINT64 icount = 0;
static UINT64 total_count = 0;
static UINT64 fast_forward_count = 0;

/*
 * Part A, B
 * Count Variable for seventeen types of instructions
 */

static UINT64 ins_type_count[NUM_INS_TYPE] = {};

/*
 * Part C
 * footprint flags to count number of memory blocks used
 */

static bool footprint[NUM_FOOTPRINT_TYPE][NUM_MEMORY_BLOCKS] = {};

/*
 * Part D
 * Counters for various statistics, +1 to account for 0 based indexing
 */

static UINT64 ins_len_count[MAX_INS_SIZE+1] = {};
static UINT64 num_operand_count[MAX_NUM_OPERANDS+1] = {};
static UINT64 num_reg_read_op_count[MAX_NUM_OPERANDS+1] = {};
static UINT64 num_reg_write_op_count[MAX_NUM_OPERANDS+1] = {};
static UINT64 num_mem_op_count[MAX_NUM_MEMOPS+1] = {};
static UINT64 num_mem_read_op_count[MAX_NUM_MEMOPS+1] = {};
static UINT64 num_mem_write_op_count[MAX_NUM_MEMOPS+1] = {};
