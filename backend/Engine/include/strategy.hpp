#pragma once
#include "MarketSimulator.hpp"
#include <vector>
#include <string>

enum class OperandType {
    CONSTANT,
    SIGNAL
};

struct Condition {
    SignalType lhs;
    char op;                 // '<' or '>'
    OperandType rhs_type;
    SignalType rhs_signal;   // valid if rhs_type == SIGNAL
    double rhs_value;        // valid if rhs_type == CONSTANT
};

enum class LogicType { AND, OR };

enum class Operator {  // unused so far, future extension
    LT,
    GT
};

struct Strategy {
    std::string name;
    LogicType buy_logic = LogicType::AND;
    LogicType sell_logic = LogicType::AND;
    std::vector<Condition> buy;
    std::vector<Condition> sell;
    bool isValid(const MarketSimulator& sim) const;
};

struct StrategyState {
    bool position_open = false;
    double entry_price = 0.0;
    double pnl = 0.0;
};


