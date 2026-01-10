#include "movingaverage.hpp"
#include <algorithm>
#include <iostream>




MovingAverageStrategy::MovingAverageStrategy(int sw, int lw, int ps)
    : short_window(sw),
      long_window(lw),
      position_size(ps),
      position_open(false),
      entry_price(0.0) {}

static double movingAverage(const std::vector<double>& prices,int t, int window){
    double sum = 0.0;
    for(int i = t - window+1; i <= t; i++)
        sum += prices[i];
    return sum/window;
    
    
    
}
void MovingAverageStrategy::run(const std::vector<double>& prices) {
    
    double cumulative_pnl = 0.0;
    
    for(int t=0; t<prices.size(); t++){
        pnl_series.push_back(cumulative_pnl);
        
        if(t<long_window){
            continue;
        }
        
        double short_ma = movingAverage(prices, t, short_window);
        
        double long_ma = movingAverage(prices, t, long_window);
        
        double prev_short = movingAverage(prices, t-1, short_window);
        
        double prev_long = movingAverage(prices, t-1, long_window);
        
        if(!position_open){
            if(prev_short<=prev_long && short_ma>long_ma){
                trades.push_back({t, "BUY", prices[t]});
                entry_price = prices[t];
                position_open = true;
                
            }
            
            
            }
        else{
            if (prev_short >= prev_long && short_ma < long_ma) {
                trades.push_back({ t, "SELL", prices[t] });
                cumulative_pnl += (prices[t] - entry_price) * position_size;
                position_open = false;
                
                pnl_series.back() = cumulative_pnl;     
                
            }
            
        
        }
        // force close all trades RAAAAAR
        
        if (position_open &&
                !prices.empty() &&
                !trades.empty() &&
            trades.back().type == "BUY") {
            
            int t = static_cast<int>(prices.size()) - 1;
            trades.push_back({ t, "SELL", prices[t] });
            cumulative_pnl += (prices[t] - entry_price) * position_size;
            position_open = false;
            pnl_series.back() = cumulative_pnl;
        }
        
        
    }
}
double MovingAverageStrategy::totalPnL() const {
    if(pnl_series.empty()){
        return 0.0;
        
    }
    else
        return pnl_series.back();
}
    
    
int MovingAverageStrategy::numberOfTrades() const {
        return static_cast<int>(trades.size()) / 2; // bcz each trade is one buy + sell so total trades
    }

double MovingAverageStrategy::winRate() const {

    int wins = 0;
    int total = 0;

    
    for (int i = 0; i + 1 < static_cast<int>(trades.size()); i += 2) { // start from buy

        double buy_price  = trades[i].price;
        
        
        
        double sell_price = trades[i + 1].price;

        if (sell_price > buy_price) {
            wins++;
        }
        
        
        
        

        total++;
        
        
        
    }

    return total == 0 ? 0.0 : static_cast<double>(wins) / total;
}

double MovingAverageStrategy::maxDrawdown() const {
    double peak = 0.0;
    double max_dd = 0.0;

    for (double pnl : pnl_series) {
        
        
        peak = std::max(peak, pnl);
        
        
        max_dd = std::max(max_dd, peak - pnl); // how much did it fall AFTER THE PEAK
        
        
    }

    return max_dd;
}


const std::vector<Trade>& MovingAverageStrategy::getTrades() const {
    return trades;
}

const std::vector<double>& MovingAverageStrategy::getPnLSeries() const {
    return pnl_series;
}



