#include "core.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

Program::Program(const std::vector<Instruction> &progFile) {}

Program::Program(std::ifstream &brilFile) {
  using json = nlohmann::json;
  using std::string;
  using std::vector;

  // If the file is not open throw an error
  // It is repsonsibility of the caller to check open and close
  if (!brilFile.is_open()) {
    throw std::runtime_error("Couldn't open the provided brilFile\n");
  }

  json brilJsonObj = json::parse(brilFile);

  /// No function hence it is an empty program
  if (!brilJsonObj.contains("functions")) {
    this->funcs = vector<FunctionBlock>();
  }

  vector<FunctionBlock> functionList;
  for (const auto &func : brilJsonObj["functions"]) {
    if (!func.contains("name")) {
      throw std::runtime_error("Unnamed Function");
    }
    string name = func["name"];

    /// Assign a return type to function. Look here to add types
    Type funcReturnType = NONE;
    if (func.contains("type")) {
      if (func["type"] == "int")
        funcReturnType = INT;
      else
        funcReturnType = BOOL;
    }

    /// Construct the args for the function.
    vector<std::pair<string, Type>> args;
    if (func.contains("args")) {
      for (const auto &arg : func["args"]) {
        if (arg.contains("name") && arg.contains("type")) {
          args.push_back(
              {string(arg["name"]), arg["type"] == "int" ? INT : BOOL});
        }
      }
    }

    /// Construct instructions for the function
    vector<Instruction> instructions;
    if (func.contains("instrs")) {
      for (const auto &instr : func["instrs"]) {
        if (instr.contains("op")) {
          string op;
          if (instr["op"] == "const") {
            op = "const";
            Type destType = instr["type"] == "int" ? INT : BOOL;

            string value = "NA";
            if (instr["value"].is_number()) {
              value = to_string(instr["value"]);
            } else {
              if (instr["value"]) {
                value = "true";
              } else {
                value = "false";
              }
            }

            instructions.push_back(Instruction(op, destType, value));
          } else {
            op = instr["op"];

            vector<string> args;
            if (instr.contains("args") && !(instr["args"].is_null() || instr["args"].empty()))
              for (const auto &arg : instr["args"]) {
                args.push_back(arg);
              }
            vector<string> funcs;
            if (instr.contains("funcs") && !(instr["funcs"].is_null() || instr["funcs"].empty()))
              for (const auto &fun : instr["funcs"]) {
                funcs.push_back(fun);
              }

            vector<string> labels;
            if (instr.contains("labels") && !(instr["labels"].is_null() || instr["labels"].empty()))
              for (const auto &label : instr["labels"]) {
                labels.push_back(label);
              }

            /// Value operation
            if (instr.contains("dest")) {
              string destination = instr["dest"];
              Type destType = instr["type"] == "int" ? INT : BOOL;
              instructions.push_back(
                  Instruction(op, destination, destType, args, funcs, labels));
            } else {
              /// Effect operation
              instructions.push_back(Instruction(op, args, funcs, labels));
            }
          }
        } else if (instr.contains("label")) {
          instructions.push_back(Instruction(instr["label"]));
        }
      } // END Search fo INSTR
    } // End INSTRS
    funcs.push_back(FunctionBlock(name, args, instructions, funcReturnType));
  }
}
