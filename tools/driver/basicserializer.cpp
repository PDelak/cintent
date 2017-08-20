#include <map>
#include <string>
#include <utility>

using namespace clang;

void basicserializer() {}

class PluginASTVisitor : public RecursiveASTVisitor<PluginASTVisitor> {
public:
  typedef RecursiveASTVisitor<PluginASTVisitor> Base;

  PluginASTVisitor(Rewriter &R, const std::string& f) : TheRewriter(R), filename(f) {}

  bool VisitCXXRecordDecl(CXXRecordDecl* r)
  {
    std::cout << "record : " << r->getNameAsString() << ":" << r->field_empty() << std::endl;
    std::map<std::string, std::string> fieldMap;
    auto begin = r->field_begin();
    auto end = r->field_end();
    for(; begin != end; ++begin)
    {
      if(isa<FieldDecl>(*begin)) {
        FieldDecl* field = dyn_cast<FieldDecl>(*begin);
        std::cout << "field : " << field->getType().getAsString() << " " <<  field->getNameAsString() << std::endl;
        fieldMap[field->getNameAsString()] = field->getType().getAsString();
      }
    }
    auto locBegin = r->getLocStart();
    auto locEnd = r->getLocEnd();

    std::string output = "void serialize()\n";
    output.append("{\n");
    for(auto field : fieldMap) {
      output.append("std::cout << \"type: ");
      output.append(field.second);
      output.append(" \"");
      output.append(" << ");
      output.append(" \"name: ");
      output.append(field.first);
      output.append(" \"");
      output.append(" << ");
      output.append(field.first);
      output.append(" << ");
      output.append("std::endl;");
      output.append("\n");

    }
    output.append("}\n");

    TheRewriter.InsertTextBefore(locEnd, output);

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

