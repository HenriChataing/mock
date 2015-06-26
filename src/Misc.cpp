#include "Misc.h"

using namespace Misc;

/// Construction and initialisation. ///

VM::VM(int16_t ramSize, int16_t parameterStackSize, int16_t returnStackSize):
    _psp(-1), _rsp(-1), _pc(0) {
  this->ramSize = (ramSize > 0) ? ramSize : 0;
  this->parameterStackSize = (parameterStackSize > 0) ? parameterStackSize : 0;
  this->returnStackSize = (returnStackSize > 0) ? returnStackSize : 0;
  _ram = new int16_t[this->ramSize];
  _parameterStack = new int16_t[this->parameterStackSize];
  _returnStack = new int16_t[this->returnStackSize];
}

VM::~VM() {
  delete _ram;
  delete _parameterStack;
  delete _returnStack;
}

/** Copy an array into the RAM memory, with the specified offset. */
void VM::load(int16_t *array, int16_t offset, int16_t length) {
  memcpy(&_ram[offset], array, 16*length);
}

/** Display the current state on to the standard output. */
void VM::dumpStack(int16_t offset, int16_t length) const {
  int16_t i,j;
  for (i=0; i<ramSize; i+=8) {
    std::cout << std::dec << std::setfill(' ') << std::left << std::setw(8) << i << " ";
    std::cout << std::hex << std::setfill('0') << std::right;
    for (j=i; j<ramSize && j<i+8; j++)
      std::cout << std::setw(4) << _ram[j] << " ";
    std::cout << std::endl;
  }
}


/// Parameter stack operations. ///

/**
 * Duplicate the top stack value.
 * a -- a a
 * @return the top value ({a}).
 */
inline int16_t VM::dup() {
  int16_t top = _parameterStack[_psp];
  _parameterStack[++_psp] = top;
  return top;
}

/**
 * Duplicate the top stack value only if it is not zero.
 * a -- 0 | a a
 */
inline void VM::dupNZ() {
  int16_t top = _parameterStack[_psp];
  if (top) _parameterStack[++_psp] = top;
}

/**
 * Delete the top stack value.
 * a --
 */
inline void VM::drop() {
  _psp--;
}

/**
 * Exchange the two values at the top of the stack.
 * a b -- b a
 */
inline void VM::swap() {
  int16_t top = _parameterStack[_psp];
  _parameterStack[_psp] = _parameterStack[_psp-1];
  _parameterStack[_psp-1] = top;
}

/**
 * Copy the second stack value and add it to the top of the stack.
 * a b -- a b a
 * @return the duplicated value ({b}).
 */
inline int16_t VM::over() {
  int16_t next = _parameterStack[_psp-1];
  _parameterStack[++_psp] = next;
  return next;
}

/**
 * Delete the second stack value.
 * a b -- b
 * @return the top value ({b}).
 */
inline int16_t VM::nip() {
  int16_t top = _parameterStack[_psp];
  _parameterStack[_psp-1] = top;
  _psp--;
  return top;
}

/**
 * Insert the top stack value behind in third position.
 * a b -- b a b
 */
inline void VM::tuck() {
  _psp++;
  _parameterStack[_psp] = _parameterStack[_psp-1];
  _parameterStack[_psp-1] = _parameterStack[_psp-2];
  _parameterStack[_psp-2] = _parameterStack[_psp];
}

/**
 * Rotate the three topmost stack values.
 * a b c -- b c a
 */
inline void VM::rotCW() {
  return;
}

/**
 * Rotate the three topmost stack values.
 * a b c -- c a b
 */
inline void VM::rotAW() {
  return;
}

/**
 * Pick the Nth element from the top of the stack and push it on top.
 */
inline void VM::pick() {
  return;
}


/// Math operations. ///

/**
 * Perform the ALU operation represented by the {opcode}.
 * @param {y} second operand, which depends upon the preceding instructions.
 * @return the result of the operation.
 */
inline int16_t VM::alu(int16_t opcode, int16_t y) {
  switch(opcode) {
    case 0x0: return _parameterStack[_psp];
    case 0x1: return 0;
    case 0x2: return _parameterStack[_psp] & y;
    case 0x3: return ~(_parameterStack[_psp] | y);
    case 0x4: return y-_parameterStack[_psp];
    case 0x5: return y-_parameterStack[_psp] - 0; // Add borrow.
    case 0x6: return _parameterStack[_psp] | y;
    case 0x7: return ~(_parameterStack[_psp] | y);
    case 0x8: return _parameterStack[_psp] + y;
    case 0x9: return _parameterStack[_psp] + y + 0; // Add carry.
    case 0xA: return _parameterStack[_psp] - y;
    case 0xB: return _parameterStack[_psp] - y - 0; // Add borrow.
    case 0xC: return 0;
    case 0xD: return y;
  }
  return 0;
}

/**
 * Perform the shift operation represented by the {opcode}.
 * @param {t} operand of the operation (supposedly the content of the top register).
 * @return the shifted value.
 */
