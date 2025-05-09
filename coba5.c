#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_YEARS 100
#define DEGREE 3

typedef struct {
    int year;
    double population;
    double internet_users;
} DataPoint;

// Fungsi untuk membaca data dari file CSV
int read_data(const char *filename, DataPoint data[]) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Tidak dapat membuka file %s\n", filename);
        exit(1);
    }

    char line[256];
    int count = 0;
    
    // Lewati header
    fgets(line, sizeof(line), file);
    
    while (fgets(line, sizeof(line), file) && count < MAX_YEARS) {
        sscanf(line, "%d,%lf,%lf", &data[count].year, &data[count].population, &data[count].internet_users);
        count++;
    }
    
    fclose(file);
    return count;
}

// Fungsi untuk interpolasi polinomial
void polynomial_interpolation(double x[], double y[], int n, double coeff[DEGREE+1]) {
    // Implementasi sederhana interpolasi polinomial (contoh: regresi polinomial)
    // Dalam implementasi nyata, Anda mungkin ingin menggunakan metode yang lebih robust
    
    // Matriks untuk menyimpan sistem persamaan
    double X[2*DEGREE+1] = {0};
    double B[DEGREE+1][DEGREE+2] = {0};
    
    // Hitung jumlah x^i untuk i=0 sampai 2*degree
    for (int i = 0; i <= 2*DEGREE; i++) {
        for (int j = 0; j < n; j++) {
            X[i] += pow(x[j], i);
        }
    }
    
    // Hitung jumlah y*x^i untuk i=0 sampai degree
    double Y[DEGREE+1] = {0};
    for (int i = 0; i <= DEGREE; i++) {
        for (int j = 0; j < n; j++) {
            Y[i] += y[j] * pow(x[j], i);
        }
    }
    
    // Bentuk matriks augmented
    for (int i = 0; i <= DEGREE; i++) {
        for (int j = 0; j <= DEGREE; j++) {
            B[i][j] = X[i+j];
        }
        B[i][DEGREE+1] = Y[i];
    }
    
    // Implementasi sederhana eliminasi Gauss-Jordan
    for (int i = 0; i <= DEGREE; i++) {
        // Normalisasi baris
        double pivot = B[i][i];
        if (pivot == 0) continue;
        
        for (int j = 0; j <= DEGREE+1; j++) {
            B[i][j] /= pivot;
        }
        
        // Eliminasi
        for (int k = 0; k <= DEGREE; k++) {
            if (k != i && B[k][i] != 0) {
                double factor = B[k][i];
                for (int j = 0; j <= DEGREE+1; j++) {
                    B[k][j] -= factor * B[i][j];
                }
            }
        }
    }
    
    // Ambil solusi
    for (int i = 0; i <= DEGREE; i++) {
        coeff[i] = B[i][DEGREE+1];
    }
}

// Fungsi untuk menghitung nilai y dari polinomial
double poly_value(double x, double coeff[DEGREE+1]) {
    double result = 0;
    for (int i = 0; i <= DEGREE; i++) {
        result += coeff[i] * pow(x, i);
    }
    return result;
}

// Fungsi untuk mencetak persamaan polinomial
void print_polynomial(double coeff[DEGREE+1], const char *name) {
    printf("Persamaan polinomial untuk %s:\n", name);
    printf("y = ");
    int first_term = 1;
    
    for (int i = DEGREE; i >= 0; i--) {
        if (coeff[i] != 0) {
            if (!first_term && coeff[i] > 0) {
                printf(" + ");
            } else if (coeff[i] < 0) {
                printf(" - ");
                coeff[i] = -coeff[i];
            }
            
            if (i == 0) {
                printf("%.4lf", coeff[i]);
            } else if (i == 1) {
                printf("%.4lfx", coeff[i]);
            } else {
                printf("%.4lfx^%d", coeff[i], i);
            }
            
            first_term = 0;
        }
    }
    printf("\n\n");
}

int main() {
    DataPoint data[MAX_YEARS];
    int n = read_data("Data Tugas Pemrograman A.csv", data);
    
    // Pisahkan data yang valid untuk populasi dan internet users
    double pop_x[MAX_YEARS], pop_y[MAX_YEARS];
    double int_x[MAX_YEARS], int_y[MAX_YEARS];
    int pop_count = 0, int_count = 0;
    
    for (int i = 0; i < n; i++) {
        if (data[i].population != 0) {
            pop_x[pop_count] = data[i].year;
            pop_y[pop_count] = data[i].population;
            pop_count++;
        }
        
        if (data[i].internet_users != 0) {
            int_x[int_count] = data[i].year;
            int_y[int_count] = data[i].internet_users;
            int_count++;
        }
    }
    
    // Hitung koefisien polinomial untuk populasi dan internet users
    double pop_coeff[DEGREE+1] = {0};
    double int_coeff[DEGREE+1] = {0};
    
    polynomial_interpolation(pop_x, pop_y, pop_count, pop_coeff);
    polynomial_interpolation(int_x, int_y, int_count, int_coeff);
    
    // Cetak persamaan polinomial
    print_polynomial(pop_coeff, "pertumbuhan populasi Indonesia");
    print_polynomial(int_coeff, "persentase pengguna Internet Indonesia");
    
    // Perkirakan nilai yang hilang
    int missing_years[] = {2005, 2006, 2015, 2016};
    printf("Perkiraan nilai yang hilang:\n");
    printf("Tahun | Populasi (jutaan) | Pengguna Internet (%%)\n");
    printf("-------------------------------------------------\n");
    
    for (int i = 0; i < 4; i++) {
        int year = missing_years[i];
        double pop = poly_value(year, pop_coeff);
        double int_use = poly_value(year, int_coeff);
        
        printf("%d | %.2f | %.2f%%\n", year, pop/1e6, int_use);
    }
    
    // Hitung estimasi untuk tahun 2030 dan 2035
    printf("\nEstimasi:\n");
    printf("Tahun 2030:\n");
    printf("  Populasi Indonesia: %.2f juta\n", poly_value(2030, pop_coeff)/1e6);
    
    printf("Tahun 2035:\n");
    printf("  Persentase pengguna Internet: %.2f%%\n", poly_value(2035, int_coeff));
    
    return 0;
}
