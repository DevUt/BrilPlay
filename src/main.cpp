#include <core/core.hpp>
#include <fstream>
#include <iostream>
#include <ostream>


template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v){
  for(auto& x : v){
    os<<x<<' ';
  }
  os<<'\n';
  return os;
}

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
    for (auto &instrs : funcs.instructions) {
      if (instrs.op.empty())
        cout << instrs.label_instr << '\n';
      else
        cout << instrs.op << '\n';
    }
    cout<<"--------------------------\n";
  }

  for(auto& x : pg.funcs){
    x.exportToDot(x.name + ".dot");
    auto rpo = x.computeRPO();
    for(auto& vd : rpo){
      std::cout<<"Sucessors: \n";
      std::cout<<x.getBasicBlock(vd).blockName<<" : ";
      for(auto& succ : x.getSucessors(vd)){
        std::cout<<x.getBasicBlock(succ).blockName<<' ';
      }
      cout<<'\n';
    }
    for(auto& vd : rpo){
      std::cout<<"Predeccessors: \n";
      std::cout<<x.getBasicBlock(vd).blockName<<" : ";
      for(auto& pred : x.getPredecessors(vd)){
        std::cout<<x.getBasicBlock(pred).blockName<<' ';
      }
      cout<<'\n';
    }
  }
  brilF.close();
}
