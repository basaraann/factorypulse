# FactoryPulse

[![agent](https://github.com/basaraann/factorypulse/actions/workflows/agent.yml/badge.svg)](https://github.com/basaraann/factorypulse/actions/workflows/agent.yml)

Industrial sensor monitoring system. A C++ agent simulates machines on a production line and streams their sensor readings to an ASP.NET Core API, which stores them in PostgreSQL and raises alarms.

> Work in progress: the C++ agent is done; the API, dashboard and Docker setup are next.

## Architecture

```
┌──────────────┐   HTTP POST /api/readings   ┌──────────────────┐      ┌────────────┐
│  C++ agent   │ ──────────────────────────▶ │ ASP.NET Core API │ ───▶ │ PostgreSQL │
│ (sensors)    │        JSON batches         │ (alarm rules)    │      └────────────┘
└──────────────┘                             └──────────────────┘
```

## C++ agent (`agent/`)

- C++17, CMake, GoogleTest
- Each machine has temperature, vibration and pressure sensors with noise and random faults
- Readings are buffered while the API is unreachable and sent when it comes back (oldest are dropped past a limit)
- Graceful shutdown on Ctrl+C / SIGTERM

### Build and test

```bash
cd agent
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build
```

Dependencies (nlohmann/json, cpp-httplib, GoogleTest) are used from the system if installed, otherwise downloaded by CMake.

### Run

```bash
./build/fp_agent --dry-run --machines 1                 # print readings to the console
./build/fp_agent --api http://localhost:8080 --machines 3 --interval-ms 1000
```

| Option | Default | Description |
|---|---|---|
| `--api URL` | `http://localhost:8080` | API base URL |
| `--machines N` | `3` | Number of simulated machines |
| `--interval-ms MS` | `1000` | Time between readings |
| `--fault-prob P` | `0.02` | Chance of a faulty reading (0–1) |
| `--dry-run` | off | Print to the console instead of sending |

## Roadmap

- [x] C++ sensor agent with unit tests
- [ ] ASP.NET Core Web API with EF Core + PostgreSQL
- [ ] Alarm rules (LINQ) and xUnit tests
- [ ] Live dashboard
- [ ] Docker Compose and GitHub Actions CI
