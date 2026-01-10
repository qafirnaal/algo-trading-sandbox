# AXIOM
## An Educational Algorithmic Trading Simulator

AXIOM is a simulation-first, educational platform designed to help users understand how algorithmic trading strategies behave under different market conditions — without real money, live markets, or black-box predictions.

The project focuses on learning and explainability, not profit or execution.

AXIOM is built as a modular system with a high-performance C++ core, a lightweight API layer, and a modern frontend for visualization.

Disclaimer:  
AXIOM is strictly for educational purposes.  
It does not provide financial advice and does not execute real trades.

---

## Project Overview

Learning algorithmic trading today often requires advanced mathematics, heavy coding, or experimentation with real financial risk.

AXIOM addresses this gap by providing:
- synthetic market simulations
- deterministic and reproducible results
- parameterized, explainable strategies
- clear performance metrics

The goal is to help users understand why strategies behave the way they do, not to predict markets.

---

## Architecture

AXIOM follows a clean, layered architecture:

Frontend (React)  
↓  
Backend (FastAPI)  
↓  
C++ Simulation Engine  

- The C++ engine performs all simulation, strategy execution, and metric computation.
- FastAPI acts as a thin orchestration layer.
- The frontend focuses on interaction and visualization.

Each layer is independent and replaceable.

---

## Repository Structure

.
├── backend/
│   ├── Engine/
│   │   ├── include/              C++ headers
│   │   ├── source/               C++ source files
│   │   ├── engine                Compiled C++ binary
│   │   ├── input.json            Local test input
│   │   └── json/                 nlohmann/json library
│   │
│   ├── app/                      FastAPI application
│   ├── input.json                Backend test input
│   ├── mockOutput.json
│   └── output.json
│
├── frontend/
│   ├── public/
│   ├── src/
│   ├── index.html
│   └── vite.config.js
│
└── README.md

---

## Core Features (Current State)

Simulation Engine:
- Synthetic market generation (trending, sideways)
- Deterministic simulations using seeded randomness
- Moving Average Crossover strategy
- Trade generation (BUY / SELL)
- Performance metrics:
  - Total PnL
  - Max drawdown
  - Win rate
  - Trade count

Strategy Design:
- Parameter-based configuration
- No strategy scripting
- No black-box models
- Fully explainable logic

---

## Example Input Format

The engine accepts a JSON configuration file.

{
  "market": "trending",
  "timesteps": 1000,
  "seed": 42,
  "strategy": {
    "type": "ma_crossover",
    "short_window": 20,
    "long_window": 50,
    "position_size": 1
  }
}

---

## How to Run the Project

### 1. Running the C++ Engine (Standalone)

Option A: Using Xcode

1. Open the Xcode project inside backend/Engine
2. Select the executable target
3. In Run Scheme → Arguments, add:
   input.json
4. Set the Working Directory to:
   backend/Engine
5. Run the project

The engine will print JSON output to the console.

Option B: Using Terminal

cd backend/Engine  
./engine input.json

---

### 2. Running the FastAPI Backend

From the repository root:

cd backend

Create and activate a virtual environment:

python3 -m venv venv  
source venv/bin/activate

Install dependencies:

pip install fastapi uvicorn

Run the backend:

uvicorn app.main:app --reload

The API will be available at:

http://127.0.0.1:8000

Swagger documentation:

http://127.0.0.1:8000/docs

---

### 3. Running the Frontend

cd frontend  
npm install  
npm run dev

Open the URL shown in the terminal (usually http://localhost:5173).

---

## LearnSphere Alignment

AXIOM aligns with the LearnSphere track by focusing on:
- conceptual understanding over outcomes
- safe experimentation
- explainability of algorithmic decision-making
- learning without financial risk

---

## Planned On-Site Extensions

The following features are designed as extensions of the existing engine and are intended to be implemented during the on-site hackathon phase:

- Strategy comparison across different market regimes
- Sensitivity analysis of strategy parameters
- Visual trade annotations on charts
- Educational explainers for metrics and signals
- Additional simple strategies (momentum, mean reversion)

These build directly on the current architecture.

---

## Team

AXIOM is built by a team of undergraduate engineers with interests in:
- systems programming
- quantitative finance
- simulation-based learning

---

## Disclaimer

AXIOM is an educational simulation platform.  
It does not provide financial advice, investment recommendations, or real trading functionality.
