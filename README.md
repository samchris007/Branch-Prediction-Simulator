# Branch Prediction Simulator

## Overview

This repo contains implementation which involves building a **branch prediction simulator** for evaluating different configurations of branch predictors. The simulator supports bimodal, gshare, and hybrid branch predictors.

## Features

- **Bimodal Predictor**: Simulates a simple bimodal branch predictor using the program counter (PC) bits as an index.
- **Gshare Predictor**: Simulates a gshare branch predictor that combines PC bits with a global history register.
- **Hybrid Predictor** (ECE 563 only): Simulates a hybrid branch predictor that selects between bimodal and gshare predictions using a chooser table.

## Inputs and Outputs

### Input Trace File
- Contains a list of branch instructions in memory.
- Format: `<hex branch PC> t|n`
  - `t`: branch actually taken.
  - `n`: branch actually not taken.

### Command-Line Arguments
1. **Bimodal Predictor**: `sim bimodal <M2> <tracefile>`
   - `<M2>`: Number of PC bits for the bimodal table index.
2. **Gshare Predictor**: `sim gshare <M1> <N> <tracefile>`
   - `<M1>`: Number of PC bits for the gshare table index.
   - `<N>`: Number of bits in the global history register.
3. **Hybrid Predictor**: `sim hybrid <K> <M1> <N> <M2> <tracefile>`
   - `<K>`: Number of PC bits for the chooser table index.
   - `<M1>` and `<N>`: Parameters for the gshare predictor.
   - `<M2>`: Parameter for the bimodal predictor.

### Output
1. Simulator command configuration.
2. Number of predictions and mispredictions.
3. Misprediction rate.
4. Final state of the predictor tables.

### Prerequisites
- **Language**: C++
- **Build Tool**: `Makefile`
