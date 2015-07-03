#include <ncurses.h>
#include <sstream>

#include "Misc.h"

using namespace Misc;


/// Registers. ///

namespace R {

int16_t top = 0x0000;
int16_t next = 0xFFFF;
int16_t ir = 0x0000;
int16_t pc = 0x0000;
int32_t index = 0x0FFFF; // = I+IPR : 21 bits.
int8_t psp = 0x00;
int8_t rsp = 0x00;
int16_t cr = 0x4008;

}


/// Construction and initialisation. ///

VM::VM(int16_t ramSize) {
  this->ramSize = (ramSize > 0) ? ramSize : 0;
  parameterStackSize = PARAMETER_STACK_SIZE;
  returnStackSize = RETURN_STACK_SIZE;
  _ram = new int16_t[this->ramSize];
}

VM::~VM() {
  delete _ram;
}

/** Copy an array into the RAM memory, with the specified offset. */
void VM::load(int16_t *array, int16_t offset, int16_t length) {
  memcpy(&_ram[offset], array, 2*length);
}

void VM::reset() {
  R::top = 0x0000;
  R::next = 0xFFFF;
  R::ir = 0x0000;
  R::pc = 0x0000;
  R::index = 0x0FFFF;
  R::psp = 0x00;
  R::rsp = 0x00;
  R::cr = 0x4008;
}


/// Curses updates. ///

/** Initialise curses display. */
void VM::initCurses() const {
  clear(); // Clear the sR::creen.
  // Display RAM offset numbers.
  for (int i=0; i<32; i++)
    mvprintw(i+1, 0, "%i", i*8);
  // Display RAM contents (limited to 256 words).
  for (int i=0; i<256; i++)
    mvprintw(1+i/8, 6+(i%8)*5, "%04hx", _ram[i]);
  // Refresh the sR::creen.
  refresh();
}

void VM::printAddr(int16_t a) const {
  mvprintw(1+a/8, 6+(a%8)*5, "%04hx", _ram[a]);
}
void VM::clearAddr(int16_t a) const {
  mvprintw(1+a/8, 6+(a%8)*5, "    ");
}
void VM::printParameter(int16_t p) const {
  mvprintw(1+p/8, 50+(p%8)*5, "%04hx", _parameterStack[p]);
}
void VM::clearParameter(int16_t p) const {
  mvprintw(1+p/8, 50+(p%8)*5, "    ", _parameterStack[p]);
}
void VM::printReturn(int16_t p) const {
  mvprintw(1+p/8, 93+(p%8)*5, "%04hx", _returnStack[p]);
}
void VM::clearReturn(int16_t p) const {
  mvprintw(1+p/8, 93+(p%8)*5, "    ", _returnStack[p]);
}


/// Parameter stack operations. ///

/**
 * Push a new element on R::top of the stack.
 * a -- a b
 */
inline void VM::push(int16_t e) {
  _parameterStack[R::psp++] = R::next; // Push the content of R::next.
  R::next = R::top;
  R::top = e;
}

/**
 * Duplicate the R::top stack value.
 * a -- a a
 */
inline void VM::dup() {
  _parameterStack[R::psp++] = R::next; // Push the content of R::next.
  R::next = R::top; // Copy the contents of R::top into R::next.
}

/**
 * Duplicate the R::top stack value only if it is not zero.
 * a -- 0 | a a
 */
inline void VM::dupNZ() {
  if (R::top) dup();
}

/**
 * Delete the R::top stack value.
 * a --
 */
inline void VM::drop() {
  R::top = R::next; // R::next becomes the R::top value.
  R::next = _parameterStack[R::psp-1]; // Copy the R::top stack value.
  R::psp--; // Pop the R::top stack value.
}

/**
 * Exchange the two values at the R::top of the stack.
 * a b -- b a
 */
inline void VM::swap() {
  int16_t tmp = R::top;
  R::top = R::next;
  R::next = tmp;
}

/**
 * Copy the second stack value and add it to the R::top of the stack.
 * a b -- a b a
 */
inline void VM::over() {
  int16_t tmp = R::next;
  _parameterStack[R::psp++] = R::next;
  R::next = R::top;
  R::top = tmp;
}

