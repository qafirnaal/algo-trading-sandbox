#ifndef MOVING_AVERAGE_STRATEGY_HPP
#define MOVING_AVERAGE_STRATEGY_HPP

#include <vector>
#include <string>


struct Trade {
    int t;
    std::string type;
    double price; // { "t": 120, "type": "BUY", "price": 102.4 }
};


class MovingAverageStrategy {
private:
    int short_window;
    int long_window;
    int position_size;

    bool position_open;
    double entry_price;

    std::vector<Trade> trades;
    std::vector<double> pnl_series;

public:
    MovingAverageStrategy(int sw, int lw, int ps);

    void run(const std::vector<double>& prices);

    double totalPnL() const;
    double maxDrawdown() const;
    
    int numberOfTrades() const;
    
    
    double winRate() const;
    

    const std::vector<Trade>& getTrades() const;
    
    
    const std::vector<double>& getPnLSeries() const;
};

#endif
