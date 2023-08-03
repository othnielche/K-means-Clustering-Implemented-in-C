#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int number_of_columns(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        printf("Failed to open the file.\n");
        return -1;
    }

    char line[1024];
    fgets(line, sizeof(line), file);

    int num_col = 1;
    int length = strlen(line);
    for (int i = 0; i < length ; i++){
        if (line[i] == ','){
            num_col++;
        }
    }
    fclose(file);
    
    return num_col;
}

int number_of_rows(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        printf("Failed to open the file. \n");
        return -1;
    }

    int num_rows = 0;
    char ch; 
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n'){
            num_rows++;
        }
    }

    fclose(file);
    
    return  num_rows;
}

double** readCSVData(const char* filePath, int numRows, int numColumns) {
    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        printf("Failed to open the file.\n");
        return NULL;
    }

    double** data = (double**)malloc(numRows * sizeof(double*));
    if (data == NULL) {
        printf("Failed to allocate memory.\n");
        fclose(file);
        return NULL;
    }

    for (int i = 0; i < numRows; i++) {
        data[i] = (double*)malloc(numColumns * sizeof(double));
        if (data[i] == NULL) {
            printf("Failed to allocate memory.\n");
            fclose(file);
            for (int j = 0; j < i; j++) {
                free(data[j]);
            }
            free(data);
            return NULL;
        }
    }

    char line[1024];
    int row = 0;
    while (fgets(line, sizeof(line), file) && row < numRows) {
        char* token = strtok(line, ",");
        int column = 0;
        while (token != NULL && column < numColumns) {
            data[row][column] = atof(token);
            column++;
            token = strtok(NULL, ",");
        }
        row++;
    }

    fclose(file);
    return data;
}


double euclidean_distance(double* vector1, double* vector2, int dim){
    double distance = 0.0;
    for (int i = 0; i < dim; i++ ){
        distance += pow((vector1[i] - vector2[i]), 2);
    }

    return sqrt(distance);
}

double cosine_similarity(double* vector1, double* vector2, int dim){
    double dot_product = 0.0, mag_vector1 = 0.0, mag_vector2 = 0.0;
    for (int i = 0; i < dim; i++){
        dot_product += vector1[i] *vector2[i];
        mag_vector1 += pow(vector1[i], 2);
        mag_vector2 += pow(vector2[i], 2);
    }

    mag_vector1 = sqrt(mag_vector1);
    mag_vector2 = sqrt(mag_vector2);

    return dot_product / (mag_vector1 * mag_vector2); 
}

double** initialize_centroids(double** data, int numRows, int numColumns, int k) {
    if (k <= 0 || k > numRows) {
        printf("Invalid number of rows to select.\n");
        return NULL;
    }

    // Seed the random number generator
    srand(time(NULL));

    // Create a new 2D array to store the selected rows
    double** centroids = (double**)malloc(k * sizeof(double*));
    if (centroids == NULL) {
        printf("Failed to allocate memory.\n");
        return NULL;
    }

    // Generate k random indices without repetition
    int* indices = (int*)malloc(numRows * sizeof(int));
    if (indices == NULL) {
        printf("Failed to allocate memory.\n");
        free(centroids);
        return NULL;
    }

    for (int i = 0; i < numRows; i++) {
        indices[i] = i;
    }

    for (int i = 0; i < k; i++) {
        int j = rand() % (numRows - i) + i;
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;

        // Copy the selected row from data to centroids
        centroids[i] = (double*)malloc(numColumns * sizeof(double));
        if (centroids[i] == NULL) {
            printf("Failed to allocate memory.\n");
            for (int r = 0; r < i; r++) {
                free(centroids[r]);
            }
            free(indices);
            free(centroids);
            return NULL;
        }

        for (int c = 0; c < numColumns; c++) {
            centroids[i][c] = data[indices[i]][c];
        }
    }

    free(indices);
    return centroids;
}

