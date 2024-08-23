//
// Created by jwc on 7/26/24.
//

#include <cstring>

#include "JWCEssentials/JWCEssentials.h"

#include <iostream>

using namespace JWCEssentials;

int main(int argc, char **argv) {
  int Length = 10;

  if (argc == 2)
  {
      int I = 0;
      char *a = argv[1];
      while (*a != 0) {
          I *= 10;
          I += (*(a++)) - '0';
      }

      Length = I;
  }

    Random_MT19937 *rand = new Random_MT19937();
    rand->SetSeed(clock());

    std::cout << rand->cstyle_identifier(Length) << std::endl;
    delete rand;

    return 0;
}
