#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gmp.h>
#include <sys/time.h> 
#include <time.h>
#include "thpool.h"  // Include the thread pool

// Global variable to store the path to soffice
char soffice_path[1024] = {0};

// Function to check file extension
int has_extension(const char *filename, const char *extension) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return 0;
    return strcmp(dot + 1, extension) == 0;
}

// Function to find the path of 'soffice' dynamically
void find_soffice_path() {
    // Use the find command to search for 'soffice' within the Applications directory
    FILE* pipe = popen("find /Applications -name soffice 2> /dev/null", "r");
    if (pipe) {
        if (fgets(soffice_path, sizeof(soffice_path), pipe) != NULL) {
            // Remove newline character from the end of path
            size_t len = strlen(soffice_path);
            if (len > 0 && soffice_path[len-1] == '\n') {
                soffice_path[len-1] = '\0';
            }
        }
        pclose(pipe);
    }
}

// Convert PDF to text using Poppler's pdftotext utility
void pdf_to_text(const char *input_path, const char *output_path) {
    char command[256];
    sprintf(command, "pdftotext '%s' '%s'", input_path, output_path);
    if (system(command) != 0) {
        perror("Failed to convert PDF to text");
    }
}

// Convert Word document to text using LibreOffice's soffice utility
void word_to_text(const char *input_path, const char *output_path) {
    char command[256];
    // Use the globally stored soffice path
    sprintf(command, "%s --headless --convert-to txt:Text --outdir /tmp '%s'", soffice_path, input_path);
    if (system(command) != 0) {
        perror("Failed to convert Word document to text");
    }

    // Extract the base name of the input file for use in the output txt filename
    const char *base_name = strrchr(input_path, '/');
    base_name = base_name ? base_name + 1 : input_path;
    char temp_base_name[256];
    strcpy(temp_base_name, base_name);
    char *dot = strrchr(temp_base_name, '.');
    if (dot) *dot = '\0';

    char temp_path[256];
    sprintf(temp_path, "/tmp/%s.txt", temp_base_name);
    if (rename(temp_path, output_path) != 0) {
        perror("Failed to move output text file");
    }
}

// Simple copy function for text files
void copy_text_file(const char *input_path, const char *output_path) {
    char buffer[1024];
    FILE *input_file = fopen(input_path, "r");
    if (!input_file) {
        perror("Failed to open input file");
        return;
    }
    FILE *output_file = fopen(output_path, "w");
    if (!output_file) {
        perror("Failed to open output file");
        fclose(input_file);
        return;
    }

    while (fgets(buffer, sizeof(buffer), input_file)) {
        fputs(buffer, output_file);
    }

    fclose(input_file);
    fclose(output_file);
}

void _mr_unrank1(mpz_t rank, int n, mpz_t *vec) {
    if (n < 1) return;

    mpz_t q, r;
    mpz_init(q);
    mpz_init(r);

    mpz_fdiv_qr_ui(q, r, rank, n);
    unsigned long r_ui = mpz_get_ui(r);

    #define SWAP(a,b) do{ mpz_swap(a, b); }while(0)
    SWAP(vec[r_ui], vec[n-1]);
    _mr_unrank1(q, n-1, vec);

    mpz_clear(q);
    mpz_clear(r);
}

