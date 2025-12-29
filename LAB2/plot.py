import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('results.csv')
K = df['K'].values
time_ms = df['Time_ms'].values

T1 = time_ms[0]
speedup = T1 / time_ms
efficiency = speedup / K

plt.style.use('seaborn-v0_8')
plt.figure(figsize=(14, 5))

plt.subplot(1, 2, 1)
plt.plot(K, speedup, 'bo-', linewidth=2, markersize=8)
plt.xscale('log', base=2)
plt.xlabel('Число потоков (K)')
plt.ylabel('Ускорение S(K)')
plt.title('Ускорение vs Число потоков')
plt.grid(True, which="both", ls="-", alpha=0.5)
for i, txt in enumerate(speedup):
    plt.annotate(f"{txt:.2f}", (K[i], speedup[i]), textcoords="offset points", xytext=(0,10), ha='center')

plt.subplot(1, 2, 2)
plt.plot(K, efficiency, 'ro-', linewidth=2, markersize=8)
plt.xscale('log', base=2)
plt.xlabel('Число потоков (K)')
plt.ylabel('Эффективность E(K)')
plt.title('Эффективность vs Число потоков')
plt.grid(True, which="both", ls="-", alpha=0.5)
for i, txt in enumerate(efficiency):
    plt.annotate(f"{txt:.2f}", (K[i], efficiency[i]), textcoords="offset points", xytext=(0,10), ha='center')

plt.tight_layout()
plt.savefig('quicksort_performance.png', dpi=300, bbox_inches='tight')
plt.show()
