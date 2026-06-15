
## Зависимости

```bash
sudo apt update
sudo apt install build-essential libboost-all-dev python3-matplotlib
```

## Тесты

```bash
make test
```

## Бенчмарк

```bash
make run-benchmark
```

## Построение графика

```bash
make graph
```

```bash
./build/thread_pool_benchmark <число задач> <число потоков>
```
