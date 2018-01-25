/*
 * Counters used across the tool
 */

// The running count of instructions is kept here
// make it static to help the compiler optimize docount
static UINT64 icount = 0;
static UINT64 total_count = 0;
static UINT64 fast_forward_count = 0;

/*
 * Part A
 * Count Variable for seventeen types of instructions
 */

const int NUM_INS_TYPE = 17;
const int NUM_FOOTPRINT_TYPE = 2;
const int NUM_MEMORY_BLOCKS = (1<<29);

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

static UINT64 ins_type_count[NUM_INS_TYPE] = {};
static bool footprint[NUM_FOOTPRINT_TYPE][NUM_MEMORY_BLOCKS] = {};
