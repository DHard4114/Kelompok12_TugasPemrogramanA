#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    int year;
    double percentage;
    long long population;
    int is_interpolated;
} DataPoint;

DataPoint* read_csv(const char* filename, int* count) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(1);
    }

    char line[1024];
    fgets(line, sizeof(line), file);

    DataPoint* data = malloc(sizeof(DataPoint) * 100);
    int capacity = 100;
    int n = 0;

    while (fgets(line, sizeof(line), file)) {
        int year;
        double percentage;
        long long population;

        if (sscanf(line, "%d,%lf,%lld", &year, &percentage, &population) == 3) {
            if (n >= capacity) {
                capacity *= 2;
                data = realloc(data, sizeof(DataPoint) * capacity);
            }
            data[n].year = year;
            data[n].percentage = percentage;
            data[n].population = population;
            data[n].is_interpolated = 0;
            n++;
        }
    }

    fclose(file);
    *count = n;
    return data;
}

int compare_years(const void* a, const void* b) {
    DataPoint* da = (DataPoint*)a;
    DataPoint* db = (DataPoint*)b;
    return da->year - db->year;
}

typedef struct {
    double a, b, c, d;
} CubicCoefficients;

CubicCoefficients fit_cubic(int* years, double* y_values, int count) {
    double sum_x = 0, sum_x2 = 0, sum_x3 = 0, sum_x4 = 0, sum_x5 = 0, sum_x6 = 0;
    double sum_y = 0, sum_xy = 0, sum_x2y = 0, sum_x3y = 0;
	int i;
    for (i = 0; i < count; i++) {
        int x = years[i] - 1960;
        double y = y_values[i];

        double x2 = x * x;
        double x3 = x2 * x;
        double x4 = x3 * x;
        double x5 = x4 * x;
        double x6 = x5 * x;

        sum_x += x;
        sum_x2 += x2;
        sum_x3 += x3;
        sum_x4 += x4;
        sum_x5 += x5;
        sum_x6 += x6;

        sum_y += y;
        sum_xy += x * y;
        sum_x2y += x2 * y;
        sum_x3y += x3 * y;
    }

    double matrix[4][5] = {
        {sum_x3, sum_x2, sum_x, count, sum_y},
        {sum_x4, sum_x3, sum_x2, sum_x, sum_xy},
        {sum_x5, sum_x4, sum_x3, sum_x2, sum_x2y},
        {sum_x6, sum_x5, sum_x4, sum_x3, sum_x3y}
    };
    
    for (i = 0; i < 4; i++) {
    	int k;
        int max_row = i;
        for (k = i + 1; k < 4; k++) {
            if (fabs(matrix[k][i]) > fabs(matrix[max_row][i])) {
                max_row = k;
            }
        }

        for (k = i; k < 5; k++) {
            double temp = matrix[i][k];
            matrix[i][k] = matrix[max_row][k];
            matrix[max_row][k] = temp;
        }
		int j;
        for (k = i + 1; k < 4; k++) {
            double factor = matrix[k][i] / matrix[i][i];
            for (j = i; j < 5; j++) {
                matrix[k][j] -= factor * matrix[i][j];
            }
        }
    }

    CubicCoefficients coeffs;
    coeffs.d = matrix[3][4] / matrix[3][3];
    coeffs.c = (matrix[2][4] - matrix[2][3] * coeffs.d) / matrix[2][2];
    coeffs.b = (matrix[1][4] - matrix[1][3] * coeffs.d - matrix[1][2] * coeffs.c) / matrix[1][1];
    coeffs.a = (matrix[0][4] - matrix[0][3] * coeffs.d - matrix[0][2] * coeffs.c - matrix[0][1] * coeffs.b) / matrix[0][0];

    return coeffs;
}