void _mr_rank1(int n, mpz_t *vec, int *inv, mpz_t rank) {
    // Base case: If there's only one element, the rank is zero.
    if (n < 2) {
        mpz_set_ui(rank, 0);
        return;
    }

    // Find the index of the last element in the permutation.
    unsigned long s = mpz_get_ui(vec[n-1]);

    // Swap the last element with the element at its index.
    SWAP(vec[n-1], vec[inv[n-1]]);
    int temp = inv[s];
    inv[s] = inv[n-1];
    inv[n-1] = temp;

    // Create variables to hold 'n' and 's' as mpz_t types.
    mpz_t n_mpz, s_mpz;
    mpz_init(n_mpz);
    mpz_init(s_mpz);
    
    // Set 'n' and 's' into the GMP variables.
    mpz_set_ui(n_mpz, n);
    mpz_set_ui(s_mpz, s);

    // Create a temporary mpz_t to hold the rank from the recursive call.
    mpz_t temp_rank;
    mpz_init(temp_rank);

    // Recursive call to calculate the rank of the n-1 subset.
    _mr_rank1(n-1, vec, inv, temp_rank);

    // Multiply the rank from the recursive call by 'n' and add 's'.
    mpz_mul(rank, temp_rank, n_mpz);
    mpz_add(rank, rank, s_mpz);

    // Clear the mpz_t variables that were used in this call.
    mpz_clear(n_mpz);
    mpz_clear(s_mpz);
    mpz_clear(temp_rank);
}

void get_permutation(mpz_t rank, int n, mpz_t *vec) {
    // Timing variables
    clock_t start, end;
    double time_spent;

    // Record start time
    start = clock();

    // Initialize the array with values
    for (int i = 0; i < n; ++i) {
        mpz_init(vec[i]);
        mpz_set_ui(vec[i], i);
    }

    // Call the function to compute the permutation
    _mr_unrank1(rank, n, vec);

    // Record end time
    end = clock();

    // Calculate time spent
    time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    // Print the latency
    printf("Latency of get_permutation function: %f seconds\n", time_spent);
}



void get_rank(int n, mpz_t *vec, mpz_t rank_result) {
    // Timing variables
    clock_t start, end;
    double time_spent;

    // Record start time
    start = clock();

    // Allocate memory for the inverse array and a copy of vec
    int *inv = malloc(n * sizeof(int));
    mpz_t *v = malloc(n * sizeof(mpz_t));
    if (inv == NULL || v == NULL) {
        fprintf(stderr, "Error allocating memory.\n");
        free(inv);
        free(v);
        return;
    }

    for (int i = 0; i < n; ++i) {
        mpz_init(v[i]);
        mpz_set(v[i], vec[i]);
        inv[mpz_get_ui(vec[i])] = i;
    }

    // Create a local rank variable
    mpz_t rank;
    mpz_init(rank);

    // Call `_mr_rank1` to compute the rank
    _mr_rank1(n, v, inv, rank);

    // Set the computed rank into the passed `rank_result` variable
    mpz_set(rank_result, rank);

    // Clean up
    for (int i = 0; i < n; ++i) {
        mpz_clear(v[i]);
    }
    mpz_clear(rank);
    free(inv);
    free(v);

    // Record end time
    end = clock();

    // Calculate time spent
    time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    // Print the latency
    printf("Latency of get_rank function: %f seconds\n", time_spent);
}


void load_text_to_gmp(const char *filename, mpz_t result) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    size_t chars_read;

    mpz_init(result);

    // Timing variables
    struct timeval start, end;
    gettimeofday(&start, NULL); // Start timing after the file is opened

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        for (chars_read = 0; buffer[chars_read] != '\0'; chars_read++) {
            if (buffer[chars_read] == '\n' || buffer[chars_read] == '\r') {
                continue;  // Skip newline characters
            }
            mpz_mul_ui(result, result, 256);  // Shift left by 8 bits (1 byte)
            mpz_add_ui(result, result, (unsigned char)buffer[chars_read]);  // Add the next byte
        }
    }

    gettimeofday(&end, NULL); // Stop timing after processing is complete

    // Calculate time spent in seconds
    double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;

    // Print the latency with a clear description
    printf("Processing and conversion latency of load_text_to_gmp function (excluding file opening): %f seconds\n", time_spent);

    fclose(file);
}


