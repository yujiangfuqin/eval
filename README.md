# Multi-Party Computation Evaluation Framework

A secure multi-party computation framework implementing comparison protocols, conditional oblivious transfer (COT), and decision tree evaluation.

## Build

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

Run three-party protocols using the provided script:

```bash
./run_party.sh <executable_name>
```

Available executables:
- `eval` - Decision tree evaluation
- `test_cmp` - Comparison protocol testing
- `test_cot` - Conditional oblivious transfer testing

Example:
```bash
./run_party.sh test_cmp
```

This will start three processes (player0, player1, player2) simultaneously and save outputs to `player*.log` files. Process IDs are saved to `pids.log` for cleanup if needed.

## Configuration

Each player requires a configuration file (`player0_config.JSON`, `player1_config.JSON`, `player2_config.JSON`) containing network settings and cryptographic keys.