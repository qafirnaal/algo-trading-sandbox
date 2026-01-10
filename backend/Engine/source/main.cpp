#include <iostream>
#include <fstream>
#include "../include/config.hpp"
#include "../include/MarketSimulator.hpp"
#include "../json/json.hpp"
#include "../include/movingaverage.hpp"


using json = nlohmann::json;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: engine <input.json>\n";
        return 1;
    }

    std::ifstream inputFile(argv[1]);
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open input file\n";
        return 1;
    }

    json input;
    inputFile >> input;

    Config cfg;
    cfg.market = input["market"];
    cfg.timesteps = input["timesteps"];
    cfg.seed = input["seed"];

    MarketSimulator sim(cfg);
    PriceSeries series = sim.run();
    
    MovingAverageStrategy strategy(
        input["strategy"]["short_window"],
        input["strategy"]["long_window"],
        input["strategy"]["position_size"]
    );

    strategy.run(series.prices);

    json output;
    output["price"] = series.prices;
    json trades_json = json::array();
    for (const auto& trade : strategy.getTrades()) {
        trades_json.push_back({
            {"t", trade.t},
            {"type", trade.type},
            {"price", trade.price}
        });
    }
    output["trades"] = trades_json;
    output["metrics"] = {
        {"total_pnl", strategy.totalPnL()},
        {"max_drawdown", strategy.maxDrawdown()},
        {"num_trades", strategy.numberOfTrades()},
        {"win_rate", strategy.winRate()}
    };
    output["pnl_series"] = strategy.getPnLSeries();

    std::cout << output.dump() << std::endl;
    return 0;
}

