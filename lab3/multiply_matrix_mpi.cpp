#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <filesystem>
#include <random>
#include <string>
#include <mpi.h>

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
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }
    
    int rows = matrix.size();
    if (rows == 0) {
        file << "0 0" << endl;
        return;
    }
    int cols = matrix[0].size();

    file << rows << " " << cols << endl;
    for (const auto& row : matrix) {
        for (int val : row) {
            file << val << " ";
        }
        file << endl;
    }
}

vector<vector<int>> multiply_matrices_part(const vector<vector<int>>& A, const vector<vector<int>>& B, 
                                        int start_row, int end_row, int matrix_size) {
    vector<vector<int>> C_part(end_row - start_row, vector<int>(matrix_size, 0));

    for (int i = 0; i < end_row - start_row; ++i) {
        for (int j = 0; j < matrix_size; ++j) {
            for (int k = 0; k < matrix_size; ++k) {
                C_part[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return C_part;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        cout << "Starting MPI matrix multiplication with " << size << " processes" << endl;
    }

    vector<int> sizes = {100, 200, 300, 400, 500, 1000, 1500, 2000};
    
    ofstream time_output;
    if (rank == 0) {
        filesystem::create_directory("results_mpi");
        filesystem::create_directory("matrices_mpi");
        time_output.open("results_mpi/time_results_mpi.txt");
        time_output << "Matrix Size\tTime (seconds)\n";
    }

    for (int matrix_size : sizes) {
        vector<vector<int>> A, B;
        double start_time = 0.0, end_time = 0.0;

        if (rank == 0) {
            cout << "Generating matrices size " << matrix_size << "x" << matrix_size << endl;
            A = generate_random_matrix(matrix_size, matrix_size);
            B = generate_random_matrix(matrix_size, matrix_size);
            
            string matrixA_file = "matrices_mpi/matrixA_" + to_string(matrix_size) + ".txt";
            string matrixB_file = "matrices_mpi/matrixB_" + to_string(matrix_size) + ".txt";
            write_matrix_to_file(matrixA_file, A);
            write_matrix_to_file(matrixB_file, B);
        }

        MPI_Bcast(&matrix_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

        vector<int> B_flat(matrix_size * matrix_size);

        if (rank == 0) {
            for (int i = 0; i < matrix_size; ++i)
                for (int j = 0; j < matrix_size; ++j)
                    B_flat[i * matrix_size + j] = B[i][j];
        }

        MPI_Bcast(B_flat.data(), matrix_size * matrix_size, MPI_INT, 0, MPI_COMM_WORLD);

        B.resize(matrix_size, vector<int>(matrix_size));
        for (int i = 0; i < matrix_size; ++i)
            for (int j = 0; j < matrix_size; ++j)
                B[i][j] = B_flat[i * matrix_size + j];

        int rows_per_process = matrix_size / size;
        int remainder = matrix_size % size;
        
        int start_row = rank * rows_per_process + min(rank, remainder);
        int end_row = start_row + rows_per_process + (rank < remainder ? 1 : 0);
        
        vector<vector<int>> local_A;
        if (rank == 0) {
            local_A = vector<vector<int>>(A.begin() + start_row, A.begin() + end_row);
            
            for (int dest = 1; dest < size; ++dest) {
                int dest_start = dest * rows_per_process + min(dest, remainder);
                int dest_end = dest_start + rows_per_process + (dest < remainder ? 1 : 0);
                
                vector<int> send_buf;
                for (int row = dest_start; row < dest_end; ++row) {
                    send_buf.insert(send_buf.end(), A[row].begin(), A[row].end());
                }
                
                MPI_Send(send_buf.data(), send_buf.size(), MPI_INT, dest, 0, MPI_COMM_WORLD);
            }
        } else {
            int recv_size = (end_row - start_row) * matrix_size;
            vector<int> recv_buf(recv_size);
            
            MPI_Recv(recv_buf.data(), recv_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            local_A.resize(end_row - start_row, vector<int>(matrix_size));
            for (int i = 0; i < end_row - start_row; ++i) {
                copy(recv_buf.begin() + i * matrix_size, 
                     recv_buf.begin() + (i + 1) * matrix_size,
                     local_A[i].begin());
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == 0) {
            start_time = MPI_Wtime();
        }

        vector<vector<int>> local_C = multiply_matrices_part(local_A, B, 0, end_row - start_row, matrix_size);

        vector<vector<int>> C;
        if (rank == 0) {
            C.resize(matrix_size, vector<int>(matrix_size));
            
            for (int i = 0; i < end_row - start_row; ++i) {
                for (int j = 0; j < matrix_size; ++j) {
                    C[start_row + i][j] = local_C[i][j];
                }
            }
            
            for (int src = 1; src < size; ++src) {
                int src_start = src * rows_per_process + min(src, remainder);
                int src_end = src_start + rows_per_process + (src < remainder ? 1 : 0);
                int src_rows = src_end - src_start;
                
                vector<int> recv_buf(src_rows * matrix_size);
                MPI_Recv(recv_buf.data(), src_rows * matrix_size, MPI_INT, 
                         src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                
                for (int i = 0; i < src_rows; ++i) {
                    for (int j = 0; j < matrix_size; ++j) {
                        C[src_start + i][j] = recv_buf[i * matrix_size + j];
                    }
                }
            }
            
            end_time = MPI_Wtime();
        } else {
            vector<int> send_buf;
            for (const auto& row : local_C) {
                send_buf.insert(send_buf.end(), row.begin(), row.end());
            }
            
            MPI_Send(send_buf.data(), send_buf.size(), MPI_INT, 0, 0, MPI_COMM_WORLD);
        }

        if (rank == 0) {
            string resultFile = "results_mpi/result_mpi_" + to_string(matrix_size) + ".txt";
            write_matrix_to_file(resultFile, C);
            
            double elapsed = end_time - start_time;
            time_output << matrix_size << "x" << matrix_size << "\t" << elapsed << endl;
            cout << "Matrix " << matrix_size << "x" << matrix_size 
                 << " done in " << elapsed << " seconds" << endl;
        }
    }

    if (rank == 0) {
        time_output.close();
        cout << "All computations completed" << endl;
    }

    MPI_Finalize();
    return 0;
}