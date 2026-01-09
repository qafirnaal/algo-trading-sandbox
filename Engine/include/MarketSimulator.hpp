#pragma once
#include "config.hpp"
#include "PriceSeries.hpp"
#include <random>
using namespace std;
class MarketSimulator{
public:
    explicit MarketSimulator(const Config &cfg);
    PriceSeries run();

private:
    Config config;
    mt19937 rng;
    
    
    double stepTrending(double price);
    double stepSideways(double price);
    
};

