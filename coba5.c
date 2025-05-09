#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAX_YEARS 100
#define DEGREE 3
#define FILENAME "Data Tugas Pemrograman A.csv"

typedef struct {
    int year;
    double percentage; // Persentase pengguna internet
    double population; // Jumlah populasi (dalam juta)
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
    int line_num = 0;
    
    while (fgets(line, sizeof(line), file) && count < MAX_YEARS) {
        line_num++;
        
        // Skip header
        if (line_num == 1) continue;
        
        // Hilangkan newline character
        line[strcspn(line, "\n")] = 0;
        
        // Parsing line
        char *token = strtok(line, ",");
        if (token == NULL) continue;
        
        data[count].year = atoi(token);
        
        token = strtok(NULL, ",");
        data[count].percentage = (token != NULL && strlen(token) > 0) ? atof(token) : NAN;
        
        token = strtok(NULL, ",");
        data[count].population = (token != NULL && strlen(token) > 0) ? atof(token) : NAN;
        
        count++;
    }
    
    fclose(file);
    return count;
}

// Fungsi untuk interpolasi polinomial
void polynomial_interpolation(double x[], double y[], int n, double coeff[DEGREE+1]) {
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
    
    // Eliminasi Gauss-Jordan
    for (int i = 0; i <= DEGREE; i++) {
        // Pivoting partial
        int max_row = i;
        for (int k = i+1; k <= DEGREE; k++) {
            if (fabs(B[k][i]) > fabs(B[max_row][i])) {
                max_row = k;
            }
        }
        
        // Tukar baris jika diperlukan
        if (max_row != i) {
            for (int k = 0; k <= DEGREE+1; k++) {
                double temp = B[i][k];
                B[i][k] = B[max_row][k];
                B[max_row][k] = temp;
            }
        }
        
        // Normalisasi baris
        double pivot = B[i][i];
        if (pivot == 0) {
            printf("Peringatan: Matriks singular terdeteksi\n");
            continue;
        }
        
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

double poly_value(double x, double coeff[DEGREE+1]) {
    double result = 0;
    for (int i = 0; i <= DEGREE; i++) {
        result += coeff[i] * pow(x, i);
    }
    return result;
}

void print_polynomial(double coeff[DEGREE+1], const char *name) {
    printf("Persamaan polinomial untuk %s:\ny = ", name);
    
    int first_term = 1;
    for (int i = DEGREE; i >= 0; i--) {
        if (fabs(coeff[i]) > 1e-6) {  // Abaikan koefisien yang sangat kecil
            if (!first_term) {
                printf(" %c ", (coeff[i] >= 0) ? '+' : '-');
            } else if (coeff[i] < 0) {
                printf("-");
            }
            
            double abs_coeff = fabs(coeff[i]);
            
            if (i == 0) {
                printf("%.4f", abs_coeff);
            } else {
                if (abs_coeff != 1.0) {
                    printf("%.4f", abs_coeff);
                }
                printf("x");
                if (i > 1) {
                    printf("^%d", i);
                }
            }
            first_term = 0;
        }
    }
    printf("\n\n");
}

int main() {
    DataPoint data[MAX_YEARS];
    int n = read_data(FILENAME, data);
    
    printf("Data berhasil dibaca (%d records)\n\n", n);
    
    // Pisahkan data yang valid
    double pop_x[MAX_YEARS], pop_y[MAX_YEARS];
    double perc_x[MAX_YEARS], perc_y[MAX_YEARS];
    int pop_count = 0, perc_count = 0;
    
    for (int i = 0; i < n; i++) {
        if (!isnan(data[i].population)) {
            pop_x[pop_count] = data[i].year;
            pop_y[pop_count] = data[i].population;
            pop_count++;
        }
        
        if (!isnan(data[i].percentage)) {
            perc_x[perc_count] = data[i].year;
            perc_y[perc_count] = data[i].percentage;
            perc_count++;
        }
    }
    
    // Hitung koefisien polinomial
    double pop_coeff[DEGREE+1] = {0};
    double perc_coeff[DEGREE+1] = {0};
    
    polynomial_interpolation(pop_x, pop_y, pop_count, pop_coeff);
    polynomial_interpolation(perc_x, perc_y, perc_count, perc_coeff);
    
    // 1. Cetak persamaan polinomial
    printf("=== HASIL ANALISIS ===\n\n");
    print_polynomial(pop_coeff, "Pertumbuhan Populasi Indonesia");
    print_polynomial(perc_coeff, "Persentase Pengguna Internet Indonesia");
    
    // 2. Perkirakan nilai yang hilang
    int missing_years[] = {2005, 2006, 2015, 2016};
    printf("=== PERKIRAAN NILAI YANG HILANG ===\n");
    printf("+-------+------------------+---------------------------+\n");
    printf("| Tahun | Populasi (juta)  | Pengguna Internet (%%)     |\n");
    printf("+-------+------------------+---------------------------+\n");
    
    for (int i = 0; i < 4; i++) {
        int year = missing_years[i];
        double pop = poly_value(year, pop_coeff);
        double perc = poly_value(year, perc_coeff);
        
        printf("| %4d  | %16.2f | %25.2f |\n", year, pop, perc);
    }
    printf("+-------+------------------+---------------------------+\n\n");
    
    // 3. Hitung estimasi untuk tahun 2030 dan 2035
    printf("=== ESTIMASI MASA DEPAN ===\n");
    printf("1. Populasi Indonesia tahun 2030: %.2f juta\n", 
           poly_value(2030, pop_coeff));
    printf("2. Persentase pengguna Internet tahun 2035: %.2f%%\n", 
           poly_value(2035, perc_coeff));
    
    return 0;
}
