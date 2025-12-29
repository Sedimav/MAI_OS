SIZE=100000
K_VALUES=(1 2 4 6 8 16 128 1024)

echo "K,Time_ms"
for K in "${K_VALUES[@]}"; do
    TIME=$(./quicksort $SIZE $K)
    echo "$K,$TIME"
done