inline int16_t VM::shift(int16_t opcode, int16_t t) {
  switch(opcode) {
    case 0x0: return t;
    case 0x1: return 0;
    case 0x2: return t << 1;
    case 0x3: return t << 1; // Add carry.
    case 0x4: return t >> 1; // Add carry, use >>>.
    case 0x5: return t >> 1; // Add carry.
    case 0x6: return t >> 1; // Use >>>.
    case 0x7: return t >> 1;
    case 0x8: return t << _parameterStack[_psp-1];
    case 0x9: return t << _parameterStack[_psp-1]; // Add carry.
    case 0xA: return t << 1;
    case 0xB: return t << 1; // Add carry.
    case 0xC: return t >> 1; // Add carry, use >>>.
    case 0xD: return t >> 1; // Add carry.
    case 0xE: return t >> 1; // Use >>>.
    case 0xF: return t >> 1;
  }
  return 0;
}

/// Return stack operations. ///

/**
 * Return from a subroutine call. The effect on the processor state is:
 *  - PC is set to the top value of the return stack.
 *  - the top value is popped.
 */
inline void VM::ret() {
  _pc = _returnStack[_rsp--];
}

/// Simulation. ///

/** Fetch the instruction at the address {_pc}. */
int16_t VM::fetch() const {
  return _ram[_pc];
}

/** Return the IO value for the given tag. */
int16_t VM::readIO(int16_t g) const {
  return 0;
}

/** Send data to one of the IO components. */
void VM::writeIO(int16_t g, int16_t v) {
  return;
}

/**
 * Run the program starting at {offset}. The execution stops when a HLT
 * instruction is encountered.
 */
void VM::execute(int16_t offset) {
  _pc = offset;
  run();
}

