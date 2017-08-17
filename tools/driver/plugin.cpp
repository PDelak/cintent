using namespace clang;

void plugin() {}

class PluginASTVisitor : public RecursiveASTVisitor<PluginASTVisitor> {
public:
  typedef RecursiveASTVisitor<PluginASTVisitor> Base;

  PluginASTVisitor(Rewriter &R, const std::string& f) : TheRewriter(R), filename(f) {}

  bool VisitDecl(Decl* decl) { return true; }
  bool VisitFunctionTemplateDecl(FunctionTemplateDecl* f) { return true; }

  bool VisitFunctionDecl(FunctionDecl *f) {
    std::cout << "VisitFunc" << std::endl;
    return true;
  }

private:
  Rewriter &TheRewriter;
  std::string filename;
};

class PluginASTConsumer : public ASTConsumer {
public:
  PluginASTConsumer(Rewriter &R, const std::string& filename) : Visitor(R, filename) {}

  virtual bool HandleTopLevelDecl(DeclGroupRef DR) {
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b)
      Visitor.TraverseDecl(*b);
    return true;
  }

private:
  PluginASTVisitor Visitor;
};

ASTConsumer* createASTConsumer(Rewriter& TheRewriter, const std::string& filename)
{
  return new PluginASTConsumer(TheRewriter, filename);
}
