#include "../include/MarketSimulator.hpp"
#include <cmath>
#include <stdexcept>

MarketSimulator::MarketSimulator(const Config& cfg)
    : config(cfg), rng(cfg.seed) {}

void MarketSimulator::runMarket() {
    prices.clear();
    double price = 100.0;
    prices.push_back(price);

    for (int t = 1; t < config.timesteps; t++) {
        if (config.market == "Trending")
            price = stepTrending(price);
        else if(config.market == "Sideways")
            price = stepSideways(price);
        else
            price = stepMeanReverting(price);

        prices.push_back(price);
    }
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

double MarketSimulator::stepMeanReverting(double price) {
    std::normal_distribution<double> noise(0.0, 0.3);
    double mean = 100.0;
    double k = 0.05;  // reversion strength
    
    double next = price + k * (mean - price) + noise(rng);
    return next > 0.0 ? next : 0.01;
}


static double mean(const std::vector<double>& v, int l, int r) {
    double s = 0.0;
    for (int i = l; i <= r; i++) s += v[i];
    return s / (r - l + 1);
}

void MarketSimulator::computeMovingAverage(int sw, int lw) {
    std::vector<double> short_ma(prices.size(), 0.0);
    std::vector<double> long_ma(prices.size(), 0.0);

    for (int t = 0; t < (int)prices.size(); t++) {
        if (t >= sw - 1)
            short_ma[t] = mean(prices, t - sw + 1, t);
        if (t >= lw - 1)
            long_ma[t] = mean(prices, t - lw + 1, t);
    }

    signals[SignalType::MA_SHORT] = short_ma;
    signals[SignalType::MA_LONG]  = long_ma;
}

void MarketSimulator::computeRSI(int period) {

    std::vector<double> rsi(prices.size(), 0.0);

    double gain = 0.0;
    double loss = 0.0;

    // initial average gain/loss
    for (int i = 1; i <= period; i++) {
        double diff = prices[i] - prices[i - 1];
        if (diff >= 0)
            gain += diff;
        else
            loss -= diff;
    }

    gain /= period;
    loss /= period;

    // first RSI value
    double rs = (loss == 0) ? 0 : gain / loss;
    rsi[period] = 100.0 - (100.0 / (1.0 + rs));

    // remaining RSI values (Wilder smoothing)
    for (int i = period + 1; i < (int)prices.size(); i++) {
        double diff = prices[i] - prices[i - 1];

        double g = diff > 0 ? diff : 0;
        double l = diff < 0 ? -diff : 0;

        gain = (gain * (period - 1) + g) / period;
        loss = (loss * (period - 1) + l) / period;

        rs = (loss == 0) ? 0 : gain / loss;
        rsi[i] = 100.0 - (100.0 / (1.0 + rs));
    }

    signals[SignalType::RSI] = rsi;
}



void MarketSimulator::computeVolatility(int window) {

    std::vector<double> vol(prices.size(), 0.0);

    for (int t = window; t < (int)prices.size(); t++) {

        double mean = 0.0;
        double sq_sum = 0.0;

        // compute log returns
        std::vector<double> rets;
        for (int i = t - window + 1; i <= t; i++) {
            double r = std::log(prices[i] / prices[i - 1]);
            rets.push_back(r);
            mean += r;
        }

        mean /= window;

        for (double r : rets)
            sq_sum += (r - mean) * (r - mean);

        vol[t] = std::sqrt(sq_sum / window);
    }

    signals[SignalType::VOLATILITY] = vol;
}

void MarketSimulator::computeMovingAverageOnSignal(
    SignalType src,
    SignalType dst,
    int window
) {
    auto it = signals.find(src);
    if (it == signals.end())
        throw std::runtime_error("Source signal not computed");

    const auto& v = it->second;
    std::vector<double> ma(v.size(), 0.0);

    for (int t = window - 1; t < (int)v.size(); t++) {
        double sum = 0.0;
        for (int i = t - window + 1; i <= t; i++)
            sum += v[i];
        ma[t] = sum / window;
    }

    signals[dst] = ma;
}

std::vector<SignalType> MarketSimulator::getAvailableSignals() const {
    std::vector<SignalType> out;
    for (const auto& kv : signals)
        out.push_back(kv.first);
    return out;
}


const std::vector<double>& MarketSimulator::getPrices() const {
    return prices;
}


std::string MarketSimulator::signalName(SignalType s) {
    switch (s) {
        case SignalType::PRICE: return "Price";
        case SignalType::RSI: return "RSI";
        case SignalType::VOLATILITY: return "Volatility";
        case SignalType::VOLATILITY_MA: return "Volatility MA";
        case SignalType::MA_SHORT: return "Short MA";
        case SignalType::MA_LONG: return "Long MA";
        default: return "Unknown";
    }
}

double MarketSimulator::getSignal(SignalType type, int t) const {
    auto it = signals.find(type);
    if (it == signals.end())
        throw std::runtime_error("Signal not computed");
    return it->second[t];
}
