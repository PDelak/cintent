#include <algorithm>
#include <cstdio>
#include <memory>
#include <sstream>
#include <string>
#include <fstream>
#include <list>


#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include <iostream>

using namespace clang;

void setupCompilerInstance(CompilerInstance& TheCompInst)
{
    TheCompInst.createDiagnostics();

    LangOptions &lo = TheCompInst.getLangOpts();
    lo.CPlusPlus = 1;
#ifdef _WIN32
    lo.MicrosoftExt = 1;
    lo.MSVCCompat = lo.MSVC2015;
#endif
    lo.CPlusPlus11 = 1;
    lo.CPlusPlus14 = 1;

    // Targer triple
    auto TO = std::make_shared<TargetOptions>();
    TO->Triple = llvm::sys::getDefaultTargetTriple();
    TargetInfo *TI =
        TargetInfo::CreateTargetInfo(TheCompInst.getDiagnostics(), TO);
    TheCompInst.setTarget(TI);

    TheCompInst.createFileManager();
    TheCompInst.createSourceManager(TheCompInst.getFileManager());
    TheCompInst.createPreprocessor(TU_Module);

    TheCompInst.createASTContext();

}

void addIncludeDirectory(CompilerInstance& TheCompInst, const std::string& includeDirectory)
{
     TheCompInst.getHeaderSearchOpts().AddPath(includeDirectory, frontend::Angled, false, false);
 
}

llvm::cl::opt<std::string> InputFilename(llvm::cl::Positional, llvm::cl::Required, llvm::cl::desc("<input file>"));
llvm::cl::opt<std::string> Inject("inject", llvm::cl::desc("Specify config filename"), llvm::cl::value_desc("config"));
llvm::cl::opt<std::string> Runtime("runtime", llvm::cl::desc("Specify runtime"), llvm::cl::value_desc("runtime"));
llvm::cl::opt<bool> ReportFunctions("reportFunctions", llvm::cl::desc("Report all found functions"));
llvm::cl::list<std::string> Include("I", llvm::cl::desc("Specify include directory"), llvm::cl::value_desc("include directory"));



void HideNotNeededOptions(llvm::StringMap<llvm::cl::Option*> &Map)
{
    using namespace llvm;
    Map["rng-seed"]->setHiddenFlag(cl::Hidden);
    Map["static-func-full-module-prefix"]->setHiddenFlag(cl::Hidden);
    Map["stats"]->setHiddenFlag(cl::Hidden);
    Map["stats-json"]->setHiddenFlag(cl::Hidden);
    Map["version"]->setHiddenFlag(cl::Hidden);
}

int cintentrt() { return 0; }

int exec(const std::string& inFileName, Rewriter& TheRewriter, ASTConsumer& TheConsumer)
{
  using namespace llvm;
  HideNotNeededOptions(cl::getRegisteredOptions());

  InputFilename = inFileName;

  if (!std::ifstream(InputFilename).is_open()) {
      llvm::errs() << "File does not exists";
      return -1;
  }

  CompilerInstance TheCompInst;

  int num = Include.getNumOccurrences();

  for (int i = 0; i < num; ++i) {
      addIncludeDirectory(TheCompInst, Include[i]);
  }

  setupCompilerInstance(TheCompInst);

  FileManager &FileMgr = TheCompInst.getFileManager();
  SourceManager &SourceMgr = TheCompInst.getSourceManager();

  const FileEntry *FileIn = FileMgr.getFile(InputFilename);
  clang::FileID fileId = SourceMgr.createFileID(FileIn, SourceLocation(), SrcMgr::C_User);

  SourceMgr.setMainFileID(fileId);
  TheCompInst.getDiagnosticClient().BeginSourceFile(
      TheCompInst.getLangOpts(), &TheCompInst.getPreprocessor());


  TheRewriter.setSourceMgr(SourceMgr, TheCompInst.getLangOpts());

  ParseAST(TheCompInst.getPreprocessor(), &TheConsumer,
           TheCompInst.getASTContext());


  const RewriteBuffer *RewriteBuf = TheRewriter.getRewriteBufferFor(SourceMgr.getMainFileID());
  if (!RewriteBuf) return -1;

  std::string oldFile = InputFilename + ".old";
  rename(InputFilename.c_str(), oldFile.c_str());

  std::string outStr;

  outStr.append(std::string(RewriteBuf->begin(), RewriteBuf->end()));

  std::ofstream out(InputFilename);

  std::error_code EC;
  std::unique_ptr<llvm::raw_fd_ostream> o (new raw_fd_ostream(InputFilename, EC, sys::fs::F_None));
  o->SetUnbuffered();
  o->write(outStr.c_str(), outStr.size());


  return 0;
}

                                         