#include "llvm/Support/Signals.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/ManagedStatic.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "cling/Interpreter/Interpreter.h"

int main(int argc, char** argv) {
  llvm::llvm_shutdown_obj shutdownTrigger;

  llvm::sys::PrintStackTraceOnErrorSignal(argv[0]);
  llvm::PrettyStackTraceProgram X(argc, argv);

  cling::Interpreter interp(argc, argv);

  const cling::InvocationOptions& Opts = interp.getOptions();

  for (const std::string& Lib : Opts.LibsToLoad)
    interp.loadFile(Lib);

  interp.AddIncludePath(".");
  interp.process("#include <cstdio>");
  interp.process("printf(\"cintent a tool for metaprogramming\")");

  return 0;
}
