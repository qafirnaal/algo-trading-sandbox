#include "../include/MarketSimulator.hpp"
#include "../include/strategy.hpp"
#include "../include/config.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

// Based on your screenshot, json.hpp is in ../json/
#include "../json/json.hpp"

using json = nlohmann::json;

// ---------------------------------------------------------
// 1. HELPER FUNCTIONS
// ---------------------------------------------------------

// Map string names to SignalType Enum
SignalType signalFromString(const std::string& s) {
    if (s == "RSI") return SignalType::RSI;
    if (s == "VOLATILITY") return SignalType::VOLATILITY;
    if (s == "VOLATILITY_MA") return SignalType::VOLATILITY_MA;
    if (s == "MA" || s == "SMA") return SignalType::MA_SHORT;
    if (s == "MA_LONG") return SignalType::MA_LONG;
    if (s == "Price") return SignalType::PRICE;
    return SignalType::PRICE; // Default
}

// Compare LHS vs RHS (Constant or Signal)
inline bool evaluateCondition(const MarketSimulator& sim, const Condition& c, int t) {
    // Get Left Value
    double left = sim.getSignal(c.lhs, t);
    
    // Get Right Value
    double right = 0.0;
    if (c.rhs_type == OperandType::SIGNAL) {
        right = sim.getSignal(c.rhs_signal, t);
    } else {
        right = c.rhs_value;
    }

    // Compare
    if (c.op == '>') return left > right;
    if (c.op == '<') return left < right;
    if (c.op == '=') return std::abs(left - right) < 0.0001;
    return false;
}

// ---------------------------------------------------------
// 2. MAIN EXECUTION
// ---------------------------------------------------------

int main(int argc, char* argv[]) {
    // --- INPUT PARSING ---
    json input;
    
    // Check if a file argument was provided (e.g. for debugging)
    if (argc > 1) {
        std::ifstream f(argv[1]);
        if (f.is_open()) {
            f >> input;
        } else {
            // If file fails, return empty JSON to prevent python crash
            std::cout << "{ \"error\": \"Cannot open file\" }" << std::endl;
            return 1;
        }
    } else {
        // Default: Read from Python Pipe (stdin)
        try {
            // Check if stdin has data
            if (std::cin.peek() == std::ifstream::traits_type::eof()) {
                return 0; 
            }
            std::cin >> input;
        } catch (...) {
            std::cout << "{ \"error\": \"Invalid JSON input\" }" << std::endl;
            return 1;
        }
    }

    // --- MARKET CONFIG ---
    Config cfg;
    std::string mkt = input.value("market", "Trending");
    
    // Handle Frontend string differences
    if (mkt == "Mean Reversion" || mkt == "MeanReversion") cfg.market = "MeanReverting";
    else if (mkt == "Sideways") cfg.market = "Sideways";
    else cfg.market = "Trending";

    cfg.timesteps = input.value("timesteps", 1000);
    cfg.seed = input.value("seed", 42);

    // --- RUN SIMULATION ---
    MarketSimulator sim(cfg);
    sim.runMarket();

    // --- PRE-COMPUTE INDICATORS ---
    // We compute ALL indicators so the user can select any combination
    sim.computeRSI(14);
    sim.computeVolatility(20);
    sim.computeMovingAverage(20, 50); // Short=20, Long=50
    sim.computeMovingAverageOnSignal(SignalType::VOLATILITY, SignalType::VOLATILITY_MA, 50);

    const auto& prices = sim.getPrices();

    // --- PARSE STRATEGY ---
    Strategy strategy;
    strategy.name = "User Strategy";

    auto parseRules = [&](const json& rules, std::vector<Condition>& target) {
        for (const auto& r : rules) {
            Condition c;
            
            // 1. LHS
            c.lhs = signalFromString(r.value("lhs", "Price"));
            
            // 2. Operator
            std::string op = r.value("op", ">");
            c.op = op[0];

            // 3. RHS Logic
            std::string type = r.value("rhs_type", "CONSTANT");
            if (type == "SIGNAL") {
                c.rhs_type = OperandType::SIGNAL;
                // Read 'rhs_signal' from JSON, map to Enum
                c.rhs_signal = signalFromString(r.value("rhs_signal", "Price"));
            } else {
                c.rhs_type = OperandType::CONSTANT;
                c.rhs_value = r.value("rhs_value", 0.0);
            }
            
            target.push_back(c);
        }
    };

    if (input.contains("strategy")) {
        if (input["strategy"].contains("buy")) parseRules(input["strategy"]["buy"], strategy.buy);
        if (input["strategy"].contains("sell")) parseRules(input["strategy"]["sell"], strategy.sell);
    }

    // --- EXECUTE TRADES ---
    bool in_pos = false;
    double entry_price = 0.0;
    std::vector<json> trades; // Store trades for JSON output
    
    double equity = 0.0;
    int win_count = 0;
    int trade_count = 0;
    double max_dd = 0.0;
    double peak = 0.0;

    // Start at t=50 to allow indicators to warm up
    for (int t = 50; t < (int)prices.size(); t++) {
        
        // Evaluate BUY (AND logic)
        bool buy_signal = !strategy.buy.empty();
        for (const auto& c : strategy.buy) {
            if (!evaluateCondition(sim, c, t)) {
                buy_signal = false;
                break;
            }
        }

        // Evaluate SELL (AND logic)
        bool sell_signal = !strategy.sell.empty();
        for (const auto& c : strategy.sell) {
            if (!evaluateCondition(sim, c, t)) {
                sell_signal = false;
                break;
            }
        }

        // State Machine
        if (!in_pos && buy_signal) {
            in_pos = true;
            entry_price = prices[t];
            
            trades.push_back({
                {"t", t},
                {"type", "BUY"},
                {"price", prices[t]}
            });
        }
        else if (in_pos && sell_signal) {
            in_pos = false;
            double pnl = prices[t] - entry_price;
            equity += pnl;
            trade_count++;
            if (pnl > 0) win_count++;

            trades.push_back({
                {"t", t},
                {"type", "SELL"},
                {"price", prices[t]},
                {"pnl", pnl}
            });
        }

        // Track Max Drawdown
        peak = std::max(peak, equity);
        double dd = peak - equity;
        if (dd > max_dd) max_dd = dd;
    }

    // --- JSON OUTPUT ---
    json output;
    output["prices"] = prices;       // Frontend App.js expects "prices"
    output["trades"] = trades;       // Frontend App.js expects "trades"
    
    output["metrics"] = {
        {"total_pnl", std::round(equity * 100.0) / 100.0},
        {"num_trades", trade_count},
        {"win_rate", trade_count > 0 ? (double)win_count/trade_count : 0.0},
        {"max_drawdown", std::round(max_dd * 100.0) / 100.0}
    };

    // Print to stdout for Python to catch
    std::cout << output.dump() << std::endl;

    return 0;
}