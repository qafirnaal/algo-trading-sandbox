#include <iostream>
#include <fstream>
#include "Config.hpp"
#include "MarketSimulator.hpp"
#include "json.hpp"

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

    json output;
    output["price"] = series.prices;

    std::cout << output.dump() << std::endl;
    return 0;
}
