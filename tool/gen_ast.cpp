#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const static map<string, string> exprs = {
  {"Assign", "Token* name, Expr* value"},
  {"Binary", "Expr* left, Token* op, Expr* right"},
  {"Call", "Expr* callee, Token* paren, Vector<Expr*> arguments"},
  {"Get", "Expr* object, Token* name"},
  {"Grouping", "Expr* expression"},
  {"Literal", "Token* value"}, // TODO
  {"Logical", "Expr* left, Token* op, Expr* right"},
  {"Set", "Expr* object, Token* name, Expr* value"},
  {"Super", "Token* keyword, Token* method"},
  {"This", "Token* keyword"},
  {"Unary", "Token* op, Expr* right"},
  {"Variable", "Token* name"},
};

const static map<string, string> stmts = {
  {"Expression", "Expr* expression"},
  {"Print", "Expr* expression"},
  {"Var", "Token* name, Expr* initializer"},
  {"Block", "Vector<Stmt*> statements"},
  {"If", "Expr* condition, Stmt* thenBranch, Stmt* elseBranch"},
  {"While", "Expr* condition, Stmt* body"},
  {"Function", "Token* name, Vector<Token*> params, Vector<Stmt*> body"},
  {"Return", "Token* keyword, Expr* value"},
  {"Class", "Token* name, Variable* superclass, Vector<Function*> methods"},
};

// const static vector<string> exprVisitorTypes = {"string", "Value*", "void"};
// const static vector<string> stmtVisitorTypes = {"string", "void"};
const static vector<string> exprVisitorTypes = {"Value*", "void"};
const static vector<string> stmtVisitorTypes = {"void"};

vector<string> split(string str, char del) {
  vector<string> result;
  string subStr;

  for (const char c : str) {
    if (c == del) {
      if (subStr != "") result.push_back(subStr);
      subStr.clear();
    } else {
      subStr += c;
    }
  }

  if (subStr != "") result.push_back(subStr);
  return result;
}

string toMember(string str) {
  // str.at(0) = toupper(str.at(0));
  // return "m" + str;
  return str;
}

string toLower(const string &str) {
  string copy = str;
  transform(copy.begin(), copy.end(), copy.begin(), ::tolower);
  return copy;
}

string toUpper(const string &str) {
  string copy = str;
  transform(copy.begin(), copy.end(), copy.begin(), ::toupper);
  return copy;
}

string defineVisitor(const string &baseName, const map<string, string> &types) {
  stringstream ss;

  ss << "template <class R> class Visitor";
  ss << " { ";
  ss << " public: ";
  for (auto &[className, fieldsAsStr] : types) {
    ss << "virtual R visit(const " << className << " "
       << "*" + toLower(baseName) << ") = 0;";
  }
  ss << " }; ";
  ss << endl << endl;

  return ss.str();
}

string toConstRefArg(string &fieldEntry) {
  // fieldEntry.erase(0, fieldEntry.find_first_not_of(' '));
  // string ref = regex_replace(fieldEntry, regex(" "), " &");
  // return "const " + ref;
  return fieldEntry;
}

void defineAst(const string &baseName, const map<string, string> &types) {
  stringstream ss;

  for (auto &[className, fieldsAsStr] : types) {
    ss << "class " << className << ";";
  }
  ss << endl << endl;

  /* base class */
  ss << "struct " << baseName;
  ss << " { ";
  // ss << " public: ";
  ss << defineVisitor(baseName, types);
  ss << "V_" << toUpper(baseName) << "_ACCEPT_METHODS" << endl;
  ss << " }; ";
  ss << endl << endl;

  /* sub classes */
  for (auto &[className, fieldsAsStr] : types) {
    auto fields = split(fieldsAsStr, ',');

    ss << "struct " << className << " : public " << baseName;
    ss << " { ";
    // ss << "public: " << endl;

    // constructor
    ss << className;
    ss << " ( ";
    for (int i = 0; i < fields.size(); i++) {
      if (i != 0) ss << ", ";
      ss << toConstRefArg(fields[i]);
    }
    ss << " ) : ";
    for (int i = 0; i < fields.size(); i++) {
      auto varname = split(fields[i], ' ')[1];
      auto member = toMember(varname);

      if (i != 0) ss << ", ";
      ss << member << "(" << varname << ")";
    }
    ss << " {} ";
    ss << endl << endl;

    // member
    for (auto field : fields) {
      auto typeAndVarname = split(field, ' ');
      auto type = typeAndVarname[0];
      auto member = toMember(typeAndVarname[1]);

      ss << type << " " << member << ";";
    }
    ss << endl;

    // accept for visitor
    ss << endl;
    ss << toUpper(baseName) << "_ACCEPT_METHODS";

    ss << " };";
    ss << endl << endl;
  }

  cout << ss.str() << endl;
}

void defineMacro(const string &baseName, const vector<string> &visitorTypes) {
  // base
  cout << "#define V_" << toUpper(baseName) << "_ACCEPT_METHODS \\" << endl;
  for (int i = 0; i < visitorTypes.size(); i++) {
    if (i != 0) cout << " \\" << endl;
    cout << " virtual " << visitorTypes[i] << " accept(Visitor<" << visitorTypes[i]
         << "> &visitor) const = 0; ";
  }
  cout << endl << endl;
  // sub
  cout << "#define " << toUpper(baseName) << "_ACCEPT_METHODS \\" << endl;
  for (int i = 0; i < visitorTypes.size(); i++) {
    if (i != 0) cout << " \\" << endl;
    cout << visitorTypes[i] << " accept(Visitor<" << visitorTypes[i]
         << "> &visitor) const override { return visitor.visit(this); }";
  }
  cout << endl << endl;
}

bool IsStmt(int argc, char const *argv[]) {
  return argc > 1 && string(argv[1]) == "stmt";
}

int main(int argc, char const *argv[]) {
  cout << "#include \"../lib/vector.h\"\n" << endl;

  if (IsStmt(argc, argv))
    defineMacro("Stmt", stmtVisitorTypes);
  else
    defineMacro("Expr", exprVisitorTypes);

  cout << "namespace lox";
  cout << " { ";
  cout << endl << endl;
  cout << "class Token;";
  cout << "class Value;";
  if (IsStmt(argc, argv)) {
    cout << "class Variable;";
    cout << "class Expr;";
  }
  cout << endl << endl;

  if (IsStmt(argc, argv))
    defineAst("Stmt", stmts);
  else
    defineAst("Expr", exprs);

  cout << " };";

  return 0;
}