/*int* assign_cluster(double** data, double** centroids, int numrows, int numcols, int k, int similarity_measure){
    int closest_cluster;
    double distance, similarity, max_similarity;
    int* cluster_assignment = (int*)malloc(numrows * sizeof(int)); 
    int num_changed = 0;
    for (int i = 0; i < numrows; i++){
        closest_cluster = -1;
        max_similarity = -1.0;

        for (int j = 0;  j < k; j++){
            switch(similarity_measure){
                case 0:
                    distance = euclidean_distance(data[i], centroids[j], numcols);
                    similarity = 1 /(1 + distance);
                    break;
                case 1:
                    similarity = cosine_similarity(data[i], centroids[j], numcols);
                    break;
                default: 
                    printf("Error: An invalid similarity measure has been chosen \n");
                    exit(1);
            }

            if (similarity > max_similarity){
                max_similarity = similarity;
                closest_cluster = j;
            }
        }
        if (cluster_assignment[i] != closest_cluster){
            cluster_assignment[i] = closest_cluster;
            num_changed++;
        }
    }
    return cluster_assignment;
}*/

/*void update_centroids(double** data, double** centroids, int num_rows, int num_cols, int k, int* cluster_assignment){
    int count;
    int assigned_clusters;
    double** centroid_sum = (double**)malloc(num_rows *sizeof(double)); 

    //initialize centroid_sum and count
    for (int i; i < k; i++){
        count = 0;
        centroid_sum[i] = (double*)malloc( k * sizeof(double));
        
        for (int j = 0; j < num_cols; j++){
            centroid_sum[i][j] = 0.0;
        }
    }

    //calculating centroid sum
    for (int i = 0; i < num_rows; i++){
        assigned_clusters = cluster_assignment[i];

        for (int j = 0; j < num_cols; j++){
            centroid_sum[assigned_clusters][j] += data[i][j];
        }

        count++;
    }

    //update centroids 
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < num_cols; j++){
            centroids[i][j] = centroid_sum[i][j] / count;
        }
    }

}*/

int* assign_cluster(double** data, double** centroids, int numrows, int numcols, int k, int similarity_measure) {
    int* cluster_assignment = (int*)malloc(numrows * sizeof(int));
    int converged = 0;

    while (!converged) {
        int num_changed = 0;
        converged = 1;

        for (int i = 0; i < numrows; i++) {
            int closest_cluster = -1;
            double max_similarity = -1.0;

            for (int j = 0; j < k; j++) {
                double similarity;

                switch(similarity_measure) {
                    case 0:
                        similarity = 1 / (1 + euclidean_distance(data[i], centroids[j], numcols));
                        break;
                    case 1:
                        similarity = cosine_similarity(data[i], centroids[j], numcols);
                        break;
                    default:
                        printf("Error: An invalid similarity measure has been chosen.\n");
                        exit(1);
                }

                if (similarity > max_similarity) {
                    max_similarity = similarity;
                    closest_cluster = j;
                }
            }

            if (cluster_assignment[i] != closest_cluster) {
                cluster_assignment[i] = closest_cluster;
                num_changed++;
            }
        }

        if (num_changed > 0)
            converged = 0;
    }

    return cluster_assignment;
}

void update_centroids(double** data, double** centroids, int num_rows, int num_cols, int k, int* cluster_assignment) {
    int* cluster_counts = (int*)calloc(k, sizeof(int));

    for (int i = 0; i < num_rows; i++) {
        int assigned_cluster = cluster_assignment[i];
        cluster_counts[assigned_cluster]++;
    }

    for (int i = 0; i < k; i++) {
        for (int j = 0; j < num_cols; j++) {
            double centroid_sum = 0.0;

            for (int r = 0; r < num_rows; r++) {
                if (cluster_assignment[r] == i)
                    centroid_sum += data[r][j];
            }

            centroids[i][j] = centroid_sum / cluster_counts[i];
        }
    }

    free(cluster_counts);
}

int check_convergence(int* prev_assignment, int* curr_assignment, int num_rows) {
    for (int i = 0; i < num_rows; i++) {
        if (prev_assignment[i] != curr_assignment[i])
            return 0;
    }

    return 1;
}

