#pragma once

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <string>
#include <vector>

enum Type {
  INT,
  BOOL,
  NONE,
};

/*
 * Each Instruction instance identifies the type of instruction.
 * The instruction in string format.
 * What more?
 */
class Instruction {
private:
  std::string type_to_str(const Type type) const {
    switch (type) {
    case INT:
      return "int";
    case BOOL:
      return "bool";
    default:
      return "unknown";
      break;
    }
  }

public:
  enum InstrType {
    CONST_INSTR,
    VALUE_INSTR,
    EFFEC_INSTR,
    LABEL_INSTR,
    UNKNW_INSTR
  };

  InstrType instrType = UNKNW_INSTR;

  /// The "opcode"
  std::string op;

  // After this everything is optional

  /// Used for label kind of instruction
  std::string label_instr;

  /// The value from  type operations. MUST BE STRING
  std::string value;

  /// The destination of the operation
  std::string dest;

  /// The type of destination
  Type destType = NONE;

  /// The argument list each entry is the name of the variable
  std::vector<std::string> arg;

  /// Names of functions referenced by the instruction
  std::vector<std::string> funcs;

  /// Names of labels referenced by the instruction
  std::vector<std::string> labels;

  Instruction() = default;

  /// Constant kind of instructions. The opcode MUST be const
  Instruction(const std::string dest, const Type destType,
              const std::string value)
      : op("const"), dest(dest), destType(destType), value(value) {
    instrType = CONST_INSTR;
  };

  /// Value kind of instructions
  Instruction(const std::string op, const std::string dest, const Type destType,
              const std::vector<std::string> args,
              const std::vector<std::string> &funcs,
              const std::vector<std::string> &labels)
      : op(op), dest(dest), destType(destType), arg(args), funcs(funcs),
        labels(labels) {
    instrType = VALUE_INSTR;
  };

  /// Effect kind of instructions
  Instruction(const std::string op, const std::vector<std::string> args,
              const std::vector<std::string> funcs,
              const std::vector<std::string> labels)
      : op(op), arg(args), funcs(funcs), labels(labels) {
    instrType = EFFEC_INSTR;
  };

  /// Label kind of instructions
  Instruction(const std::string label) : label_instr(label) {
    instrType = LABEL_INSTR;
  };

  std::string to_string() const {
    std::string ret;
    switch (instrType) {
    case CONST_INSTR:
      ret = (dest + " : ") + type_to_str(destType) + " = const " + value;
      break;
    case VALUE_INSTR:
    case EFFEC_INSTR:
      if (dest.empty()) {
        ret = op + " ";
      } else {
        ret = (dest + " : ") + type_to_str(destType) + " " + op + " ";
      }

      for (const std::string &str : arg) {
        ret += (str + " ");
      }
      for (const std::string &str : funcs) {
        ret += (str + " ");
      }
      for (const std::string &str : labels) {
        ret += (str + " ");
      }
      break;
    case LABEL_INSTR:
      ret = label_instr;
      break;
    case UNKNW_INSTR:
      ret = "UNKOWN";
      break;
    }
    return ret;
  }
};

/**
 * Class to support basic block. Each basic block has set
 * of instructions and a single pred and a single successor
 */
class BasicBlock {

public:
  enum BlockType { START, END, NORMAL };

  std::string blockName;
  BlockType type = NORMAL;
  std::vector<Instruction> instructions;

  /**
   * Generate a BasicBlock containing these instructions.
   * WARNING THIS DOESN'T CHECK CORRECTNESS OF THE BLOCK.
   * To properly generate a basic block use Function Block
   */
  BasicBlock(const std::vector<Instruction> instructions,
             const std::string blockName)
      : instructions(instructions), blockName(blockName) {};
  BasicBlock(const std::string blockName) : blockName(blockName) {};
  BasicBlock(const std::string blockName, const BlockType type)
      : blockName(blockName), type(type) {};
  BasicBlock() = default;
};

/**
 * Class to support Edges to and from basic blocks.
 */
class Edge {
public:
  std::string label;
  enum EdgeType {
    CONDITIONAL,
    UNCONDITIONAL,
    FLOW,
  };

  EdgeType type;
  Edge(std::string label, EdgeType type) : label(label), type(type) {};
  Edge() = default;
};

using CFG = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
                                  BasicBlock, Edge, boost::no_property>;

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
      out << "[label=\"" << graph[e].label << "\"]";
    }
  }

private:
  const CFG &graph;
};
/**
 * This interface gives access to the basic blocks control
 * flow graph (inside a single function) as both adjacency
 * list and adjacency matrix.
 */
class FunctionBlock {
public:
  /// Is this the function that is run on entry point
  bool isRootFunction = false;
  /// The function name [MUST BE UNIQUE THROUGHOUT THE PROGRAM]
  std::string name;

  /// Return type of the function, is set to `None` if not provided
  Type type;

  /// Arguments to the function
  std::vector<std::pair<std::string, Type>> args;

  /// Instructions in the function. This is used to construct BB
  std::vector<Instruction> instructions;

  /// Instructions in the function. This is used to construct BB
  std::vector<BasicBlock> basicBlocks;

  /// Control Flow Graph of this function
  CFG graph;

  FunctionBlock(std::string, std::vector<std::pair<std::string, Type>>,
                std::vector<Instruction>, Type);
  // FunctionBlock(std::string name, std::vector<BasicBlock> rootBasicBlock);

  void exportToDot(const std::string &filename) {
    std::ofstream dot_file(filename);
    boost::write_graphviz(dot_file, graph, VertexWriter(graph),
                          EdgeWriter(graph));
  };
};
/**
 * Each program is set of functions. This class provides that
 * abstraction. This will store the program as a call graph.
 * The call graph can be accessed node wise.
 */
class Program {
public:
  std::vector<FunctionBlock> funcs;
  Program(const std::vector<Instruction> &progFile);
  Program(std::ifstream &brilFile);
};
