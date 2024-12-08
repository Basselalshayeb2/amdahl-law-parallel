#include <mpi.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <iomanip>
#include <chrono>
#include <cstring>

using namespace std;

void generate_matrix(vector<vector<double>> &matrix, int n);
void matrix_multiply(const vector<vector<double>> &A, const vector<vector<double>> &B, vector<vector<double>> &result);
double scalar_product_v2(const vector<double> &x, const vector<double> &y, int start, int end);
void matrix_vector_multiply(const vector<vector<double>> &matrix, const vector<double> &vec, vector<double> &result, int start, int end);

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    
    int n = 1000;
    int seed = 42;
    if (argc > 1)
        n = atoi(argv[1]);

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--size") == 0 && i + 1 < argc)
        {
            n = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc)
        {
            seed = atoi(argv[++i]);
        }
    }

    // init seed
    srand(seed);

    vector<vector<double>> B, B2, B3, B4;
    vector<double> x(n), y(n);

    B.resize(n, vector<double>(n));
    B2.resize(n, vector<double>(n, 0.0));
    B3.resize(n, vector<double>(n, 0.0));
    B4.resize(n, vector<double>(n, 0.0));

    // init of matrix and vector
    if (rank == 0)
    {
        generate_matrix(B, n);
        matrix_multiply(B, B, B2);
        matrix_multiply(B2, B, B3);
        matrix_multiply(B3, B, B4);

        for (int i = 0; i < n; i++)
        {
            x[i] = static_cast<double>(rand()) / RAND_MAX * 10.0;
            y[i] = static_cast<double>(rand()) / RAND_MAX * 10.0;
        }
    }


    MPI_Bcast(x.data(), n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(y.data(), n, MPI_DOUBLE, 0, MPI_COMM_WORLD);


    for (int i = 0; i < n; i++)
    {
        MPI_Bcast(B3[i].data(), n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(B4[i].data(), n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

    double comp_start = MPI_Wtime();

    int rows_per_proc = n / size;
    int extra_rows = n % size;
    int start = rank * rows_per_proc + min(rank, extra_rows);
    int end = start + rows_per_proc + (rank < extra_rows ? 1 : 0);
    int local_rows = end - start;

    vector<double> local_B3x(local_rows, 0.0), local_B4x(local_rows, 0.0);
    matrix_vector_multiply(B3, x, local_B3x, start, end);
    matrix_vector_multiply(B4, x, local_B4x, start, end);
    
    double local_scalar_B3xy = scalar_product_v2(local_B3x, y, start, end); // two lines only there
    double local_scalar_B4xy = scalar_product_v2(local_B4x, y, start, end);


    double global_scalar_B3xy = 0.0, global_scalar_B4xy = 0.0;
    MPI_Reduce(&local_scalar_B3xy, &global_scalar_B3xy, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_scalar_B4xy, &global_scalar_B4xy, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double global_scalar_xy = scalar_product_v2(x, y, 0, n);
        double d = (global_scalar_B4xy / global_scalar_xy) - (global_scalar_B3xy / global_scalar_xy);

        double comp_end = MPI_Wtime();

        double total_time = (comp_end - comp_start) * 1000; // Convert to milliseconds

        cout << fixed << setprecision(2);
        cout << "Result (d): " << d << endl;
        cout << "Execution Time: " << total_time << endl;
        cout << "Global scalar <B3x, y>: " << global_scalar_B3xy << endl;
        cout << "Global scalar <B4x, y>: " << global_scalar_B4xy << endl;
        cout << "Global scalar <x, y>: " << global_scalar_xy << endl;
    }

    MPI_Finalize();
    return 0;
}

void generate_matrix(vector<vector<double>> &matrix, int n)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            matrix[i][j] = static_cast<double>(rand()) / RAND_MAX * 10.0;
        }
    }
}

void matrix_multiply(const vector<vector<double>> &A, const vector<vector<double>> &B, vector<vector<double>> &result)
{
    int n = A.size();
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            result[i][j] = 0.0;
            for (int k = 0; k < n; k++)
            {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

double scalar_product_v2(const vector<double> &local_section, const vector<double> &gloabal_y, int global_start, int global_end)
{
    double result = 0.0;
    for (int i = global_start; i < global_end; i++)
    {
        result += local_section[i - global_start] * gloabal_y[i];
    }
    return result;
}


void matrix_vector_multiply(const vector<vector<double>> &matrix, const vector<double> &vec, vector<double> &result, int start, int end)
{
    for (int i = start; i < end; i++)
    {
        result[i - start] = 0.0;
        for (int j = 0; j < vec.size(); j++)
        {
            result[i - start] += matrix[i][j] * vec[j];
        }
    }
}
