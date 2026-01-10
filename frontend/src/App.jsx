import { useState } from "react";
import "./App.css";

function App() {
  const [market, setMarket] = useState("trending");
  const [shortWindow, setShortWindow] = useState(20);
  const [longWindow, setLongWindow] = useState(50);
  const [positionSize, setPositionSize] = useState(1.0);
  const [loading, setLoading] = useState(false);
  const [result, setResult] = useState(null);

  const handleRunSimulation = async () => {
    setLoading(true);
    try {
      const response = await fetch("http://localhost:8000/simulate", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
          market,
          timesteps: 100,
          seed: 42,
          strategy: {
            type: "ma_crossover",
            short_window: shortWindow,
            long_window: longWindow,
            position_size: positionSize
          }
        })
      });

      setResult(await response.json());
    } catch (e) {
      console.error(e);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="app">
      <header>
        <h1>Algorithmic Trading Sandbox</h1>
        <p className="disclaimer">
          Educational simulation only. No financial advice. No real trading.
        </p>
      </header>

      <div className="bento-grid">
        {/* Strategy Card */}
        <div className="card strategy">
          <h2>Strategy</h2>

          <label>Market Regime</label>
          <select value={market} onChange={(e) => setMarket(e.target.value)}>
            <option value="trending">Trending</option>
            <option value="sideways">Sideways</option>
          </select>

          <label>Short MA</label>
          <input
            type="number"
            value={shortWindow}
            onChange={(e) => setShortWindow(Number(e.target.value))}
          />

          <label>Long MA</label>
          <input
            type="number"
            value={longWindow}
            onChange={(e) => setLongWindow(Number(e.target.value))}
          />

          <label>Position Size</label>
          <input
            type="number"
            step="0.1"
            value={positionSize}
            onChange={(e) => setPositionSize(Number(e.target.value))}
          />

          <button onClick={handleRunSimulation} disabled={loading}>
            {loading ? "Running..." : "Run Simulation"}
          </button>
        </div>

        {/* Metrics Bento (4 Cards) */}
        {result?.metrics && (
          <div className="metrics-grid">
            <div
              className={`card metric-card ${
                result.metrics.total_pnl >= 0 ? "positive" : "negative"
              }`}
            >
              <span>Total PnL</span>
              <strong>{result.metrics.total_pnl}</strong>
            </div>

            <div className="card metric-card">
              <span>Max Drawdown</span>
              <strong>{result.metrics.max_drawdown}</strong>
            </div>

            <div className="card metric-card">
              <span>Win Rate</span>
              <strong>{(result.metrics.win_rate * 100).toFixed(1)}%</strong>
            </div>

            <div className="card metric-card">
              <span>Trades</span>
              <strong>{result.metrics.num_trades}</strong>
            </div>
          </div>
        )}
      </div>
    </div>
  );
}

export default App;
