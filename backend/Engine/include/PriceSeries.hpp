#pragma once
#include <vector>

using namespace std;
class PriceSeries {
    public :
    vector<double> prices;
    void add (double price){
        prices.push_back(price);
    }
    
};