void print_clusters(double** data, int num_rows, int num_cols, int* cluster_assignment, int k) {
    for (int i = 0; i < k; i++) {
        printf("Cluster %d:\n", i + 1);

        for (int j = 0; j < num_rows; j++) {
            if (cluster_assignment[j] == i) {
                printf("Data point %d: ", j + 1);

                for (int c = 0; c < num_cols; c++) {
                    printf("%.2f ", data[j][c]);
                }

                printf("\n");
            }
        }

        printf("\n");
    }
}

int main(){
    char filepath[256];
    printf("Enter the file path for the CSV file: ");
    scanf("%s", filepath);
    
    //Value for k
    int num_clusters;
    printf("\nEnter the number of clusters(k): ");
    scanf("%d", &num_clusters);

    int similarity_measure;
    printf("\nChoose Similarity measure.\nEnter 0 for Euclidean distnace.\nEnter 1 for cosine similarty: ");
    scanf("%d", &similarity_measure);

    int numrows = number_of_rows(filepath);
    int numcols = number_of_columns(filepath);
    
    double** csvData = readCSVData(filepath, numrows, numcols);
    

    if (csvData == NULL){
        printf("\n No data found");

        exit(1);
    }

    printf("\nData From CSV file: ");
    printf("\n");

    for (int i = 0; i < numrows; i++){
        
        for (int j = 0; j < numcols; j++){
            printf("%.2f ", csvData[i][j]);
        }
        printf("\n________________________________\n");
    } 

    printf("\nNumber of rows in the csv file: %d", numrows);
    printf("\nNumber of columns in the CSV file is: %d", numcols);

    //testing euclidean distance function
    //double eucl = euclidean_distance(csvData[1], csvData[2], numcols);

    //testing initializing centroids function
    double** centriods = initialize_centroids(csvData, numrows, numcols, num_clusters);
    
    if (centriods == NULL) {
        printf("Failed to initialize centroids.\n");
        return 1;
    }

    printf("\n\nInitialized %d Centoroids: ", num_clusters);
    printf("\n");

    for (int i = 0; i < num_clusters; i++){
        for (int j = 0; j < numcols; j++){
            printf("%.2f ", centriods[i][j]);

        }
        printf("\n__________________________________\n");
    }

    int* assigned_clusters = assign_cluster(csvData, centriods, numrows, numcols, num_clusters, similarity_measure);
    /*for (int i = 0; i < numrows; i++){
            printf("\n%d", assigned_clusters[i]);
            printf("\n");
    }*/

    update_centroids(csvData, centriods, numrows, numcols, num_clusters, assigned_clusters);
     

    /*for (int i = 0; i < num_clusters; i++){
        for (int j = 0; j < numcols; j++){
            printf("%.2f ", centriods[i][j]);

        }
        printf("\n");
    }*/   

    int* prev_cluster_assignment = (int*)malloc(numrows * sizeof(int));
    int* curr_cluster_assignment = assign_cluster(csvData, centriods, numrows, numcols, num_clusters, similarity_measure);

    int iterations = 0;

    while (!check_convergence(prev_cluster_assignment, curr_cluster_assignment, numrows)) {
        iterations++;
        printf("Iteration %d:\n", iterations);

        update_centroids(csvData, centriods, numrows, numcols, num_clusters, curr_cluster_assignment);

        int* temp = prev_cluster_assignment;
        prev_cluster_assignment = curr_cluster_assignment;
        curr_cluster_assignment = temp;

        curr_cluster_assignment = assign_cluster(csvData, centriods, numrows, numcols, num_clusters, similarity_measure);
    }

    printf("\nFinal clusters:\n");
    print_clusters(csvData, numrows, numcols, curr_cluster_assignment, num_clusters);


    getchar();
    getchar();
    
    for (int i = 0; i < numrows; i++){
        free(csvData[i]);
    }

    free(assigned_clusters);

    free(csvData);
    
    for (int i = 0; i < num_clusters; i++){
        free(centriods[i]);
    }
    free(centriods);

    free(prev_cluster_assignment);
    free(curr_cluster_assignment);

    return 0;
}