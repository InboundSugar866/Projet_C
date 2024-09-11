#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 256

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int command_executed = 0;

// Struct for passing data to threads
typedef struct {
    int index;
    int rows;
    int cols;
    char **map;
    char (*position)[MAX_LINE_LENGTH];
    char *orientations;
    char (*commande)[MAX_LINE_LENGTH];
    int **drone_indices;
} DroneData;

// function to read the txt file
void read_file(FILE *file, char position[MAX_LINES][MAX_LINE_LENGTH], char commande[MAX_LINES][MAX_LINE_LENGTH], int *line_count) {
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        if (*line_count > 0 && strlen(line) > 1) {
            char *last_space = strrchr(line, ' ');
            if (last_space != NULL) {
                *last_space = '\0';
                strcpy(commande[*line_count - 1], last_space + 1);
            }
            strcpy(position[*line_count - 1], line);
        }
        (*line_count)++;
    }
}

// function to initialize the map with the drones
void initialize_map(int rows, int cols, char **map) {
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            map[i][j] = ' ';
        }
    }
}

// function to add 10 '#' randomly to the map
void add_random_hashes(int rows, int cols, char **map) {
    srand(time(NULL));
    for(int i = 0; i < 50; i++) {
        int x, y;
        do {
            x = (rand() % (rows - 2)) + 1;
            y = rand() % cols;
        } while(map[x][y] != ' ');
        map[x][y] = '#';
    }
}

// function to place 'V' at the positions specified in the 'position' array
void place_drones(char position[MAX_LINES][MAX_LINE_LENGTH], int line_count, int rows, int cols, char **map) {
	int x, y;
	char orientation;
	for(int i = 0; i < line_count-1; i++) {
        sscanf(position[i], "%d %d %c", &x, &y, &orientation);
		if (orientation == 'N') {
			map[x][y] = '^';
		}
		else if (orientation == 'S') {
			map[x][y] = 'v';
		}
		else if (orientation == 'E') {
			map[x][y] = '>';
		}
		else if (orientation == 'W') {
			map[x][y] = '<';
		}
		
    }
}

// function to print the map
void print_map(int rows, int cols, char **map) {
    for(int i = 0; i < cols; i++) {
        printf("=");
    }
    printf("\n");

    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            printf("%c", map[i][j]);
        }
        printf("\n");
    }

    for(int i = 0; i < cols; i++) {
        printf("=");
    }
    printf("\n");
}

// Thread function for printing the map
void *print_map_thread(void *arg) {
    DroneData *data = (DroneData *)arg;
    while (1) {
        pthread_mutex_lock(&lock);
        while (!command_executed) {
            pthread_cond_wait(&cond, &lock);
        }
        print_map(data->rows, data->cols, data->map);  // Assuming you have a function to print the map
        command_executed = 0;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

// function to print the postions and commands
void print_positions_and_commands(char position[MAX_LINES][MAX_LINE_LENGTH], char commande[MAX_LINES][MAX_LINE_LENGTH], int line_count) {
    for(int i = 0; i < line_count-1; i++) {
        printf("Position drone : %s\n", position[i]);
        printf("Commande : %s\n",commande[i]);
    }
}

// When printing the final positions, use the drone_indices array to get the drone index
void find_drones(int rows, int cols, char **map, int **drone_indices) {
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            if(map[i][j] == '^' || map[i][j] == 'v' || map[i][j] == '>' || map[i][j] == '<') {
                char orientation;
                if(map[i][j] == '^') orientation = 'N';
                else if(map[i][j] == 'v') orientation = 'S';
                else if(map[i][j] == '>') orientation = 'E';
                else if(map[i][j] == '<') orientation = 'W';
                printf("Position finale du drone %d: %d %d %c\n", drone_indices[i][j], i, j, orientation);
            }
        }
    }
}

// Function to update orientation based on command
void update_orientation(char *orientation, char command) {
    switch (command) {
        case 'L':
            if (*orientation == 'N') *orientation = 'W';
            else if (*orientation == 'W') *orientation = 'S';
            else if (*orientation == 'S') *orientation = 'E';
            else if (*orientation == 'E') *orientation = 'N';
            break;
        case 'R':
            if (*orientation == 'N') *orientation = 'E';
            else if (*orientation == 'E') *orientation = 'S';
            else if (*orientation == 'S') *orientation = 'W';
            else if (*orientation == 'W') *orientation = 'N';
            break;
    }
}

// Function to update position based on command
void update_position(int *x, int *y, char orientation, char command) {
    switch (command) {
        case 'M':
            if (orientation == 'N') (*x)--;
            else if (orientation == 'E') (*y)++;
            else if (orientation == 'S') (*x)++;
            else if (orientation == 'W') (*y)--;
            break;
        case 'B':
            if (orientation == 'N') (*x)--;
            else if (orientation == 'E') (*y)--;
            else if (orientation == 'S') (*x)++;
            else if (orientation == 'W') (*y)++;
            break;
    }
}



