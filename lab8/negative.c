#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/time.h>

#define NUMBERS "numbers"
#define BLOCK "block"

int W, H, M;
char mode[3];

void check_input(int argc, char *argv[], int *threads_num, char **option, char **img_file_name, char **res_file_name){
    if (argc != 5){
        printf("Wrong number of arguments!\n");
        exit(EXIT_FAILURE);
    }

    if ((*threads_num = atoi(argv[1])) == 0){
        printf("Threads number must be a positive integer!\n");
        exit(EXIT_FAILURE);
    }

    *option = argv[2];
    if (strcmp(*option, NUMBERS) && strcmp(*option, BLOCK)){
        printf("Provided option is incorrect!\n");
        exit(EXIT_FAILURE);
    }

    *img_file_name = argv[3];
    *res_file_name = argv[4];
}

void check_comments(FILE *fp){
    int c;
    char line[80];
    while((c = fgetc(fp)) != EOF && isspace(c));

    if (c == '#'){
        fgets(line, sizeof(line), fp);
        check_comments(fp);
    }
    else
        fseek(fp, -1, SEEK_CUR);
}

struct Data {
        int **img_arr;
        int min_val;
        int max_val;
        int W;
        int *i_indices;
        int *j_indices;
        int indices_len;
    };

void *assign_pixels(void *ptr){
    struct timeval t0, t1, res;
    gettimeofday(&t0, NULL);
    struct Data *data = (struct Data*) ptr;
    int cnt = 0;
    for (int i = 0; i < H; i++){
        for (int j = 0; j < W; j++){
            if (data->img_arr[i][j] >= data->min_val && data->img_arr[i][j] <= data->max_val){
                data->i_indices[cnt] = i;
                data->j_indices[cnt] = j;
                cnt++;
            }  
        }
    }
    data->indices_len = cnt;

    gettimeofday(&t1, NULL);
    timersub(&t1, &t0, &res);
    long time = res.tv_sec * 1e6 + res.tv_usec;

    pthread_exit((void *) time);
}

void *compute_by_value(void *ptr){
    struct timeval t0, t1, res;

    gettimeofday(&t0, NULL);
    struct Data *data = (struct Data*) ptr;
    for (int i = 0; i < data->indices_len; i++)
        data->img_arr[data->i_indices[i]][data->j_indices[i]] = 255 - data->img_arr[data->i_indices[i]][data->j_indices[i]];
    
    gettimeofday(&t1, NULL);
    timersub(&t1, &t0, &res);
    long time = res.tv_sec * 1e6 + res.tv_usec;

    pthread_exit((void *) time);
}


void *compute_by_range(void *ptr){
    struct timeval t0, t1, res;
    gettimeofday(&t0, NULL);
    struct Data *data = (struct Data*) ptr;
    int max_j;
    if (data->max_val >= data->W)
        max_j = data->W - 1;
    else
        max_j = data->max_val;

    for (int i = 0; i < H; i++){
        for (int j = data->min_val; j <= max_j; j++)
            data->img_arr[i][j] = 255 - data->img_arr[i][j];
    }
    gettimeofday(&t1, NULL);
    timersub(&t1, &t0, &res);
    long time = res.tv_sec * 1e6 + res.tv_usec;

    pthread_exit((void *) time);
}

void save_to_file(char *res_file_name, char *mode, int **arr){
    FILE *res_fp = fopen(res_file_name, "w");
    fwrite(mode, sizeof(char), strlen(mode), res_fp);
    fwrite("\n", sizeof(char), sizeof(char), res_fp);

    char line[80];
    sprintf(line, "%d  %d\n", W, H);
    fwrite(line, sizeof(char), strlen(line), res_fp);
    
    sprintf(line, "%d\n", M);
    fwrite(line, sizeof(char), strlen(line), res_fp);

    int cnt = 0;
    for (int i = 0; i < H; i++){
        for (int j = 0; j < W; j++){
            cnt++;
            if (cnt == 12){
                sprintf(line, "%d\n", arr[i][j]);
                cnt = 0;
            }
            else
                sprintf(line, "%d  ", arr[i][j]);
                
            fwrite(line, sizeof(char), strlen(line), res_fp);
        }
    }

    for (int i = 0; i < H; i++)
        free(arr[i]);

    free(arr);
    fclose(res_fp);
}

