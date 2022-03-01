#ifndef LOGICBLOCK_H
#define LOGICBLOCK_H

#include "Logic.hpp"
#include "LogicTerm/LogicTerm.hpp"
#include "Model.hpp"
#include <iostream>
#include <set>
#include <sys/types.h>
#include <vector>

namespace logicbase {
class LogicBlock : public Logic {
protected:
  std::set<LogicTerm, TermDepthComparator> clauses;
  Model *model{};
  bool convertWhenAssert;
  virtual void internal_reset() = 0;
  unsigned long long gid = 0;

public:
  LogicBlock(bool convertWhenAssert = false)
      : convertWhenAssert(convertWhenAssert) {}
  virtual ~LogicBlock() {}

  unsigned long long getNextId() { return gid++; };
  unsigned long long getId() { return gid; };

  Model *getModel() { return model; }

  virtual void dump(const LogicTerm &a, std::ostream &stream) {
    a.print(stream);
  }
  virtual void dumpAll(std::ostream &stream) {
    for (const LogicTerm &term : clauses) {
      dump(term, stream);
    }
  }

  void assertFormula(const LogicTerm &a) {
    if (a.getOpType() == OpType::AND) {
      for (const auto &clause : a.getNodes()) {
        clauses.insert(clause);
      }
    } else {
      clauses.insert(a);
    }
  }

  LogicTerm makeVariable(const std::string &name, CType type = CType::BOOL,
                         short bv_size = 32) {
    if (type == CType::BITVECTOR && bv_size == 0) {
      throw std::invalid_argument("bv_size must be > 0");
    }
    return LogicTerm(name, type, this, bv_size);
  }

  virtual void produceInstance() = 0;
  virtual Result solve() = 0;
  virtual void reset() {
    delete model;
    model = nullptr;
    clauses.clear();
    internal_reset();
    gid = 0;
  };
};

class LogicBlockOptimizer : public LogicBlock {
protected:
  std::vector<std::pair<LogicTerm, double>> weightedTerms;

public:
  LogicBlockOptimizer(bool convertWhenAssert) : LogicBlock(convertWhenAssert) {}
  virtual ~LogicBlockOptimizer() {}
  void weightedTerm(const LogicTerm &a, double weight) {
    weightedTerms.push_back(std::make_pair(a, weight));
  };
  void dumpAll(std::ostream &stream) override {
    for (const LogicTerm &term : clauses) {
      dump(term, stream);
      stream << std::endl;
      stream.flush();
    }
    for (const auto &it : weightedTerms) {
      dump(it.first, stream);
      stream << "(wt: " << it.second << ")" << std::endl;
      stream.flush();
    }
  }
  virtual bool makeMinimize() = 0;
  virtual bool makeMaximize() = 0;
  virtual bool maximize(const LogicTerm &term) = 0;
  virtual bool minimize(const LogicTerm &term) = 0;
  virtual void reset() override {
    model = nullptr;
    clauses.clear();
    weightedTerms.clear();
    internal_reset();
    gid = 0;
  };
};
} // namespace logicbase
#endif // LOGICBLOCK_H