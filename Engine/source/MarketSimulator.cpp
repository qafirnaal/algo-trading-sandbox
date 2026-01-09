#include "../include/MarketSimulator.hpp"

MarketSimulator::MarketSimulator(const Config& cfg) : config(cfg), rng(cfg.seed){}

PriceSeries MarketSimulator::run() {
    PriceSeries series;
    double price = 100.0;
    series.add(price);
    for(int t = 1; t< config.timesteps; t++){
        if(config.market == "Trending"){
            price = stepTrending(price);
        }
        else{
            price = stepSideways(price);
        }
        series.add(price);
    }
    
    return series;
}

double MarketSimulator::stepTrending(double price) {
    std::normal_distribution<double> noise(0.0, 0.2);
    double drift = 0.05;
    double next = price + drift + noise(rng);
    return next > 0.0 ? next : 0.01;
}

double MarketSimulator::stepSideways(double price) {
    std::normal_distribution<double> noise(0.0, 0.2);
    double next = price + noise(rng);
    return next > 0.0 ? next : 0.01;
}