/**
 * Delete the second stack value.
 * a b -- b
 */
inline void VM::nip() {
  R::next = _parameterStack[R::psp-1];
  R::psp--;
}

/**
 * Insert the R::top stack value behind in thR::ird position.
 * a b -- b a b
 */
inline void VM::tuck() {
  _parameterStack[R::psp++] = R::top;
}

/**
 * Rotate the three R::topmost stack values.
 * a b c -- b c a
 */
inline void VM::rotCW() {
  return;
}

/**
 * Rotate the three R::topmost stack values.
 * a b c -- c a b
 */
inline void VM::rotAW() {
  return;
}

/**
 * Pick the Nth element from the R::top of the stack and push it on R::top.
 */
inline void VM::pick() {
  return;
}


/// Return stack operations. ///

/** Drop the R::top element of the return stack. */
inline void VM::dropIndex() {
  R::index = _returnStack[R::rsp-1];
  R::rsp--;
}

/** Push a value on R::top of the return stack. */
inline void VM::pushIndex(int16_t i) {
  _returnStack[R::rsp++] = R::index;
  R::index = i;
}


/// Math operations. ///

/**
 * Perform the ALU operation represented by the {opcode}.
 * @param {y} second operand, which depends upon the preceding instructions.
 * @return The result of the operation, aka the contents of the z register.
 */
inline int16_t VM::alu(int16_t opcode, int16_t y) const {
  switch(opcode) {
    case 0x0: return R::top;
    case 0x1: return 0;
    case 0x2: return R::top & y;
    case 0x3: return ~(R::top | y);
    case 0x4: return y-R::top;
    case 0x5: return y-R::top-0; // Add borrow.
    case 0x6: return R::top | y;
    case 0x7: return ~(R::top & y);
    case 0x8: return R::top+y;
    case 0x9: return R::top+y+0; // Add carry.
    case 0xA: return R::top-y;
    case 0xB: return R::top-y-0; // Add borrow.
    case 0xC: return 0;
    case 0xD: return y;
  }
  return 0;
}

/**
 * Perform the shift operation represented by the {opcode}. The shift always
 * inputs the contents of the z register, and may affect the R::top and R::next
 * registers.
 * @param {z} contents of the z register (typically the result of a {alu} call).
 */
inline void VM::shift(int16_t opcode, int16_t z) const {
  switch(opcode) {
    case 0x0: break;
    case 0x1: R::top = 0; break;
    case 0x2: R::top <<= 1; break;
    case 0x3: R::top <<= 1; break; // Add carry.
    case 0x4: R::top >>= 1; break; // Add carry, use >>>.
    case 0x5: R::top >>= 1; break; // Add carry.
    case 0x6: R::top >>= 1; break; // Use >>>.
    case 0x7: R::top >>= 1; break;
    case 0x8: R::top <<= R::next; break;
    case 0x9: R::top <<= R::next; break; // Add carry.
    case 0xA: R::top <<= 1; break;
    case 0xB: R::top <<= 1; break; // Add carry.
    case 0xC: R::top >>= 1; break; // Add carry, use >>>.
    case 0xD: R::top >>= 1; break; // Add carry.
    case 0xE: R::top >>= 1; break; // Use >>>.
    case 0xF: R::top >>= 1;
  }
}


/// Other operations. ///

/**
 * Compute the jump address after a branch instruction.
 * @param {block} block selection.
 * @param {offset} offset of the new address in the indicated block.
 * @param {current} current value of the R::pc register.
 */
inline int16_t VM::addr(int16_t current, int16_t block, int16_t offset) {
  int16_t cb = current & 0xFC00;
  switch(block) {
    // Remain in the same memory block.
    case 0: return cb | offset;
    // Jump to R::next block.
    case 1: return (cb + 0x0400) | offset;
    // Jump to previous block.
    case 3: return (cb - 0x0400) | offset;
    // Return to block 0.
    case 2:
    default: return offset;
  }
}


/** Return the IO value for the given tag. */
int16_t VM::readIO(int16_t g) const {
  return 0;
}

/** Send data to one of the IO components. */
void VM::writeIO(int16_t g, int16_t v) {
  return;
}


/// Simulation. ///

