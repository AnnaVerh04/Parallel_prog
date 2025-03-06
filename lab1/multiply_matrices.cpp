#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <filesystem>
#include <random>
#include <string>

using namespace std;

vector<vector<int>> generate_random_matrix(int rows, int cols) {
    vector<vector<int>> matrix(rows, vector<int>(cols));
    random_device rd; 
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 100); 

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = dis(gen);
        }
    }
    return matrix;
}

void write_matrix_to_file(const string& filename, const vector<vector<int>>& matrix) {
    ofstream file(filename);
    int rows = matrix.size();
    int cols = matrix[0].size();

    file << rows << " " << cols << endl;
    for (const auto& row : matrix) {
        for (int val : row) {
            file << val << " ";
        }
        file << endl;
    }
}

vector<vector<int>> multiply_matrices(const vector<vector<int>>& A, const vector<vector<int>>& B) {
    int rowsA = A.size();
    int colsA = A[0].size();
    int colsB = B[0].size();
    vector<vector<int>> C(rowsA, vector<int>(colsB, 0));

    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            for (int k = 0; k < colsA; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return C;
}

int main() {

    vector<int> sizes = {100, 200, 300, 400, 500, 1000, 1500, 2000};
    
    ofstream time_output("results/time_results.txt");
    time_output << "Размер матриц\tВремя выполнения (секунд)\n";

    for (int size : sizes) {
        auto A = generate_random_matrix(size, size); 
        auto B = generate_random_matrix(size, size); 

        string matrixA_file = "matrices/matrixA_" + to_string(size) + ".txt";
        string matrixB_file = "matrices/matrixB_" + to_string(size) + ".txt";
        write_matrix_to_file(matrixA_file, A);
        write_matrix_to_file(matrixB_file, B);

        auto start = chrono::high_resolution_clock::now();
        vector<vector<int>> C = multiply_matrices(A, B);
        auto end = chrono::high_resolution_clock::now();

        string resultFile = "results/result_" + to_string(size) + ".txt";
        write_matrix_to_file(resultFile, C);

        chrono::duration<double> duration = end - start;
        time_output << size << "x" << size << "\t" << duration.count() << endl;
    }

    time_output.close(); 
    cout << "Done task" << endl;

    return 0;
}
