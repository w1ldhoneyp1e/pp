## Зависимости

```bash
sudo apt update
sudo apt install g++ make pkg-config libpng-dev python3-matplotlib
```

## Запуск с PNG

```bash
make run IMAGE=images/photo.png REPETITIONS=5 THREADS=1,2,4,8,16
```

Или напрямую:

```bash
./build/histogram_benchmark \
    --image images/photo.png \
    --repetitions 5 \
    --threads 1,2,4,8,16 \
    --output results/benchmark.csv
```

## Запуск на синтетическом изображении

```bash
make synthetic SYNTHETIC_SIZE=8000x8000 PATTERN=random
```

Доступные паттерны:

- `random` — значения каналов распределены случайно
- `solid` — все пиксели одного цвета
- `gradient` — повторяющийся градиент

## Тестирование

```bash
make test
```

## Графики

После получения `results/benchmark.csv`:

```bash
make graph
```

Будут созданы:

- `results/time_vs_threads.png`;
- `results/speedup_vs_threads.png`.

