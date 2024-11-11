#include <core/core.hpp>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <unordered_map>

class LiveVariableAnalysis {
private:
  FunctionBlock func;
  using VarSet = std::set<std::string>;
  using LiveResult = std::unordered_map<CFG::vertex_descriptor, VarSet>;

public:
  LiveResult LiveIn;
  LiveResult LiveOut;
  LiveVariableAnalysis(FunctionBlock func) : func(func) {
    for (const auto &bb : func.computeRPO()) {
      LiveIn[bb] = VarSet();
      LiveOut[bb] = VarSet();
    }
  }

  VarSet getUsage(const Instruction &instr) const {
    VarSet uses;

    // Currently just assume anything in arg is being used.
    for (const std::string &arg : instr.arg) {
      uses.insert(arg);
    }
    return uses;
  }

  VarSet getDefs(const Instruction &instr) const {
    VarSet defs;

    // Only one variable being can be defined
    defs.insert(instr.dest);
    return defs;
  }

  /**
   * @brief Returns Gen and Kill for a Basic Block
   *
   * @param[in] instr Set of instructions to run this on
   * @return It returns two set `Gen` and `Kill`
   */
  std::pair<VarSet, VarSet>
  computeGenKill(const std::vector<Instruction> &instrs) const {
    VarSet Gen, Kill;

    for (const auto &instr : instrs) {
      // FIRST usage then def
      VarSet usage = getUsage(instr);
      for (const auto &use : usage) {
        // It should not preceed its Definition
        if (!Kill.contains(use)) {
          Gen.insert(use);
        }
      }

      // Everything defined is killed
      VarSet def = getDefs(instr);
      Kill.insert(def.begin(), def.end());
    }

    return {Gen, Kill};
  }

  /**
   * @brief Perform merge operation for computing out of a basic block
   *
   * @param[in] successor Vertex descriptor of the successor of the block
   * @return The `VarSet` containing the result
   */
  VarSet mergeOut(const std::vector<CFG::vertex_descriptor> &successor) const {
    VarSet result;
    for (const CFG::vertex_descriptor &succ : successor) {
      std::vector<std::string> temp_result;
      std::set_union(result.begin(), result.end(), LiveIn.at(succ).begin(),
                     LiveIn.at(succ).end(), std::back_inserter(temp_result));
      result.insert(temp_result.begin(), temp_result.end());
    }
    return result;
  }

  void analyze() {
    bool changed = false;
    std::vector<CFG::vertex_descriptor> rpo = func.computeRPO();

    do {
      changed = false;
      for (const auto &bb_descriptor : rpo) {
        auto [Gen, Kill] =
            computeGenKill(func.getBasicBlock(bb_descriptor).instructions);

        // Check for change
        VarSet old_LiveOut = LiveOut[bb_descriptor];
        VarSet old_LiveIn = LiveIn[bb_descriptor];

        // Compute Out
        LiveOut[bb_descriptor] = mergeOut(func.getSucessors(bb_descriptor));

        // Now LiveIn = Gen U (LiveOut - Kill)
        std::vector<std::string> temp;
        std::set_difference(LiveOut[bb_descriptor].begin(),
                            LiveOut[bb_descriptor].end(), Kill.begin(),
                            Kill.end(), std::back_inserter(temp));

        // LiveOut - Kill
        VarSet tempDiff(temp.begin(), temp.end());

        temp = std::vector<std::string>();
        std::set_union(tempDiff.begin(), tempDiff.end(), Gen.begin(), Gen.end(),
                       std::back_inserter(temp));

        LiveIn[bb_descriptor] = VarSet(temp.begin(), temp.end());

        if (old_LiveOut != LiveOut[bb_descriptor]) {
          changed = true;
        } else if (old_LiveIn != LiveIn[bb_descriptor]) {
          changed = true;
        }
      }

    } while (changed);
  }
};

int main() {
  std::cout << "Hey!\n";
  std::cout << "Enter a Bril JSON File Path" << std::endl;

  std::string path;
  std::cin >> path;

  std::ifstream progFile(path);
  if (progFile.bad()) {
    std::cout << "Cannot access the file" << std::endl;
    exit(EXIT_FAILURE);
  }

  Program pg(progFile);
  for (auto &func : pg.funcs) {
    LiveVariableAnalysis LivVar(func);
    LivVar.analyze();
    std::cout << "-------------------------------------------\n";
    std::cout << func.name << '\n';
    std::cout << "-------------------------------------------\n";

    for (const auto &vd : func.computeRPO()) {
      std::cout << "For Basic Block : " << func.getBasicBlock(vd).blockName
                << '\n';
      std::cout << "-------------------------------------------\n";
      std::cout << "LiveIn: ";
      for (const auto &var : LivVar.LiveIn[vd]) {
        std::cout << var << ' ';
      }
      std::cout<<'\n';
      std::cout << "LiveOut: ";
      for (const auto &var : LivVar.LiveOut[vd]) {
        std::cout << var << ' ';
      }
      std::cout << "\n\n";
    }
  }
}
