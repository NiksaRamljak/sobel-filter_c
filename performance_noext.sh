#!/bin/bash

mkdir -p performance_logs
mkdir -p output

for img in ./test_pgm/*.pgm; do
    img_name=$(basename "$img" .pgm)
    log_file="./performance_logs/statsfor${img_name}-pure.txt"
    perf stat -o "$log_file" ./build/fltr_sobel "$img" "./output/${img_name}.pgm"
    echo "Processed $img, stats saved to $log_file"
done
