#pragma once
#include <vector>
#include <unordered_map>
#include <random>
#include "config.hpp"
#include "PriceSeries.hpp"

enum class SignalType {
    PRICE,
    MA_SHORT,
    MA_LONG,
    RSI,
    VOLATILITY,
    VOLATILITY_MA
};


class MarketSimulator {
public:
    MarketSimulator(const Config& cfg);
    
    // Phase 1
    void runMarket();

    // Phase 2
    void computeMovingAverage(int short_w, int long_w);

    // Access
    const std::vector<double>& getPrices() const;
    std::vector<SignalType> getAvailableSignals() const;
    double getSignal(SignalType type, int t) const;
    void computeRSI(int period);
    void computeVolatility(int window);
    void computeMovingAverageOnSignal(
        SignalType src,
        SignalType dst,
        int window
    );
    static std::string signalName(SignalType s);


private:
    // existing
    Config config;
    std::mt19937 rng;
    double stepTrending(double price);
    double stepSideways(double price);
    double stepMeanReverting(double price);
    // new
    std::vector<double> prices;
    std::unordered_map<SignalType, std::vector<double>> signals;
};



