#include "../include/strategy.hpp"
#include <algorithm>

bool Strategy::isValid(const MarketSimulator& sim) const {
    auto available = sim.getAvailableSignals();

    auto hasSignal = [&](SignalType s) {
        return std::find(available.begin(), available.end(), s)
               != available.end();
    };

    for (const auto& c : buy) {
        if (!hasSignal(c.lhs)) return false;
        if (c.rhs_type == OperandType::SIGNAL &&
            !hasSignal(c.rhs_signal))
            return false;
    }

    for (const auto& c : sell) {
        if (!hasSignal(c.lhs)) return false;
        if (c.rhs_type == OperandType::SIGNAL &&
            !hasSignal(c.rhs_signal))
            return false;
    }

    return true;
}


