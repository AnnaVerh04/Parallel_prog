import os
import numpy as np
import matplotlib.pyplot as plt

matrix_dir = "matrices_mpi"
result_dir = "results_mpi"
verification_file = "results_mpi/verification_results_mpi.txt"

def read_matrix_from_file(filename):
    with open(filename, "r") as file:
        rows, cols = map(int, file.readline().split())
        matrix = np.zeros((rows, cols), dtype=int)
        for i in range(rows):
            matrix[i] = [int(x) for x in file.readline().split()]
    return matrix

def check_results():
    with open(verification_file, "w", encoding="utf-8") as f:
        f.write("Размер матриц\tРезультат проверки\n")
        for size in [100, 200, 300, 400, 500, 1000, 1500, 2000]:
            matrix_a_file = os.path.join(matrix_dir, f"matrixA_{size}.txt")
            matrix_b_file = os.path.join(matrix_dir, f"matrixB_{size}.txt")
            result_file = os.path.join(result_dir, f"result_mpi_{size}.txt")

            matrix_a = read_matrix_from_file(matrix_a_file)
            matrix_b = read_matrix_from_file(matrix_b_file)
            result = read_matrix_from_file(result_file)

            expected_result = np.matmul(matrix_a, matrix_b)

            if np.array_equal(result, expected_result):
                status = "Корректно"
            else:
                status = "Некорректно"
            f.write(f"{size}x{size}\t{status}\n")
            print(f"Результат для матриц размера {size}x{size} - {status}")

matrix_sizes_base = []
execution_times_base = []
with open("C:/Users/Professional/Desktop/parallel_prog/Parallel_prog/lab1/results/time_results.txt", "r") as file:
    next(file)
    for line in file:
        size, time = line.strip().split("\t")
        if "x" in size:
            rows, cols = map(int, size.split("x"))
            matrix_sizes_base.append(rows)
        else:
            matrix_sizes_base.append(int(size))
        execution_times_base.append(float(time))

matrix_sizes_mpi = []
execution_times_mpi = []
with open("C:/Users/Professional/Desktop/parallel_prog/Parallel_prog/lab3/results_mpi/time_results_mpi.txt", "r") as file:
    next(file)
    for line in file:
        size, time = line.strip().split("\t")
        if "x" in size:
            rows, cols = map(int, size.split("x"))
            matrix_sizes_mpi.append(rows)
        else:
            matrix_sizes_mpi.append(int(size))
        execution_times_mpi.append(float(time))


plt.figure(figsize=(10, 6))
plt.plot(matrix_sizes_base, execution_times_base, color='blue', marker='o', label='Без использования MPI')
plt.plot(matrix_sizes_mpi, execution_times_mpi, color='red', marker='o', label='С использованием MPI')
plt.xlabel("Размер матриц")
plt.ylabel("Время выполнения (с)")
plt.title("Зависимость времени выполнения от размера матриц")
plt.grid()
plt.legend()
plt.savefig("results_mpi/timing_graph_mpi.png")

if __name__ == "__main__":
    check_results()