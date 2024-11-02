#include "core.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <string>
#include <unordered_map>
#include <vector>

FunctionBlock::FunctionBlock(std::string name,
                             std::vector<std::pair<std::string, Type>> args,
                             std::vector<Instruction> instructions,
                             Type returnType = NONE)
    : name(name), args(args), instructions(instructions), type(returnType) {

  // Map each label to instruction
  std::unordered_map<std::string, int> label_to_instr;
  std::unordered_map<int, CFG::vertex_descriptor> vd_map;
  std::vector<std::string> label_store;
  int instr_cnt = 0;
  for (Instruction &instr : instructions) {
    if (instr.instrType == Instruction::LABEL_INSTR) {
      label_store.push_back(instr.label_instr);
    } else {
      for (std::string &x : label_store) {
        label_to_instr[x] = instr_cnt;
      }
      CFG::vertex_descriptor vd = boost::add_vertex(
          BasicBlock({instr}, name + "_BLOCK_" + std::to_string(instr_cnt)),
          graph);
      vd_map[instr_cnt++] = vd;
      label_store.clear();
    }
  }

  bool jumped = false;
  for (int i = 0; Instruction & instr : instructions) {
    if (instr.instrType != Instruction::LABEL_INSTR) {
      if (!jumped && i) {
        Edge e("", Edge::FLOW);
        boost::add_edge(vd_map[i - 1], vd_map[i], e, graph);
      }
      if (instr.op == "jmp") {
        Edge e(instr.labels[0], Edge::UNCONDITIONAL);
        boost::add_edge(vd_map[i], vd_map[label_to_instr[instr.labels[0]]], e,
                        graph);
        jumped = true;
      } else if (instr.op == "br") {
        Edge e(instr.labels[0], Edge::CONDITIONAL);
        boost::add_edge(vd_map[i], vd_map[label_to_instr[instr.labels[0]]], e,
                        graph);
        Edge e2(instr.labels[1], Edge::CONDITIONAL);
        boost::add_edge(vd_map[i], vd_map[label_to_instr[instr.labels[1]]], e2,
                        graph);
        jumped = true;
      } else {
        jumped = false;
      }
      i++;
    }
  }

};
