#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <iomanip>

namespace Misc {

/** MISC Instruction set. */
typedef enum {
  PSH,
  ADD,
  POP,
  SET,
  HLT
} Instruction;

/** MISC Register set. */
typedef enum {
  TOP,    // Contains the value on top of the parameter stack.
  NEXT,   // Contains the next parameter stack value.
  IR,     // Instruction register, contains the instruction being executed.
  PC,     // Program counter, points to the instruction to be executed next.
  CR,     // Configuration register, contains the state of the processor.
  I,      // Index, contains the top value of the return stack.
  MD,
  SR,
  REGISTER_COUNT
} Register;

/**
 * Build the core elements of a VM, including RAM memory and separate stack.
 * Registers are limited to the stack pointer and instruction pointer. The
 * program is loaded from the RAM (no distinct memory).
 */
class VM {

  public:
    VM(int16_t ramSize=4096, int16_t parameterStackSize=256, int16_t returnStackSize=256);
    ~VM();

    /** Copy an array into the RAM memory, with the specified offset. */
    void load(int16_t *array, int16_t offset, int16_t length);

    /**
     * Execute the program starting at {offset}. The execution stops when a HLT
     * instruction is encountered.
     */
    void execute(int16_t offset=0);

    /** Display the current state on to the standard output. */
    void dumpStack(int16_t offset=0, int16_t length=-1) const;

    int16_t ramSize;
    int16_t parameterStackSize;
    int16_t returnStackSize;

  private:
    /** Fetch the instruction pointed to by IP. */
    int16_t fetch() const;
    /** Return the IO value for the given tag. */
    int16_t readIO(int16_t g) const;
    void writeIO(int16_t g, int16_t n);
    /** Run the current instruction. */
    void run();

    /// Parameter stack operations. ///
    inline int16_t dup();
    inline void dupNZ();
    inline void drop();
    inline void swap();
    inline int16_t over();
    inline int16_t nip();
    inline void tuck();
    inline void rotCW();
    inline void rotAW();
    inline void pick();

    /// Math operations. ///
    inline int16_t alu(int16_t opcode, int16_t y);
    inline int16_t shift(int16_t opcode, int16_t t);

    /// Return stack operations. ///
    inline void ret();

    int16_t* _ram; // Random Access Memory.
    int16_t* _parameterStack;
    int16_t* _returnStack;

    int16_t _ir; // Instruction register.
    int16_t _pc; // Program counter.
    int16_t _cr; // Configuration register.
    int16_t _psp; // Parameter stack pointer.
    int16_t _rsp; // Return stack pointer.

};

};

