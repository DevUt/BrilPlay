#include "core.hpp"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/detail/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/properties.hpp>
#include <string>
#include <unordered_map>
#include <vector>

class VertexWriter {
public:
  VertexWriter(const CFG &g) : graph(g) {}

  void operator()(std::ostream &out, const CFG::vertex_descriptor v) const {
    out << "[label=\"" << graph[v].blockName << "\\n";
    for (auto &instr : graph[v].instructions) {
      out << instr.to_string() << "\\n";
    }
    out << "\"]";
  }

private:
  const CFG &graph;
};

class EdgeWriter {
public:
  EdgeWriter(const CFG &g) : graph(g) {}

  void operator()(std::ostream &out, const CFG::edge_descriptor e) const {
    if (!graph[e].label.empty()) {
      out << "[label=\"" << graph[e].label << "\"";
      out << " dir=one color=\""
          << (graph[e].type == Edge::CONDITIONAL ? "blue" : "red") << "\"]";
    } else {
      out << "[dir=one color=\"green\"]";
    }
  }

private:
  const CFG &graph;
};

class PostOrderVisitor : public boost::default_dfs_visitor {
private:
  std::vector<CFG::vertex_descriptor> &vertexOrder;

public:
  PostOrderVisitor(std::vector<CFG::vertex_descriptor> &vertexOrder)
      : vertexOrder(vertexOrder) {};
  void finish_vertex(const CFG::vertex_descriptor v, const CFG &g) {
    vertexOrder.push_back(v);
  }
};

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
      BasicBlock bb =
          BasicBlock({instr}, name + "_BLOCK_" + std::to_string(instr_cnt));
      basicBlocks.push_back(bb);

      CFG::vertex_descriptor vd = boost::add_vertex(bb, graph);
      if (rootBlock == -1) {
        rootBlock = vd;
      }
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
}

/**
 * Export to a .dot file
 */
void FunctionBlock::exportToDot(const std::string &filename) const {
  std::ofstream dot_file(filename);
  boost::write_graphviz(dot_file, graph, VertexWriter(graph),
                        EdgeWriter(graph));
}

/**
 * Get RPOrder of CFG of this function.
 */
std::vector<CFG::vertex_descriptor> FunctionBlock::computeRPO() const {
  std::vector<CFG::vertex_descriptor> post_order;
  std::vector<boost::default_color_type> colors(num_vertices(graph));

  PostOrderVisitor visitor(post_order);

  boost::depth_first_search(
      graph, visitor,
      make_iterator_property_map(colors.begin(),
                                 get(boost::vertex_index, graph)),
      rootBlock);
  return post_order;
}

/**
 * @brief Compute Post Order
 *
 */
std::vector<CFG::vertex_descriptor> FunctionBlock::computePO() const {
  std::vector<CFG::vertex_descriptor> post_order;
  std::vector<boost::default_color_type> colors(num_vertices(graph));

  PostOrderVisitor visitor(post_order);

  boost::depth_first_search(
      graph, visitor,
      make_iterator_property_map(colors.begin(),
                                 get(boost::vertex_index, graph)),
      rootBlock);
  std::reverse(post_order.begin(), post_order.end());
  return post_order;
}

/**
 * Retrieve basic block associated with the vertex descriptor
 */
BasicBlock
FunctionBlock::getBasicBlock(const CFG::vertex_descriptor &vd) const {
  return graph[vd];
}

/**
 * Get sucessor of the basic block
 */
std::vector<CFG::vertex_descriptor>
FunctionBlock::getSucessors(const CFG::vertex_descriptor &vd) const {
  std::vector<CFG::vertex_descriptor> successors;
  auto [ei_begin, ei_end] = out_edges(vd, graph);
  for (; ei_begin != ei_end; ++ei_begin) {
    successors.push_back(target(*ei_begin, graph));
  }
  return successors;
}

/**
 * Get predecssors of the basic block
 */
std::vector<CFG::vertex_descriptor>
FunctionBlock::getPredecessors(const CFG::vertex_descriptor &vd) const {
  std::vector<CFG::vertex_descriptor> pred;
  auto [ei_begin, ei_end] = boost::in_edges(vd, graph);
  for (; ei_begin != ei_end; ++ei_begin) {
    pred.push_back(source(*ei_begin, graph));
  }
  return pred;
}
