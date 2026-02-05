import { useState, useMemo } from "react";
import "./App.css";

import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ResponsiveContainer
} from "recharts";

export default function App() {
  const [market, setMarket] = useState("Trending");
  const [timesteps, setTimesteps] = useState(1000);
  const [seed, setSeed] = useState(42);
  const [loading, setLoading] = useState(false);
  const [result, setResult] = useState(null);
  const [activeTab, setActiveTab] = useState("buy");

  // State with fixed default values
  const [strategy, setStrategy] = useState({
    buy: [{ lhs: "RSI", op: "<", rhs_type: "CONSTANT", rhs_value: 30 }],
    sell: [{ lhs: "RSI", op: ">", rhs_type: "CONSTANT", rhs_value: 70 }]
  });

  // FIX: These strings must EXACTLY match your C++ signalFromString function
  const indicators = ["RSI", "VOLATILITY", "VOLATILITY_MA", "MA" , "MA_LONG", "Price"];

  const handleUpdateRule = (side, index, field, value) => {
    const newRules = [...strategy[side]];
    newRules[index] = { ...newRules[index], [field]: value };
    setStrategy({ ...strategy, [side]: newRules });
  };

  const addRule = (side) => {
    setStrategy({
      ...strategy,
      [side]: [...strategy[side], { lhs: "Price", op: ">", rhs_type: "CONSTANT", rhs_value: 0 }]
    });
  };

  const removeRule = (side, index) => {
    const newRules = strategy[side].filter((_, i) => i !== index);
    setStrategy({ ...strategy, [side]: newRules });
  };

  const handleRunSimulation = async () => {
    setLoading(true);
    try {
      // Prepare payload - Engine expects "rhs_type" as "SIGNAL" or "CONSTANT"
      const payloadStrategy = {
        buy: strategy.buy.map(r => ({
          lhs: r.lhs,
          op: r.op,
          rhs_type: r.rhs_type,
          rhs_value: r.rhs_type === "CONSTANT" ? r.rhs_value : 0,
          rhs_signal: r.rhs_type === "SIGNAL" ? (r.rhs_signal || "RSI") : "Price"
        })),
        sell: strategy.sell.map(r => ({
          lhs: r.lhs,
          op: r.op,
          rhs_type: r.rhs_type,
          rhs_value: r.rhs_type === "CONSTANT" ? r.rhs_value : 0,
          rhs_signal: r.rhs_type === "SIGNAL" ? (r.rhs_signal || "RSI") : "Price"
        }))
      };

      const res = await fetch("http://localhost:8000/simulate", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
          market,
          timesteps,
          seed,
          strategy: payloadStrategy
        })
      });

      const data = await res.json();
      setResult(data);
    } catch (e) {
      console.error(e);
    } finally {
      setLoading(false);
    }
  };

  const priceSeries = useMemo(() => {
    if (!result?.prices) return [];
    return result.prices.map((p, i) => ({ t: i, price: p }));
  }, [result]);

  const pnlSeries = useMemo(() => {
    if (!result?.prices) return [];
    let currentPnl = 0;
    const series = [];
    
    // Quick lookup for trades to calculate PnL curve
    const tradeMap = {};
    if (result.trades) {
        result.trades.forEach(t => {
            // Only SELL trades realize PnL in this engine logic
            if(t.type === "SELL" && t.pnl) tradeMap[t.t] = t.pnl;
        });
    }

    for (let i = 0; i < result.prices.length; i++) {
        if (tradeMap[i]) currentPnl += tradeMap[i];
        series.push({ t: i, pnl: currentPnl });
    }
    return series;
  }, [result]);

  return (
    <div className="full-screen-wrapper">
      <header className="header">
        <div className="brand">
          <span className="logo-icon">∿</span>
          <span className="logo-text">AXIOM</span>
        </div>
        <div className="header-meta">
          Synthetic Markets • Deterministic • Educational
        </div>
      </header>

      <div className="main-dashboard">
        {/* LEFT PANEL */}
        <aside className="panel config-panel">
          <h3 className="label-tiny">● STRATEGY CONFIGURATION</h3>

          <div className="field">
            <label>MARKET REGIME</label>
            <div className="regime-toggle">
              {["Trending", "Sideways", "Mean Reversion"].map(m => (
                <div 
                  key={m} 
                  className={`opt ${market === m ? "active" : ""}`} 
                  onClick={() => setMarket(m)}
                >
                  <span className={m === "Trending" ? "dot" : "dash"}></span> {m}
                </div>
              ))}
            </div>
          </div>

          <div className="field">
            <label>LOGIC GATE</label>
            <div className="logic-tabs">
              <button 
                className={`tab-btn ${activeTab === "buy" ? "active" : ""}`} 
                onClick={() => setActiveTab("buy")}
              >BUY CONDITIONS</button>
              <button 
                className={`tab-btn ${activeTab === "sell" ? "active" : ""}`} 
                onClick={() => setActiveTab("sell")}
              >SELL CONDITIONS</button>
            </div>
          </div>

          <div className="rules-scroll-area">
            {strategy[activeTab].map((rule, idx) => (
              <div key={idx} className="rule-item">
                <div className="rule-inputs">
                  
                  {/* LHS Indicator */}
                  <select 
                    className="ui-input sm"
                    value={rule.lhs}
                    onChange={e => handleUpdateRule(activeTab, idx, "lhs", e.target.value)}
                  >
                    {indicators.map(i => <option key={i} value={i}>{i}</option>)}
                  </select>

                  {/* Operator */}
                  <select 
                    className="ui-input sm op"
                    value={rule.op}
                    onChange={e => handleUpdateRule(activeTab, idx, "op", e.target.value)}
                  >
                    <option value="<">&lt;</option>
                    <option value=">">&gt;</option>
                  </select>

                  {/* RHS Type */}
                  <select
                    className="ui-input sm op"
                    style={{fontSize: '9px', width: '50px'}}
                    value={rule.rhs_type}
                    onChange={e => handleUpdateRule(activeTab, idx, "rhs_type", e.target.value)}
                  >
                    <option value="CONSTANT">VAL</option>
                    <option value="SIGNAL">SIG</option>
                  </select>

                  {/* RHS Value */}
                  {rule.rhs_type === "CONSTANT" ? (
                    <input 
                      type="number"
                      className="ui-input sm"
                      value={rule.rhs_value}
                      onChange={e => handleUpdateRule(activeTab, idx, "rhs_value", +e.target.value)}
                    />
                  ) : (
                    <select 
                      className="ui-input sm"
                      value={rule.rhs_signal || "RSI"}
                      onChange={e => handleUpdateRule(activeTab, idx, "rhs_signal", e.target.value)}
                    >
                      {indicators.map(i => <option key={i} value={i}>{i}</option>)}
                    </select>
                  )}
                  
                  <button className="remove-rule" onClick={() => removeRule(activeTab, idx)}>×</button>
                </div>
              </div>
            ))}
            <button className="add-rule-link" onClick={() => addRule(activeTab)}>
              + ADD CONDITION
            </button>
          </div>

          <button className="run-button" onClick={handleRunSimulation}>
            {loading ? "..." : "▶ Run Simulation"}
          </button>
        </aside>

        {/* CENTER PANEL - CHARTS */}
        <section className="charts-area">
          <div className="chart-box">
            <h3 className="label-tiny">↗ PRICE EVOLUTION</h3>
            <div className="grid-bg">
              {priceSeries.length === 0 ? "Run simulation to see price data" : (
                <ResponsiveContainer width="100%" height="100%">
                  <LineChart data={priceSeries}>
                    <CartesianGrid stroke="#141e2e" vertical={false} />
                    <XAxis dataKey="t" hide />
                    <YAxis domain={['auto', 'auto']} hide />
                    <Tooltip contentStyle={{ backgroundColor: '#080f1a', border: '1px solid #141e2e' }} />
                    <Line type="monotone" dataKey="price" stroke="#00f2ff" strokeWidth={2} dot={false} />
                  </LineChart>
                </ResponsiveContainer>
              )}
            </div>
          </div>

          <div className="chart-box">
            <h3 className="label-tiny">$ CUMULATIVE PNL</h3>
            <div className="grid-bg">
              {pnlSeries.length === 0 ? "Run simulation to see PnL data" : (
                <ResponsiveContainer width="100%" height="100%">
                  <LineChart data={pnlSeries}>
                    <CartesianGrid stroke="#141e2e" vertical={false} />
                    <XAxis dataKey="t" hide />
                    <YAxis hide />
                    <Tooltip contentStyle={{ backgroundColor: '#080f1a', border: '1px solid #141e2e' }} />
                    <Line type="monotone" dataKey="pnl" stroke="#22ff88" strokeWidth={2} dot={false} />
                  </LineChart>
                </ResponsiveContainer>
              )}
            </div>
          </div>
        </section>

        {/* METRICS PANEL */}
        <aside className="panel metrics-panel">
            {result?.metrics ? (
                <>
                  <div className="m-card">
                    <label>TOTAL PNL</label>
                    <div className="m-val">{result.metrics.total_pnl}</div>
                  </div>
                  <div className="m-card">
                    <label>MAX DRAWDOWN</label>
                    <div className="m-val">{result.metrics.max_drawdown}%</div>
                  </div>
                  <div className="m-card">
                    <label>WIN RATE</label>
                    <div className="m-val">{(result.metrics.win_rate * 100).toFixed(1)}%</div>
                  </div>
                  <div className="m-card">
                    <label>TRADES</label>
                    <div className="m-val">{result.metrics.num_trades}</div>
                  </div>
                </>
            ) : (
                <div className="m-card status"><label>STATUS</label><div className="m-val cyan">READY</div></div>
            )}
        </aside>
      </div>
    </div>
  );
}