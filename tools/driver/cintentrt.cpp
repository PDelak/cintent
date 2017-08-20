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

int cintentrt() { return 0; }

int exec(const std::string& inputFilename, Rewriter& TheRewriter, ASTConsumer& TheConsumer)
{
  using namespace llvm;

  if (!std::ifstream(inputFilename).is_open()) {
      llvm::errs() << "File does not exists";
      return -1;
  }

  CompilerInstance TheCompInst;

  setupCompilerInstance(TheCompInst);

  FileManager &FileMgr = TheCompInst.getFileManager();
  SourceManager &SourceMgr = TheCompInst.getSourceManager();

  const FileEntry *FileIn = FileMgr.getFile(inputFilename);
  clang::FileID fileId = SourceMgr.createFileID(FileIn, SourceLocation(), SrcMgr::C_User);

  SourceMgr.setMainFileID(fileId);
  TheCompInst.getDiagnosticClient().BeginSourceFile(
      TheCompInst.getLangOpts(), &TheCompInst.getPreprocessor());


  TheRewriter.setSourceMgr(SourceMgr, TheCompInst.getLangOpts());

  ParseAST(TheCompInst.getPreprocessor(), &TheConsumer,
           TheCompInst.getASTContext());


  const RewriteBuffer *RewriteBuf = TheRewriter.getRewriteBufferFor(SourceMgr.getMainFileID());
  if (!RewriteBuf) return -1;

  std::string oldFile = inputFilename + ".old";
  rename(inputFilename.c_str(), oldFile.c_str());

  std::string outStr;

  outStr.append(std::string(RewriteBuf->begin(), RewriteBuf->end()));

  std::ofstream out(inputFilename);

  std::error_code EC;
  std::unique_ptr<llvm::raw_fd_ostream> o (new raw_fd_ostream(inputFilename, EC, sys::fs::F_None));
  o->SetUnbuffered();
  o->write(outStr.c_str(), outStr.size());


  return 0;
}



                                         