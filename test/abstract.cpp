#include "../src/Misc.h"

using namespace Misc;

int main() {

  int16_t program[] = {
    PUSH(1),
    PUSH(1),
    MATH(ADD, 3, 0, 0),
    MATH(ADD, 3, 0, 0),
    MATH(ADD, 3, 0, 0),
    MATH(ADD, 3, 0, 0),
    MATH(ADD, 3, 0, 0),
    MATH(ADD, 3, 0, 0)
  };

  Misc::VM vm;
  vm.load(program, 0, 8);
  vm.run(0);

  return 0;
}