void get_rank_from_function(mpz_t result_rank, const char *filename) {
    // Timing variables
    clock_t start, end;
    double time_spent;

    // Record start time
    start = clock();

    // Initialize result_rank
    mpz_init(result_rank);

    // Load text from file and convert it to a big integer
    load_text_to_gmp(filename, result_rank);

    // Record end time
    end = clock();

    // Calculate time spent
    time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    // Print the latency
    printf("Latency of get_rank_from_function function: %f seconds\n", time_spent);
}


void save_gmp_to_text(const char *filename, mpz_t gmp_data) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    mpz_t temp;
    mpz_init_set(temp, gmp_data);
    size_t total_bits = mpz_sizeinbase(gmp_data, 2);
    size_t total_bytes = (total_bits + 7) / 8; // Round up to the nearest byte

    char *output = (char *)malloc(total_bytes + 1);
    if (!output) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    output[total_bytes] = '\0'; // Null-terminate the string

    for (int i = total_bytes - 1; i >= 0; i--) {
        mpz_t byte;
        mpz_init(byte);
        mpz_mod_ui(byte, temp, 256); // Get the least significant byte
        output[i] = (char)mpz_get_ui(byte);
        mpz_fdiv_q_ui(temp, temp, 256); // Shift right by 8 bits (1 byte)
        mpz_clear(byte);
    }

    fputs(output, file); // Write the string to file

    free(output);
    mpz_clear(temp);
    fclose(file);
}

// Task function for the thread pool
void process_file(void* arg) {
    char* filename = (char*) arg;
    char output_path[256];
    sprintf(output_path, "%s.txt", filename);  // Construct the output path

    if (has_extension(filename, "pdf")) {
        pdf_to_text(filename, output_path);
        printf("Input filename: %s\n", filename);
    } else if (has_extension(filename, "doc") || has_extension(filename, "docx")) {
        word_to_text(filename, output_path);
        printf("Input filename: %s\n", filename);
    } else if (has_extension(filename, "txt")) {
        copy_text_file(filename, output_path);
        printf("Input filename: %s\n", filename);
    } else {
        fprintf(stderr, "Unsupported file type: %s\n", filename);
    }

    const char *input_filename = output_path;

    char op_filename[256];
    sprintf(op_filename, "outputfile_%s", output_path);
    const char *output_filename = op_filename;
    
    int n = 1000; 
    mpz_t rank, perm[1000], rank_result;
    int i;

    for (i = 0; i < n; ++i) {
        mpz_init(perm[i]);
    }
    mpz_init(rank);
    mpz_init(rank_result);

    mpz_t gmp_data;

    get_rank_from_function(rank_result, input_filename );
    
    save_gmp_to_text(output_filename, rank_result);

    get_permutation(rank_result, n, perm);
    
    for (i = 0; i < n; ++i) {
        mpz_clear(perm[i]);
    }
    mpz_clear(rank);
    mpz_clear(rank_result);

    printf("File processed and saved successfully.\n");

}

int main(int argc, char **argv) {
    // Timing variables
    struct timeval start, end;
    gettimeofday(&start, NULL);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_file1> <input_file2> ... <input_fileN>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Find the path of 'soffice'
    find_soffice_path();
    if (strlen(soffice_path) == 0) {
        fprintf(stderr, "Failed to locate 'soffice'. Please ensure LibreOffice is installed.\n");
        return EXIT_FAILURE;
    }

    // Initialize a thread pool with an appropriate number of worker threads
    int num_threads = 4;
    threadpool thpool = thpool_init(num_threads);

    // Submit a task for each file
    for (int i = 1; i < argc; i++) {
        thpool_add_work(thpool, process_file, argv[i]);
    }

    // Wait for all tasks to complete
    thpool_wait(thpool);

    // Clean up the thread pool
    thpool_destroy(thpool);
    printf("All files have been processed.\n");

    // Record end time
    gettimeofday(&end, NULL);

    // Calculate time spent
    double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;

    // Print the latency of the entire program
    printf("Total program execution time: %f seconds\n", time_spent);

    return 0;
}

