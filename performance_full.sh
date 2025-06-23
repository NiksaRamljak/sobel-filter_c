#!/bin/bash

mkdir -p performance_logs
mkdir -p output

for img in ./test_jpeg/*.{jpg,jpeg}; do
    img_name=$(basename "$img" .${img##*.})
    log_file="./performance_logs/statsfor${img_name}.txt"
    perf stat -o "$log_file" ./build/fltr_sobel_test "$img" "./output/${img_name}.jpg"
    echo "Processed $img, stats saved to $log_file"
done
