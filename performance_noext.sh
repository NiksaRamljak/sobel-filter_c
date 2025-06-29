#!/bin/bash

mkdir -p performance_logs
mkdir -p output

for img in ./test_pgm/*.pgm; do
    img_name=$(basename "$img" .pgm)
    log_file="./performance_logs/statsfor${img_name}-pure.txt"
    perf stat -r 5 -o "$log_file" ./build/fltr_sobel "$img" "./output/${img_name}-pure.pgm"
    echo "Processed $img, stats saved to $log_file"
done
for img in ./test_pgm/*.pgm; do
    img_name=$(basename "$img" .pgm)
    log_file="./performance_logs/statsfor${img_name}-pure-one.txt"
    perf stat -r 5 -o "$log_file" ./build/fltr_sobel "$img" "./output/${img_name}-pure-one.pgm" -t 1
    echo "Processed $img, stats saved to $log_file"
done
