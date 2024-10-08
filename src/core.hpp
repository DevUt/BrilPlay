#pragma once

#include <cassert>
#include <memory>
#include <ostream>
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
  enum InstrType {
    CONST_INSTR,
    VALUE_INSTR,
    EFFEC_INSTR,
    LABEL_INSTR,
    UNKNW_INSTR
  };

  InstrType instrType = UNKNW_INSTR;

public:
  /// The "opcode"
  const std::string op;

  // After this everything is optional

  /// Used for label kind of instruction
  std::string label_instr;

  /// The value from const type operations. MUST BE STRING
  std::string value;

  /// The destination of the operation
  const std::string dest;

  /// The type of destination
  const Type destType = NONE;

  /// The argument list each entry is the name of the variable
  const std::vector<std::string> arg;

  /// Names of functions referenced by the instruction
  const std::vector<std::string> funcs;

  /// Names of labels referenced by the instruction
  const std::vector<std::string> labels;

  Instruction() = delete;

  /// Constant kind of instructions. The opcode MUST be const
  Instruction(const std::string &dest, const Type &destType,
              const std::string value)
      : op("const"), dest(dest), destType(destType), value(value) {
    instrType = CONST_INSTR;
  };

  /// Value kind of instructions
  Instruction(const std::string &op, const std::string &dest,
              const Type &destType, const std::vector<std::string> &args,
              const std::vector<std::string> &funcs,
              const std::vector<std::string> &labels)
      : op(op), dest(dest), destType(destType), arg(args), funcs(funcs),
        labels(labels) {
    instrType = VALUE_INSTR;
  };

  /// Effect kind of instructions
  Instruction(const std::string &op, const std::vector<std::string> &args,
              const std::vector<std::string> &funcs,
              const std::vector<std::string> &labels)
      : op(op), arg(args), funcs(funcs), labels(labels) {
    instrType = VALUE_INSTR;
  };

  /// Label kind of instructions
  Instruction(const std::string &label) : label_instr(label) {
    instrType = LABEL_INSTR;
  };
};

/**
 * Class to support basic block. Each basic block has set
 * of instructions and a single pred and a single successor
 */
class BasicBlock {
  std::shared_ptr<BasicBlock> predecessor;
  std::shared_ptr<BasicBlock> successor;

public:
  bool isRootBasicBlock = false;
  const std::vector<Instruction> instructions;

  /**
   * Generate a BasicBlock containing these instructions.
   * WARNING THIS DOESN'T CHECK CORRECTNESS OF THE BLOCK.
   * To properly generate a basic block use Function Block
   */
  BasicBlock(const std::vector<Instruction> &);
  void set_predecssor(std::shared_ptr<BasicBlock>);
  void set_successor(std::shared_ptr<BasicBlock>);
};

/**
 * This interface gives access to the basic blocks control
 * flow graph (inside a single function) as both adjacency
 * list and adjacency matrix.
 */
class FunctionBlock {
private:

  /// Predecessors of this function block.
  std::vector<std::shared_ptr<FunctionBlock>> predecessor;

  /// Successors of this function block.
  std::vector<std::shared_ptr<FunctionBlock>> successors;

  /**
   * Contains the basic blocks constrained in this basic
   * blocks.The plan is to guarantee that the first element
   * to be the first basic block in the function. The rest
   * can be figured out with that basic block.
   */
  std::vector<BasicBlock> BasicBlocks;

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
  std::vector<Instruction> instrs;

  FunctionBlock(std::string name,
                std::vector<std::pair<std::string, Type>> args,
                std::vector<Instruction> instructions, Type returnType = NONE) : name(name), args(args), instrs(instructions), type(returnType) {};
  FunctionBlock(std::string name, std::vector<BasicBlock> rootBasicBlock);

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