// Function to move drone
void move_drone(int *x, int *y, char *orientation, char command, int rows, int cols, char **map, int **drone_indices, int drone_index) {
    int new_x = *x;
    int new_y = *y;

    // Update orientation and position based on command
    update_orientation(orientation, command);
    update_position(&new_x, &new_y, *orientation, command);

    // Check if the new position is within the map and not an obstacle
    if (new_x >= 0 && new_x < cols && new_y >= 0 && new_y < rows && map[new_x][new_y] != '#') {
        // Update the drone's position on the map
        map[*x][*y] = ' ';  // Remove the drone from its current position
        drone_indices[*x][*y] = -1;  // Remove the drone index from its current position

        char display_orientation = *orientation;
        if (*orientation == 'N') display_orientation = '^';
        else if (*orientation == 'E') display_orientation = '>';
        else if (*orientation == 'S') display_orientation = 'v';
        else if (*orientation == 'W') display_orientation = '<';
		
        map[new_x][new_y] = display_orientation;  // Place the drone at the new position
        drone_indices[new_x][new_y] = drone_index;  // Place the drone index at the new position
        *x = new_x;
        *y = new_y;
    }
}

// Function to execute a sequence of commands for a drone
void execute_commands(int *x, int *y, char *orientation, char *commands, int rows, int cols, char **map, char position[MAX_LINES][MAX_LINE_LENGTH], char orientations[MAX_LINES], int index, int **drone_indices) {
    for (int i = 0; commands[i] != '\0'; i++) {
        if (commands[i] == 'L' || commands[i] == 'R' || commands[i] == 'M' || commands[i] == 'B') {
            move_drone(x, y, orientation, commands[i], rows, cols, map, drone_indices, index);
            sprintf(position[index], "%d %d", *x, *y); // Update the drone's position in the 'position' array
            orientations[index] = *orientation; // Update the drone's orientation
        }
    }
}

// Thread function for moving a single drone
void *move_drone_thread(void *arg) {
    DroneData *data = (DroneData *)arg;
    int x, y;
    char orientation;
    sscanf(data->position[data->index], "%d %d %c", &x, &y, &orientation);
    execute_commands(&x, &y, &orientation, data->commande[data->index], data->rows, data->cols, data->map, data->position, data->orientations, data->index, data->drone_indices);

    pthread_mutex_lock(&lock);
    command_executed = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);

    return NULL;
}


// Function to move all drones according to their command sequences
void move_all_drones(char position[MAX_LINES][MAX_LINE_LENGTH], char commande[MAX_LINES][MAX_LINE_LENGTH], int line_count, int rows, int cols, char **map, char orientations[MAX_LINES], int **drone_indices){
    pthread_t threads[line_count];
    DroneData droneData[line_count];

    pthread_t print_thread;
    pthread_create(&print_thread, NULL, print_map_thread, &droneData[0]);

    for(int i = 0; i < line_count-1; i++) {
        droneData[i].index = i;
        droneData[i].rows = rows;
        droneData[i].cols = cols;
        droneData[i].map = map;
        droneData[i].position = position;
        droneData[i].orientations = orientations;
        droneData[i].commande = commande;
        droneData[i].drone_indices = drone_indices;
        pthread_create(&threads[i], NULL, move_drone_thread, &droneData[i]);
        pthread_join(threads[i], NULL);
    }

    pthread_cancel(print_thread);
}

int main() {
    FILE *file = fopen("version2.txt", "r");
    if (file == NULL) {
        printf("Could not open file\n");
        return 1;
    }

    int rows, cols;
    fscanf(file, "%d %d", &cols, &rows);
    printf("Hauteur carte : %d largeur carte %d\n", cols, rows);

    char position[MAX_LINES][MAX_LINE_LENGTH];
    char commande[MAX_LINES][MAX_LINE_LENGTH];
    int line_count = 0;

    // Create a dynamically allocated 2D array for drone_indices
    int **drone_indices = malloc(rows * sizeof(int *));
    for (int i = 0; i < rows; i++) {
        drone_indices[i] = malloc(cols * sizeof(int));
    }

    read_file(file, position, commande, &line_count);
    print_positions_and_commands(position, commande, line_count);

    // Create a dynamically allocated 2D array for map
    char **map = malloc(rows * sizeof(char *));
    for (int i = 0; i < rows; i++) {
        map[i] = malloc(cols * sizeof(char));
    }

    char orientations[MAX_LINES];
    initialize_map(rows, cols, map);
    add_random_hashes(rows, cols, map);
    place_drones(position, line_count, rows, cols, map);
    print_map(rows, cols, map);

    DroneData droneData;
    droneData.index = 0;
    droneData.rows = rows;
    droneData.cols = cols;
    droneData.map = map;
    droneData.position = position;
    droneData.orientations = orientations;
    droneData.commande = commande;
    droneData.drone_indices = drone_indices;

    pthread_t print_thread;
    pthread_create(&print_thread, NULL, print_map_thread, &droneData);

    move_all_drones(position, commande, line_count, rows, cols, map, orientations, drone_indices);

    pthread_cancel(print_thread);

    print_map(rows, cols, map);
    find_drones(rows, cols, map, drone_indices);

    // Don't forget to free the dynamically allocated memory when you're done with it
    for (int i = 0; i < rows; i++) {
        free(map[i]);
        free(drone_indices[i]);
    }
    free(map);
    free(drone_indices);
    fclose(file);
    return 0;
}
