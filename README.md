# Educational Algorithmic Trading Simulator

AlgoSim is a **simulation-first, educational algorithmic trading sandbox** designed to help users understand how algorithmic strategies behave under different market conditions — **without advanced math, complex coding, or real money**.

The focus is on **learning and experimentation**, not live trading or financial advice.

> This project is for **educational purposes only**.  
> It does **not** provide investment advice and does **not** execute real trades.

---

## Motivation

Most algorithmic trading platforms today are built for:
- experienced quants
- professional traders
- advanced programmers

AlgoSim aims to **lower the barrier to entry** by:
- using **simple, explainable strategies**
- running them on **synthetic markets**
- showing *why* a strategy works or fails, not just the outcome

---

## Core Principles

- **Simulation-first** (no live data, no brokers)
- **Deterministic & reproducible**
- **Explainability over profit**
- **Clear separation of concerns**
- **Beginner-friendly abstractions**

---

## Architecture Overview

```
React (Frontend)
      ↓
FastAPI (Python Backend)
      ↓
C++ Simulation Engine (this repository)
```

- **React** handles user input and visualization  
- **FastAPI** acts as a thin glue layer  
- **C++ Engine** performs market simulation  

This repository contains the **C++ simulation engine**.

---

## C++ Engine Responsibilities

The engine is a **command-line program** that:

- Reads configuration from a JSON file
- Generates **synthetic market price series**
- Uses seeded RNG for **deterministic results**
- Outputs results as JSON to `stdout`

The engine deliberately **does not**:
- fetch real market data
- execute real trades
- compute strategy metrics
- contain UI or backend logic

---

## Input Format

The engine is invoked as:

```bash
./engine input.json
```

### Example `input.json`

```json
{
  "market": "trending",
  "timesteps": 1000,
  "seed": 42
}
```

### Parameters
- `market`: `"trending"` or `"sideways"`
- `timesteps`: number of simulation steps
- `seed`: random seed for reproducibility

---

## Output Format

The engine prints JSON to **stdout**.

### Example Output

```json
{
  "price": [100.0, 100.12, 99.97, 100.21]
}
```

This output is consumed by FastAPI and forwarded to the frontend.

---

## Market Models (V1)

### Trending Market
- Positive drift + Gaussian noise

### Sideways Market
- Zero drift + Gaussian noise

Both models:
- are fully synthetic
- deterministic given a seed
- clamp prices to remain positive

---

## Build & Run

### Requirements
- C++17 compatible compiler
- Xcode / clang / gcc
- `nlohmann/json` (single-header, included)

### Run Locally

```bash
./engine input.json
```

In Xcode:
- Pass `input.json` as a launch argument
- Set the working directory to the project root

---

## Why Synthetic Data?

Synthetic markets are used intentionally to:
- avoid overfitting to historical data
- ensure reproducibility
- explore different market regimes
- keep the project educational and safe

The goal is to understand **strategy behavior**, not to optimize past performance.

---

## Current Scope (V1)

- Synthetic market generation
- Deterministic simulations
- JSON-based CLI engine

**Explicitly excluded:**
- real market data
- live trading
- strategy recommendations

---

## Planned Extensions

- Strategy execution layer
- Performance metrics (PnL, drawdown, win rate)
- Market regime comparison
- CSV-based historical data loader (offline)
- Strategy explainability tools

---

## Team

Built by a team of undergraduate engineers with interests in:
- systems programming
- quantitative finance
- simulation-based learning

---

## Disclaimer

This project is an **educational simulation platform**.  
It does **not** provide financial advice, investment recommendations, or real trading functionality.
