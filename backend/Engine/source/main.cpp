#include "../include/MarketSimulator.hpp"
#include "../include/strategy.hpp"
#include "../include/config.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "../json/json.hpp"

using json = nlohmann::json;

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

SignalType signalFromString(const std::string& s) {
    if (s == "RSI") return SignalType::RSI;
    if (s == "VOLATILITY") return SignalType::VOLATILITY;
    if (s == "VOLATILITY_MA") return SignalType::VOLATILITY_MA;
    if (s == "MA_SHORT") return SignalType::MA_SHORT;
    if (s == "MA_LONG") return SignalType::MA_LONG;
    throw std::runtime_error("Unknown signal: " + s);
}


int main() {
    json input;
    std::cin >> input;
    Config cfg;
    cfg.market = input["market"];
    cfg.timesteps = input["timesteps"];
    cfg.seed = input["seed"];

    MarketSimulator sim(cfg);
    sim.runMarket();
    sim.computeRSI(14);
    sim.computeVolatility(20);
    sim.computeMovingAverageOnSignal(
        SignalType::VOLATILITY,
        SignalType::VOLATILITY_MA,
        50
    );

    const auto& prices = sim.getPrices();

    Strategy strategy;
    strategy.name = "User Strategy";

    for (const auto& c : input["strategy"]["buy"]) {
        Condition cond;
        cond.lhs = signalFromString(c["lhs"]);
        cond.op = c["op"].get<std::string>()[0];

        if (c["rhs_type"] == "CONSTANT") {
            cond.rhs_type = OperandType::CONSTANT;
            cond.rhs_value = c["rhs_value"];
        } else {
            cond.rhs_type = OperandType::SIGNAL;
            cond.rhs_signal = signalFromString(c["rhs_signal"]);
        }

        strategy.buy.push_back(cond);
    }

    for (const auto& c : input["strategy"]["sell"]) {
        Condition cond;
        cond.lhs = signalFromString(c["lhs"]);
        cond.op = c["op"].get<std::string>()[0];

        if (c["rhs_type"] == "CONSTANT") {
            cond.rhs_type = OperandType::CONSTANT;
            cond.rhs_value = c["rhs_value"];
        } else {
            cond.rhs_type = OperandType::SIGNAL;
            cond.rhs_signal = signalFromString(c["rhs_signal"]);
        }

        strategy.sell.push_back(cond);
    }

    bool position_open = false;
    double entry_price = 0.0;

    std::vector<json> trades;
    double equity = 0.0;
    double peak = 0.0;
    double max_dd = 0.0;
    int wins = 0;

    for (int t = 50; t < (int)prices.size(); t++) {
        bool buy = true;
        for (const auto& cond : strategy.buy)
            buy &= evaluateCondition(sim, cond, t);

        bool sell = true;
        for (const auto& cond : strategy.sell)
            sell &= evaluateCondition(sim, cond, t);

        if (!position_open && buy) {
            position_open = true;
            entry_price = prices[t];
            trades.push_back({
                {"t", t},
                {"type", "BUY"},
                {"price", prices[t]}
            });
        }
        else if (position_open && sell) {
            position_open = false;
            double pnl = prices[t] - entry_price;

            equity += pnl;
            if (pnl > 0) wins++;

            peak = std::max(peak, equity);
            max_dd = std::max(max_dd, peak - equity);

            trades.push_back({
                {"t", t},
                {"type", "SELL"},
                {"price", prices[t]},
                {"pnl", pnl}
            });
        }
    }

    int num_trades = wins > 0 ? wins : trades.size() / 2;

    json output;
    output["prices"] = prices;
    output["trades"] = trades;
    output["metrics"] = {
        {"total_pnl", equity},
        {"max_drawdown", max_dd},
        {"win_rate", num_trades > 0 ? (double)wins / num_trades : 0.0},
        {"num_trades", num_trades}
    };

    std::cout << output.dump(2) << std::endl;
    return 0;
}
