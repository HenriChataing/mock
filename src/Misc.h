#ifndef MISC_INCLUDED
#define MISC_INCLUDED

#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <iomanip>

/** Harris RTX 200 instruction set. */

#define AND  0x2
#define NOR  0x3
#define SUB  0x4
#define OR   0x6
#define NAND 0x7
#define ADD  0x8
#define XOR  0xA
#define XNOR 0xB
#define RSUB 0xC

#define SHL  0x2
#define SHR  0x4

#define CALL(a) (a << 1)
#define MATH(c, b, r, s) (0xA000 | (c << 8) | (b << 6) | (r << 5) | s)
#define PUSH(d) (0xBE40 | d)

#define PARAMETER_STACK_SIZE 256
#define RETURN_STACK_SIZE 256

namespace Misc {

/**
 * Build the core elements of a VM, including RAM memory and separate stack.
 * Registers are limited to the stack pointer and instruction pointer. The
 * program is loaded from the RAM (no distinct memory).
 */
class VM {

  public:
    VM(int16_t ramSize=4096);
    ~VM();

    /** Copy an array into the RAM memory, with the specified offset. */
    void load(int16_t *array, int16_t offset, int16_t length);

    /** Reset the state of the processor. */
    void reset();

    /** Execute the program starting at {offset}. */
    void run(int16_t offset=0);

    int16_t ramSize;
    int16_t parameterStackSize;
    int16_t returnStackSize;

  private:

    /// Curses updates. ///
    void initCurses() const;
    void printAddr(int16_t a) const;
    void clearAddr(int16_t a) const;
    void printParameter(int16_t p) const;
    void clearParameter(int16_t p) const;
    void printReturn(int16_t p) const;
    void clearReturn(int16_t p) const;

    /// Parameter stack operations. ///
    inline void push(int16_t e);
    inline void dup();
    inline void dupNZ();
    inline void drop();
    inline void swap();
    inline void over();
    inline void nip();
    inline void tuck();
    inline void rotCW();
    inline void rotAW();
    inline void pick();

    /// Return stack operations. ///
    inline void dropIndex();
    inline void pushIndex(int16_t i);

    /// Math operations. ///
    inline int16_t alu(int16_t opcode, int16_t y) const;
    inline void shift(int16_t opcode, int16_t z) const;

    /// Other operations. ///
    inline int16_t addr( int16_t current, int16_t block, int16_t offset);
    int16_t readIO(int16_t g) const;
    void writeIO(int16_t g, int16_t n);

    /// Simulation. ///
    inline void ret() const;
    inline void fetch() const;
    inline void step();

    /// Physical elements. ///
    int16_t* _ram;
    int16_t _parameterStack[PARAMETER_STACK_SIZE];
    int16_t _returnStack[RETURN_STACK_SIZE];

};

};

#endif // MISC_INCLUDED
