#include "../src/Misc.h"

using namespace Misc;

int main() {

  int16_t program[] = {
    PSH, 5,
    PSH, 6,
    ADD,
    POP,
    HLT
  };

  Misc::VM vm;
  vm.load(program, 0, 7);
  vm.execute(0);
  vm.dumpStack();

  return 0;
}