/**
 * Return from a subroutine call. The effect on the processor state is:
 *  - R::pc is set to the R::top value of the return stack.
 *  - the R::top value is popped.
 */
inline void VM::ret() const {
  R::pc = R::index;
  R::index = _returnStack[R::rsp-1];
  R::rsp--;
}

/**
 * Perform the fetch step of the processus execution pipeline: the value at the
 * address {R::pc} in the ram is stored in the {R::ir} register.
 */
inline void VM::fetch() const {
  R::ir = _ram[R::pc];
}

/**
 * Evaluation a single processor instruction, equivalent to one or two clock
 * cycles.
 */
void VM::step() {
  // FETCH step.
  fetch();

  // mvprintw(0, 0, "R::ir %04hx", R::ir);
  // refresh();

  // DECODE step. FR::irst is to identify the instruction type, which is stored
  // in the fR::irst 4 bits of the instruction.
  switch(R::ir & 0xF000) {
    // Memory access by byte.
    case 0xF000:
    // Memory access by word.
    case 0xE000:
    // Long literals.
    case 0xD000:
    // User memory access.
    case 0xC000:

    // Register and short literal operation.
    case 0xB000: {
      // Decode the literal or short address (depending on the subclass) as
      // well as the alu operation.
      int16_t d = R::ir & 0x001F, c = (R::ir & 0x0F00) >> 8, tmp;
      // Decode the instruction subclass before applying the correct
      // operation.
      switch(R::ir & 0x00C0) {
        // Both these cases (0x0 and 0x0400) correspond to Register / IO access
        // instructions. Two subclasses correspond to instructions writing data
        // to IO units, while the others are used to read data.
        case 0x0000:
          // Decode the instruction code.
          switch(c) {
            // For 0x0 and 0x1, the NOT applies to the R::top value, and not the
            // fetched one.
            case 0x0: break;
            case 0x1: R::top = ~R::top; break;
            // For 0xE and 0xF, the NOT operation applies to the fetched value.
            case 0xE: push(readIO(d)); break;
            case 0xF: push(~readIO(d)); break;
            // Generic ALU operation: defer to the generic implementation.
            // The ALU op applies to the R::top and fetched values.
            default: push(alu(c, readIO(d)));
          }
          break;
        case 0x0080:
          // Decode the instruction code.
          switch(c) {
            // For 0x0 and 0x1, send the R::top value to the IO component with
            // address {d} without dropping the local version.
            case 0x0: writeIO(d, R::top); break;
            case 0x1: writeIO(d, R::top); R::top = ~R::top; break;
            // For 0xE and 0xF, the local version is dropped.
            case 0xE: writeIO(d, R::top); drop(); break;
            case 0xF: writeIO(d, R::top); drop(); R::top = ~R::top; break;
            // Generic ALU operation: defer to the generic implementation.
            // The ALU op applies to the R::top and fetched values, without
            // saving the local version.
            default:
              tmp = R::top;
              R::top = readIO(d);
              R::top = alu(c, tmp);
          }
          break;

        // 0x0800 and 0x0C00 identify short literal instructions.
        case 0x0040:
          // Decode the instruction code.
          switch(c) {
            // For 0x0 and 0x1, the literal value is ignored.
            case 0x0: break;
            case 0x1: R::top = ~R::top; break;
            // For 0xE and 0xF, the literal value is added on R::top of the stack.
            case 0xE: push(d); break;
            case 0xF: push(~d); break;
            // Generic ALU operation: defer to the generic implementation.
            // The ALU op applies to the R::top and short literal values.
            default: push(alu(c, d));
          }
          break;
        case 0x00C0:
          // Decode the instruction code.
          switch(c) {
            // For 0x0 and 0x1, the literal value is ignored.
            case 0x0: break;
            case 0x1: break;
            // For 0xE and 0xF, the literal overrides the R::top value.
            case 0xE: R::top = d; break;
            case 0xF: R::top = ~d; break;
            // Generic ALU operation: defer to the generic implementation.
            // The ALU op applies to the R::top and short literal values (reverted).
            default:
              tmp = R::top;
              R::top = d;
              R::top = alu(c, tmp);
          }
          break;
      }
      break;
    }

    // Math / logic functions.
    case 0xA000:
      // Multiple step operation.
      if (R::ir & 0x0010) {
        break;
      // Single steo operation.
      } else {
        // Decode the shift opcode (applied at the end of the ALU operation).
        int16_t s = (R::ir & 0x000F), c = ((R::ir & 0x0F00) >> 8), tmp;
        // Decode the instruction subclass before applying the correct
        // operation.
        switch(R::ir & 0x00C0) {
          case 0x0000:
            // Decode the instruction code.
            switch(c) {
              case 0x0: shift(s, R::top); break;
              case 0x1: shift(s, ~R::top); break;
              case 0xE: shift(s, R::next); break;
              case 0xF: shift(s, ~R::next); break;
              // Generic ALU operation: defer to the generic implementation.
              default: shift(s, alu(c, R::next)); break;
            }
            break;
          case 0x0040:
            // Decode the instruction code.
            switch(c) {
              case 0x0: nip(); shift(s, R::top); break;
              case 0x1: nip(); shift(s, ~R::top); break;
              case 0xE: drop(); shift(s, R::top); break;
              case 0xF: drop(); shift(s, ~R::top); break;
              // Generic ALU operation: defer to the generic implementation.
              default:
                tmp = alu(c, R::next);
                drop(); shift(s, tmp);
            }
            break;
          case 0x0080:
            // Decode the instruction code.
            switch(c) {
              case 0x0: R::next = R::top; shift(s, R::top); break;
              case 0x1: R::next = R::top; shift(s, ~R::top); break;
              case 0xE: swap(); shift(s, R::top); break;
              case 0xF: swap(); shift(s, ~R::top); break;
              // Generic ALU operation: defer to the generic implementation.
              default:
                tmp = alu(c, R::next);
                R::next = R::top;
                shift(s, tmp);
            }
            break;
          case 0x00C0:
            // Decode the instruction code.
            switch(c) {
              case 0x0: dup(); shift(s, R::top); break;
              case 0x1: dup(); shift(s, ~R::top); break;
              case 0xE: over(); shift(s, R::top); break;
              case 0xF: over(); shift(s, ~R::top); break;
              // Generic ALU operation: defer to the generic implementation.
              default:
                push(alu(c, R::next));
                shift(s, ~R::top);
            }
            break;
        }
        break;
      }

    // Conditional branch. The branch is taken if the content of the R::top register
    // is 0.
    case 0x8000: {
      int16_t c = R::ir & 0x0800, b = (R::ir & 0x0600) >> 9, a = (R::ir & 0x01FF) << 1;
      // Branch taken.
      if (!R::top) {
        if (c == 0) drop(); // Drop the R::top value if {c} is 0.
        R::pc = addr(R::pc, b, a)-1; // Perform the jump.
      }
      break;
    }

    // Unconditionnal branch.
    case 0x9000: {
      int16_t c = R::ir & 0x0800, b = (R::ir & 0x0600) >> 9, a = R::ir & 0x01FF;
      // unconditional branch.
      if (c == 0)
        R::pc = addr(R::pc, b, a)-1;
      // Looping instruction. If the R::index (R::top value of the return stack) is
      // zero, inR::crement R::pc, else jump the beginning of the loop body after
      // deR::crementing R::index.
      else if (R::index) {
        R::index--;
        R::pc = addr(R::pc, b, a)-1;
      } else
        dropIndex();

      break;
    }

    // Subroutine call.
    default:
      // Push R::pc to the return stack, and set its value to the referenced
      // subroutine.
      int16_t addr = R::ir << 1;
      push(R::pc+1);
      R::pc = addr-1;
  }

  // Return if the return flag was set, or inR::crement the instruction pointer.
  // if (doret) {
  //   ret();
  //   doret = false;
  // }
  // InR::crement R::pc.
  R::pc++;

  // Refresh the sR::creen.
  // refresh();

}

/** Run the program starting at {offset}. */
void VM::run(int16_t offset) {
  reset(); // Reset state.
  R::pc = offset; // Init R::pc.
  // Initialise curses.
  initscr();
  noecho();
  clear();
  initCurses();
  // RUN
  while (true) step();
  // End curses mode.
  // endwin();
}