/** Run the current instruction. */
void VM::run() {
  bool goon = true, doret = false;
  // Main loop, running until some instruction causes the execution to stop.
  while (goon) {
    // FETCH step.
    int16_t ir = fetch();
    // DECODE step. First is to identify the instruction type, which is stored
    // in the first 4 bits of the instruction.
    switch(ir & 0xF000) {
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
        int16_t d = ir & 0x001F, c = (ir & 0x0F00) >> 8, r, top, next;
        // Decode the instruction subclass before applying the correct
        // operation.
        switch(ir & 0x00C0) {
          // Both these cases (0x0 and 0x0400) correspond to IO entries. Two
          // subclasses correspond to instructions writing data to IO units,
          // while the others are used to read data.
          case 0x0000:
            // Decode the instruction code.
            switch(c) {
              // For 0x0 and 0x1, the NOT applies to the top value, and not the
              // fetched one.
              case 0x0: break;
              case 0x1: _parameterStack[_psp] = ~_parameterStack[_psp]; break;
              // For 0xE and 0xF, the NOT operation applies to the fetched value.
              case 0xE: _parameterStack[++_psp] = readIO(d); break;
              case 0xF: _parameterStack[++_psp] = ~readIO(d); break;
              // Generic ALU operation: defer to the generic implementation.
              // The ALU op applies to the top and fetched values.
              default: _parameterStack[++_psp] = alu(c, readIO(d));
            }
            break;
          case 0x0040:
            // Decode the instruction code.
            switch(c) {
              // For 0x0 and 0x1, send the top value to the IO component with
              // address {d} without dropping the local version.
              case 0x0: writeIO(d, _parameterStack[_psp]); break;
              case 0x1:
                writeIO(d, _parameterStack[_psp]);
                _parameterStack[_psp] = ~_parameterStack[_psp];
                break;
              // For 0xE and 0xF, the local version is dropped.
              case 0xE:
                writeIO(d, _parameterStack[_psp]);
                _psp--; break;
              case 0xF:
                writeIO(d, _parameterStack[_psp]);
                _psp--; _parameterStack[_psp] = ~_parameterStack[_psp];
                break;
              // Generic ALU operation: defer to the generic implementation.
              // The ALU op applies to the top and fetched values, without
              // saving the local version.
              default:
                _parameterStack[++_psp] = readIO(d);
                r = alu(c, _parameterStack[_psp-1]);
                _parameterStack[--_psp] = r;
            }
            break;

          // 0x0800 and 0x0C00 identify short literal instructions.
          case 0x0080:
            // Decode the instruction code.
            switch(c) {
              // For 0x0 and 0x1, the literal value is ignored.
              case 0x0: break;
              case 0x1: _parameterStack[_psp] = ~_parameterStack[_psp]; break;
              // For 0xE and 0xF, the literal value is added on top of the stack.
              case 0xE: _parameterStack[++_psp] = d; break;
              case 0xF: _parameterStack[++_psp] = ~d; break;
              // Generic ALU operation: defer to the generic implementation.
              // The ALU op applies to the top and short literal values.
              default: _parameterStack[++_psp] = alu(c, d);
            }
            break;
          case 0x00C0:
            // Decode the instruction code.
            switch(c) {
              // For 0x0 and 0x1, the literal value is ignored.
              case 0x0: break;
              case 0x1: _parameterStack[_psp] = ~_parameterStack[_psp]; break;
              // For 0xE and 0xF, the literal overrides the top value.
              case 0xE: _parameterStack[_psp] = d; break;
              case 0xF: _parameterStack[_psp] = ~d; break;
              // Generic ALU operation: defer to the generic implementation.
              // The ALU op applies to the top and short literal values (reverted).
              default:
                _parameterStack[++_psp] = d;
                r = alu(c, _parameterStack[_psp-1]);
                _parameterStack[--_psp] = r;
            }
            break;
        }
        break;
      }

      // Math / logic functions.
      case 0xA000:
        // Multiple step operation.
        if (ir & 0x0010) {
          break;
        // Single steo operation.
        } else {
          // Decode the shift opcode (applied at the end of the ALU operation).
          int16_t s = (ir & 0x000F), c = ((ir & 0x0F00) >> 8), r, top, next;
          // Decode the instruction subclass before applying the correct
          // operation.
          switch(ir & 0x00C0) {
            case 0x0000:
              // Decode the instruction code.
              switch(c) {
                case 0x0: _parameterStack[_psp] = shift(s, _parameterStack[_psp]); break;
                case 0x1: _parameterStack[_psp] = shift(s, ~_parameterStack[_psp]); break;
                case 0xE: _parameterStack[_psp] = shift(s, _parameterStack[_psp-1]); break;
                case 0xF: _parameterStack[_psp] = shift(s, ~_parameterStack[_psp-1]); break;
                // Generic ALU operation: defer to the generic implementation.
                default: _parameterStack[_psp] = shift(s, alu(c, _parameterStack[_psp-1]));
              }
              break;
            case 0x0040:
              // Decode the instruction code.
              switch(c) {
                case 0x0:
                  r = shift(s, _parameterStack[_psp]);
                  _parameterStack[--_psp] = r;
                  break;
                case 0x1:
                  r = shift(s, ~_parameterStack[_psp]);
                  _parameterStack[--_psp] = r;
                  break;
                case 0xE:
                  r = shift(s, _parameterStack[--_psp]);
                  _parameterStack[_psp] = r;
                  break;
                case 0xF:
                  r = shift(s, ~_parameterStack[--_psp]);
                  _parameterStack[_psp] = r;
                  break;
                // Generic ALU operation: defer to the generic implementation.
                default:
                  int16_t r = shift(s, alu(c, _parameterStack[_psp-1]));
                  _parameterStack[--_psp] = r;
              }
              break;
            case 0x0080:
              // Decode the instruction code.
              switch(c) {
                case 0x0:
                  top = _parameterStack[_psp];
                  _parameterStack[_psp-1] = top;
                  _parameterStack[_psp] = shift(s, top);
                  break;
                case 0x1:
                  top = _parameterStack[_psp];
                  _parameterStack[_psp-1] = top;
                  _parameterStack[_psp] = shift(s, ~top);
                  break;
                case 0xE:
                  top = _parameterStack[_psp], next = _parameterStack[_psp-1];
                  _parameterStack[_psp-1] = top;
                  _parameterStack[_psp] = shift(s, next);
                  break;
                case 0xF:
                  top = _parameterStack[_psp], next = _parameterStack[_psp-1];
                  _parameterStack[_psp-1] = top;
                  _parameterStack[_psp] = shift(s, ~next);
                  break;
                // Generic ALU operation: defer to the generic implementation.
                default:
                  top = _parameterStack[_psp];
                  _parameterStack[_psp] = shift(s, alu(c, _parameterStack[_psp-1]));
                  _parameterStack[_psp-1] = top;
              }
              break;
            case 0x00C0:
              // Decode the instruction code.
              switch(c) {
                case 0x0:
                  r = shift(s, _parameterStack[_psp]);
                  _parameterStack[++_psp] = r;
                  break;
                case 0x1:
                  r = shift(s, ~_parameterStack[_psp]);
                  _parameterStack[++_psp] = r;
                  break;
                case 0xE:
                  r = shift(s, _parameterStack[_psp-1]);
                  _parameterStack[++_psp] = r;
                  break;
                case 0xF:
                  r = shift(s, ~_parameterStack[_psp-1]);
                  _parameterStack[++_psp] = r;
                  break;
                // Generic ALU operation: defer to the generic implementation.
                default:
                  r = shift(s, alu(c, _parameterStack[_psp-1]));
                  _parameterStack[++_psp] = r;
              }
              break;
          }
          break;
        }

      // Branches and loops.
      case 0x8000:
      case 0x9000:

      // Subroutine call.
      default:
        // Push PC to the return stack, and set its value to the referenced
        // subroutine.
        int16_t addr = ir << 1;
        _returnStack[++_rsp] = _pc+1;
        _pc = addr;
        break;
    }

    // Return if the return flag was set, or increment the instruction pointer.
    if (doret) {
      ret();
      doret = false;
    } else
      _pc++;
  }
}