int **read_input_file(char *img_file_name){
    FILE *fp;
    if ((fp = fopen(img_file_name, "r")) == NULL){
        printf("Can't open provided file!\n");
        exit(EXIT_FAILURE);
    }

    fscanf(fp, "%s", mode);
    check_comments(fp);

    fscanf(fp, "%d %d", &W, &H);
    check_comments(fp);
    fscanf(fp, "%d", &M);
    check_comments(fp);

    int **arr = malloc(sizeof(int *) * H);
    for (int i = 0; i < H; i++){
        arr[i] = malloc(sizeof(int) * W);
        for (int j = 0; j < W; j++)
            fscanf(fp, "%d", &arr[i][j]);
    }

    fclose(fp);
    return arr;
}

void handle_option(int **arr, int threads_num, char *option, FILE *times_fp){
    pthread_t *id = malloc(sizeof(pthread_t) * threads_num);
    struct Data *data = malloc(sizeof(struct Data) * threads_num);
    long *times = malloc(sizeof(long) * threads_num);
    long *prep_times = malloc(sizeof(long) * threads_num);
    if (!strcmp(option, NUMBERS)){
        int next = 0;
        double div = 256 / (double) threads_num;
        for (int i = 0; i < threads_num; i++){
            data[i].img_arr = arr;
            data[i].min_val = next;
            data[i].max_val = (i + 1) * div;
            data[i].i_indices = malloc(sizeof(int) * W * H);
            data[i].j_indices = malloc(sizeof(int) * W * H);
            pthread_create(&id[i], NULL, assign_pixels, (void *) &data[i]);
            next = data[i].max_val + 1;
        }

        for (int i = 0; i < threads_num; i++)
            pthread_join(id[i], (void *) &prep_times[i]);

        for (int i = 0; i < threads_num; i++)
            pthread_create(&id[i], NULL, compute_by_value, (void *) &data[i]);
    }
    else{
        for (int i = 0; i < threads_num; i++){
            data[i].img_arr = arr;
            data[i].min_val = i * (W / threads_num + 1);
            data[i].max_val = (i + 1) * (W / threads_num + 1) - 1;
            data[i].W = W;
            pthread_create(&id[i], NULL, compute_by_range, (void *) &data[i]);
        }
    }

    for (int i = 0; i < threads_num; i++)
        pthread_join(id[i], (void *) &times[i]);

    char buffer[256];
    for (int i = 0; i < threads_num; i++){
        if (!strcmp(option, NUMBERS))
            sprintf(buffer, "Thread number %d: %ld microseconds\n", i + 1, times[i] + prep_times[i]);
        else
            sprintf(buffer, "Thread number %d: %ld microseconds\n", i + 1, times[i]);
        
        fwrite(buffer, sizeof(char), strlen(buffer), times_fp);
    }

    if (!strcmp(option, NUMBERS)){
        for (int i = 0; i < threads_num; i++){
            free(data[i].i_indices);
            free(data[i].j_indices);
        }
    }
    free(times);
    free(id);
    free(data);
    free(prep_times);
}


int main(int argc, char *argv[]){
    int threads_num;
    char *option, *img_file_name, *res_file_name;

    check_input(argc, argv, &threads_num, &option, &img_file_name, &res_file_name);
    int **arr = read_input_file(img_file_name);
    
    // saving info to Times.txt file
    FILE *times_fp = fopen("Times.txt", "a");
    char buffer[256];
    unsigned long long pixels = W * H;
    double ratio = pixels / threads_num;
    sprintf(buffer, "Image: %s\nPixels: %llu; Number of threads: %d\nRatio: %lf\nMethod: %s\n", img_file_name, pixels, threads_num, ratio, option);
    fwrite(buffer, sizeof(char), strlen(buffer), times_fp);
    // end saving info to Times.txt file

    struct timeval t0, t1, res;
    gettimeofday(&t0, NULL);

    handle_option(arr, threads_num, option, times_fp);
    
    // saving info to Times.txt file
    gettimeofday(&t1, NULL);
    timersub(&t1, &t0, &res);
    long time = res.tv_sec * 1e6 + res.tv_usec;
    printf("Img: %s; Threads: %d;\nReal time used by main thread: %ld microseconds\n\n", img_file_name, threads_num, time);
    sprintf(buffer, "Real time used by main thread: %ld microseconds\n\n", time);
    fwrite(buffer, sizeof(char), strlen(buffer), times_fp);
    // end saving info to Times.txt file

    // save changed images
    save_to_file(res_file_name, mode, arr);
    fclose(times_fp);
    return 0;
}