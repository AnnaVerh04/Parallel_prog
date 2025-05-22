import os
import re
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

folder_path = 'result_supercomp'

files = [
    '1_process.txt',
    '2_processes.txt',
    '4_processes.txt',
    '8_processes.txt',
    '16_processes.txt'
]

data = {}

pattern = re.compile(r'SIZE: (\d+)x\d+, Processes: (\d+), Time: ([\d.]+) seconds')

for file in files:
    file_path = os.path.join(folder_path, file)
    with open(file_path, 'r') as f:
        content = f.readlines()
    
    processes = None
    sizes = []
    times = []
    
    for line in content:
        match = pattern.match(line.strip())
        if match:
            size = int(match.group(1))
            processes = int(match.group(2))
            time = float(match.group(3))
            sizes.append(size)
            times.append(time)
    
    if processes is not None:
        data[processes] = {'size': sizes, 'time': times}

plt.figure(figsize=(10, 6))

for i, processes in enumerate(sorted(data.keys())):
    sizes = data[processes]['size']
    times = data[processes]['time']
    plt.plot(sizes, times, 
             marker='o',  
             color=plt.cm.tab10(i), 
             linestyle='-',
             linewidth=2,
             markersize=8,
             label=f'{processes} потоков')

plt.xlabel('Размер матрицы (N x N)', fontsize=12)
plt.ylabel('Время выполнения (секунды)', fontsize=12)
plt.title('Зависимость времени выполнения от размера матрицы', fontsize=14, pad=20)

plt.xticks(np.arange(0, 2100, 250))  

max_time = max([max(data[p]['time']) for p in data])
yticks = np.arange(0, max_time * 1.1, max_time/10)  
plt.yticks(yticks)

plt.grid(True, which='both', linestyle='--', alpha=0.7)
plt.legend(fontsize=10, title='Количество потоков:', title_fontsize=11)

plt.tight_layout()
plt.savefig('supercomp_time_comparison.png', dpi=300, bbox_inches='tight')
plt.show()