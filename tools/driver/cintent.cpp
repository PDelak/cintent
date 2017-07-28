#include "cling/Interpreter/Interpreter.h"

int main(int argc, char** argv) {
  const char* LLVMRESDIR = "/usr/local/"; //path to llvm resource directory
  cling::Interpreter interp(argc, argv, LLVMRESDIR);

  interp.declare("int p=0;");
}
        
