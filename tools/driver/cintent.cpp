#include "cling/Interpreter/Interpreter.h"
#include "cling/MetaProcessor/MetaProcessor.h"
#include "cling/UserInterface/UserInterface.h"

#include "clang/Basic/LangOptions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/FrontendTool/Utils.h"

#include "llvm/Support/Signals.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/ManagedStatic.h"

#include <iostream>
#include <fstream>
#include <vector> 
#include <string>


// If we are running with -verify a reported has to be returned as unsuccess.
// This is relevant especially for the test suite.

static int checkDiagErrors(clang::CompilerInstance* CI, unsigned* OutErrs = 0) {

  unsigned Errs = CI->getDiagnostics().getClient()->getNumErrors();

  if (CI->getDiagnosticOpts().VerifyDiagnostics) {
    // If there was an error that came from the verifier we must return 1 as
    // an exit code for the process. This will make the test fail as expected.
    clang::DiagnosticConsumer* Client = CI->getDiagnostics().getClient();
    Client->EndSourceFile();
    Errs = Client->getNumErrors();

    // The interpreter expects BeginSourceFile/EndSourceFiles to be balanced.
    Client->BeginSourceFile(CI->getLangOpts(), &CI->getPreprocessor());
  }

  if (OutErrs)
    *OutErrs = Errs;

  return Errs ? EXIT_FAILURE : EXIT_SUCCESS;
}

int validateInterpreter(const cling::Interpreter& Interp)
{

  const cling::InvocationOptions& Opts = Interp.getOptions();
  if (!Interp.isValid()) {
    if (Opts.Help || Opts.ShowVersion)
      return EXIT_SUCCESS;

    unsigned ErrsReported = 0;
    if (clang::CompilerInstance* CI = Interp.getCIOrNull()) {
      // If output requested and execution succeeded let the DiagnosticsEngine
      // determine the result code
      if (Opts.CompilerOpts.HasOutput && ExecuteCompilerInvocation(CI))
        return checkDiagErrors(CI);

      checkDiagErrors(CI, &ErrsReported);
    }

    // If no errors have been reported, try perror
    if (ErrsReported == 0)
      ::perror("Could not create Interpreter instance");

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int main( int argc, char **argv ) {

  if(argc < 3) {
    std::cout << "usage : cintent [plugin] [filename]" << std::endl;
    return -1;
  }

  llvm::llvm_shutdown_obj shutdownTrigger;

  llvm::sys::PrintStackTraceOnErrorSignal(argv[0]);
  llvm::PrettyStackTraceProgram X(argc, argv);

  // Set up the interpreter
  cling::Interpreter Interp(argc, argv);
  const cling::InvocationOptions& Opts = Interp.getOptions();

  if(validateInterpreter(Interp) != EXIT_SUCCESS) return EXIT_FAILURE;

  Interp.AddIncludePath(".");
  Interp.AddIncludePath("/home/delak/cling/src/build/tools/clang/include");

  for (const std::string& Lib : Opts.LibsToLoad)
    Interp.loadFile(Lib);

  cling::UserInterface Ui(Interp);
  // If we are not interactive we're supposed to parse files

  std::string runtimeLib = "cintentrt.cpp";

  std::string Plugin = Opts.Inputs[0];
  std::string Input = Opts.Inputs[1];

  std::string Cmd;
  cling::Interpreter::CompilationResult Result;

  Cmd = ".x ";
  Cmd += runtimeLib;
  Ui.getMetaProcessor()->process(Cmd, Result, 0);

  Cmd = ".x ";
  Cmd += Plugin;
  Ui.getMetaProcessor()->process(Cmd, Result, 0);

  std::string rewriterCmd = "Rewriter TheRewriter;";
  Interp.process(rewriterCmd);

  std::string injectorCmd = "ASTConsumer* TheConsumer = createASTConsumer(TheRewriter,";
  injectorCmd.append("\"").append(Input).append("\"").append(");");

  Interp.process(injectorCmd);

  std::string invCmd = "exec(";
  invCmd.append("\"").append(Input).append("\"");
  invCmd.append(",");
  invCmd.append("TheRewriter").append(",").append("*TheConsumer");
  invCmd.append(");");

  Interp.process(invCmd);

  // Only for test/OutputRedirect.C, but shouldn't affect performance too much.
  ::fflush(stdout);
  ::fflush(stderr);

  return checkDiagErrors(Interp.getCI());
}
