#include "core.hpp"
#include <fstream>
#include <iostream>

int main() {
  using std::cin, std::cout;
  cout << "Hello\n";
  std::string path;
  cout << "Enter path to a JSON brile file" << std::endl;
  cin >> path;

  std::ifstream brilF(path);
  Program pg(brilF);

  for (auto funcs : pg.funcs) {
    cout << funcs.name << "\n";
    cout << "INSTRUCTIONS :\n";
    for (auto &instrs : funcs.instrs) {
      if (instrs.op.empty())
        cout << instrs.label_instr << '\n';
      else
        cout << instrs.op << '\n';
    }
    cout<<"--------------------------\n";
  }
  brilF.close();
}