int main() {
    int n;
    int i, k, year; 
    DataPoint* data = read_csv("Data Tugas Pemrograman A.csv", &n);
    qsort(data, n, sizeof(DataPoint), compare_years);

    int start_year = 1960;
    int end_year = 2023;
    int total_years = end_year - start_year + 1;
    DataPoint* complete_data = malloc(sizeof(DataPoint) * total_years);
    int complete_count = 0;

    int current_data_index = 0;
    for (year = start_year; year <= end_year; year++) {
        if (current_data_index < n && data[current_data_index].year == year) {
            complete_data[complete_count++] = data[current_data_index++];
        } else {
            DataPoint prev, next;
            int found_prev = 0, found_next = 0;
			int i;
            for (i = current_data_index - 1; i >= 0; i--) {
                if (data[i].year < year) {
                    prev = data[i];
                    found_prev = 1;
                    break;
                }
            }

            for (i = current_data_index; i < n; i++) {
                if (data[i].year > year) {
                    next = data[i];
                    found_next = 1;
                    break;
                }
            }

            if (found_prev && found_next) {
                int gap = next.year - prev.year;
                double fraction = (double)(year - prev.year) / gap;
                DataPoint interpolated;
                interpolated.year = year;
                interpolated.population = prev.population + (next.population - prev.population) * fraction;
                interpolated.percentage = prev.percentage + (next.percentage - prev.percentage) * fraction;
                interpolated.is_interpolated = 1;
                complete_data[complete_count++] = interpolated;
            } else {
                fprintf(stderr, "Cannot extrapolate for year %d\n", year);
                exit(1);
            }
        }
    }

    FILE* output = fopen("output.csv", "w");
    fprintf(output, "Year,Percentage_Internet_User,Population\n");

    for (i = 0; i < total_years; i++) {
        fprintf(output, "%d,%.6f,%lld\n", complete_data[i].year, complete_data[i].percentage, complete_data[i].population);
    }
    fclose(output);

    int* years = malloc(total_years * sizeof(int));
    double* populations = malloc(total_years * sizeof(double));
    double* percentages = malloc(total_years * sizeof(double));

    for (i = 0; i < total_years; i++) {
        years[i] = complete_data[i].year;
        populations[i] = complete_data[i].population;
        percentages[i] = complete_data[i].percentage;
    }

    CubicCoefficients pop_coeffs = fit_cubic(years, populations, total_years);
    CubicCoefficients perc_coeffs = fit_cubic(years, percentages, total_years);

    printf("\n2a. Persentase pengguna Internet Indonesia: y = %.6fx^3 + %.6fx^2 + %.6fx + %.6f\n", perc_coeffs.a, perc_coeffs.b, perc_coeffs.c, perc_coeffs.d);
    printf("2b. Pertumbuhan populasi Indonesia: y = %.6fx^3 + %.6fx^2 + %.6fx + %.6f\n", pop_coeffs.a, pop_coeffs.b, pop_coeffs.c, pop_coeffs.d);

    // Estimasi untuk 2030 dan 2035
    int x_2030 = 2030 - 1960;
    double pop_2030 = pop_coeffs.a * x_2030 * x_2030 * x_2030 + pop_coeffs.b * x_2030 * x_2030 + pop_coeffs.c * x_2030 + pop_coeffs.d;

    int x_2035 = 2035 - 1960;
    double perc_2035 = perc_coeffs.a * x_2035 * x_2035 * x_2035 + perc_coeffs.b * x_2035 * x_2035 + perc_coeffs.c * x_2035 + perc_coeffs.d;
    double pop_2035 = pop_coeffs.a * x_2035 * x_2035 * x_2035 + pop_coeffs.b * x_2035 * x_2035 + pop_coeffs.c * x_2035 + pop_coeffs.d;
    double internet_users_2035 = (perc_2035 / 100.0) * pop_2035;

    printf("\n3a. Estimasi populasi Indonesia tahun 2030: %.0f\n", pop_2030);
    printf("3b. Estimasi pengguna Internet tahun 2035: %.0f\n", internet_users_2035);

    free(data);
    free(complete_data);
    free(years);
    free(populations);
    free(percentages);

    return 0;
}
