#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>
#include <fstream>

using namespace std;

void read_matrix(const string &filename, vector<vector<double>> &matrix, int n);
void read_vector(const string &filename, vector<double> &vec, int n);
void generate_matrix(vector<vector<double>> &matrix, int n);
void generate_vector(vector<double> &vec, int n);
double scalar_product(const vector<double> &x, const vector<double> &y, int n);
void matrix_multiply(const vector<vector<double>> &A, const vector<vector<double>> &B, vector<vector<double>> &C, int n);

int main(int argc, char *argv[]) {
    int n = 1000;
    int i, j;

    vector<vector<double>> B(n, vector<double>(n));
    vector<vector<double>> B3(n, vector<double>(n));
    vector<vector<double>> B4(n, vector<double>(n));
    vector<double> x(n);
    vector<double> y(n);

    bool use_file = (argc > 1 && string(argv[1]) == "--use-files");

    if (use_file) {
        cout << "Reading data from files..." << endl;
        read_matrix("matrix_B.txt", B, n);
        read_vector("vector_x.txt", x, n);
        read_vector("vector_y.txt", y, n);
    } else {
        cout << "Generating random data..." << endl;
        srand(time(NULL));
        generate_matrix(B, n);
        generate_vector(x, n);
        generate_vector(y, n);
    }

    double start_time = omp_get_wtime();

    vector<vector<double>> B2(n, vector<double>(n));
    matrix_multiply(B, B, B2, n);
    matrix_multiply(B, B2, B3, n);
    matrix_multiply(B2, B2, B4, n);

    double scalar_xy, scalar_B4xy, scalar_B3xy;

    scalar_xy = scalar_product(x, y, n);

    // Compute B^4 * x and B^3 * x
    vector<double> B4x(n, 0.0), B3x(n, 0.0);
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            B4x[i] += B4[i][j] * x[j];
            B3x[i] += B3[i][j] * x[j];
        }
    }
    #pragma omp parallel sections
    {
        #pragma omp section
        scalar_B4xy = scalar_product(B4x, y, n);

        #pragma omp section
        scalar_B3xy = scalar_product(B3x, y, n);
    }

    double d = (scalar_B4xy / scalar_xy) - (scalar_B3xy / scalar_xy);

    double end_time = omp_get_wtime();

    cout << "Result (d): " << d << endl;
    cout << "Execution Time: " << (end_time - start_time) << endl;

    return 0;
}

// Function to generate a random matrix
void generate_matrix(vector<vector<double>> &matrix, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            matrix[i][j] = (double)rand() / RAND_MAX * 10.0;
}

void generate_vector(vector<double> &vec, int n) {
    for (int i = 0; i < n; i++)
        vec[i] = (double)rand() / RAND_MAX * 10.0;
}

double scalar_product(const vector<double> &x, const vector<double> &y, int n) {
    double result = 0.0;

    #pragma omp parallel for reduction(+:result)
    for (int i = 0; i < n; i++)
        result += x[i] * y[i];

    return result;
}

void matrix_multiply(const vector<vector<double>> &A, const vector<vector<double>> &B, vector<vector<double>> &C, int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            C[i][j] = 0.0;
            for (int k = 0; k < n; k++)
                C[i][j] += A[i][k] * B[k][j];
        }
}

void read_matrix(const string &filename, vector<vector<double>> &matrix, int n) {
    ifstream file(filename);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            file >> matrix[i][j];
    file.close();
}

void read_vector(const string &filename, vector<double> &vec, int n) {
    ifstream file(filename);
    for (int i = 0; i < n; ++i)
        file >> vec[i];
    file.close();
}