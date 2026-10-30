# FactoryPulse

[![agent](https://github.com/basaraann/factorypulse/actions/workflows/agent.yml/badge.svg)](https://github.com/basaraann/factorypulse/actions/workflows/agent.yml)
[![api](https://github.com/basaraann/factorypulse/actions/workflows/api.yml/badge.svg)](https://github.com/basaraann/factorypulse/actions/workflows/api.yml)

Industrial sensor monitoring system. A C++ agent simulates machines on a production line and streams their sensor readings to an ASP.NET Core API, which stores them in PostgreSQL and raises alarms.

> Work in progress: the C++ agent and the API (with PostgreSQL storage) are done; alarms, the dashboard and Docker setup are next.

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

## API (`api/`)

- ASP.NET Core minimal API on .NET 10
- `POST /api/readings` accepts a batch of readings from an agent (up to 10,000 per request)
- The whole batch is rejected with a `400 ValidationProblem` if any reading is invalid, listing the problems per reading
- Readings are stored in PostgreSQL with EF Core; each batch is saved in one transaction
- Database schema is managed with EF Core migrations, applied automatically on startup in Development
- xUnit tests: unit tests for the validator, endpoint tests with `WebApplicationFactory`, and PostgreSQL tests against a real database started by Testcontainers

### Build, test and run

Start PostgreSQL (needs Docker):

```bash
docker run -d --name factorypulse-db -p 5432:5432 \
  -e POSTGRES_USER=factorypulse -e POSTGRES_PASSWORD=factorypulse -e POSTGRES_DB=factorypulse \
  postgres:17-alpine
```

Then:

```bash
cd api
dotnet test                                                        # PostgreSQL tests need Docker running
dotnet run --project src/FactoryPulse.Api --launch-profile http   # listens on http://localhost:8080
```

The connection string is in `appsettings.json` (`ConnectionStrings:FactoryPulse`) and can be overridden with the `ConnectionStrings__FactoryPulse` environment variable.

With the API running, start the agent without `--dry-run` and it will stream readings to it.

| Endpoint | Description |
|---|---|
| `POST /api/readings` | Store a batch of readings, returns `{"accepted": n}` |
| `GET /health` | Liveness check |

## Roadmap

- [x] C++ sensor agent with unit tests
- [x] API endpoint that receives and validates readings
- [x] PostgreSQL storage with EF Core
- [ ] Alarm rules (LINQ)
- [ ] Live dashboard
- [ ] Docker Compose
- [x] GitHub Actions CI
