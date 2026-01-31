#include "../include/MarketSimulator.hpp"
#include "../include/strategy.hpp"
#include "../include/config.hpp"
#include <iostream>
#include <vector>

inline bool evaluateCondition(
    const MarketSimulator& sim,
    const Condition& c,
    int t
) {
    double left = sim.getSignal(c.lhs, t);
    double right = (c.rhs_type == OperandType::SIGNAL)
                   ? sim.getSignal(c.rhs_signal, t)
                   : c.rhs_value;

    if (c.op == '>') return left > right;
    if (c.op == '<') return left < right;
    return false;
}


int main() {

    // --------------------------------------------------
    // 1. Market config
    // --------------------------------------------------
    Config cfg;
    cfg.timesteps = 2000;
    cfg.market = "Mean Reverting";
    cfg.seed = 42;

    MarketSimulator sim(cfg);
    sim.runMarket();

    // --------------------------------------------------
    // 2. Compute signals
    // --------------------------------------------------
    sim.computeRSI(14);
    sim.computeVolatility(20);
    sim.computeMovingAverageOnSignal(
        SignalType::VOLATILITY,
        SignalType::VOLATILITY_MA,
        50
    );

    const auto& prices = sim.getPrices();

    // --------------------------------------------------
    // 3. Define strategies
    // --------------------------------------------------
    Strategy strategy;
    strategy.name = "User Strategy";

    // BUY conditions
    strategy.buy.push_back({
        SignalType::RSI, '<',
        OperandType::CONSTANT,
        SignalType::RSI,
        30.0
    });

    strategy.buy.push_back({
        SignalType::VOLATILITY, '>',
        OperandType::SIGNAL,
        SignalType::VOLATILITY_MA,
        0.0
    });

    // SELL conditions
    strategy.sell.push_back({
        SignalType::RSI, '>',
        OperandType::CONSTANT,
        SignalType::RSI,
        70.0
    });

    strategy.sell.push_back({
        SignalType::VOLATILITY, '<',
        OperandType::SIGNAL,
        SignalType::VOLATILITY_MA,
        0.0
    });

    std::vector<Strategy> strategies = {
        strategy
    };

    std::vector<StrategyState> states(strategies.size());

    // --------------------------------------------------
    // 4. Run strategies
    // --------------------------------------------------
    for (int t = 50; t < (int)prices.size(); t++) {

        for (size_t i = 0; i < strategies.size(); i++) {

            auto& strat = strategies[i];
            auto& state = states[i];

            bool buy = true;
            for (const auto& cond : strat.buy)
                buy &= evaluateCondition(sim, cond, t);

            bool sell = true;
            for (const auto& cond : strat.sell)
                sell &= evaluateCondition(sim, cond, t);

            if (!state.position_open && buy) {
                state.position_open = true;
                state.entry_price = prices[t];

                std::cout << "[" << strat.name << "] BUY @ "
                          << prices[t] << " (t=" << t << ")\n";
            }
            else if (state.position_open && sell) {
                state.position_open = false;

                double pnl = prices[t] - state.entry_price;
                std::cout << "[" << strat.name << "] SELL @ "
                          << prices[t] << " | PnL=" << pnl
                          << " (t=" << t << ")\n";
            }
        }
    }

    // --------------------------------------------------
    // 5. Force close
    // --------------------------------------------------
    for (size_t i = 0; i < strategies.size(); i++) {
        if (states[i].position_open) {
            double pnl = prices.back() - states[i].entry_price;
            std::cout << "[" << strategies[i].name
                      << "] FORCED EXIT | PnL=" << pnl << "\n";
        }
    }
}
