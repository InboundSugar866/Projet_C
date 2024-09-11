#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 256

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
void initialize_map(int rows, int cols, char map[rows][cols]) {
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            map[i][j] = ' ';
        }
    }
}

// function to add 10 '#' randomly to the map
void add_random_hashes(int rows, int cols, char map[rows][cols]) {
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
void place_drones(char position[MAX_LINES][MAX_LINE_LENGTH], int line_count, int rows, int cols, char map[rows][cols]) {
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
void print_map(int rows, int cols, char map[rows][cols]) {
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

// function to print the postions and commands
void print_positions_and_commands(char position[MAX_LINES][MAX_LINE_LENGTH], char commande[MAX_LINES][MAX_LINE_LENGTH], int line_count) {
    for(int i = 0; i < line_count-1; i++) {
        printf("Position drone : %s\n", position[i]);
        printf("Commande : %s\n",commande[i]);
    }
}

// When printing the final positions, use the drone_indices array to get the drone index
void find_drones(int rows, int cols, char map[rows][cols], int drone_indices[rows][cols]) {
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
void move_drone(int *x, int *y, char *orientation, char command, int rows, int cols, char map[rows][cols], int drone_indices[rows][cols], int drone_index) {
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
void execute_commands(int *x, int *y, char *orientation, char *commands, int rows, int cols, char map[rows][cols], char position[MAX_LINES][MAX_LINE_LENGTH], char orientations[MAX_LINES], int index, int drone_indices[rows][cols]) {
    for (int i = 0; commands[i] != '\0'; i++) {
        if (commands[i] == 'L' || commands[i] == 'R' || commands[i] == 'M' || commands[i] == 'B') {
            move_drone(x, y, orientation, commands[i], rows, cols, map, drone_indices, index);
            sprintf(position[index], "%d %d", *x, *y); // Update the drone's position in the 'position' array
            orientations[index] = *orientation; // Update the drone's orientation
        }
    }
}

// Function to move all drones according to their command sequences
void move_all_drones(char position[MAX_LINES][MAX_LINE_LENGTH], char commande[MAX_LINES][MAX_LINE_LENGTH], int line_count, int rows, int cols, char map[rows][cols], char orientations[MAX_LINES], int drone_indices[rows][cols]){
    int x, y;
    char orientation;
    for(int i = 0; i < line_count-1; i++) {
        sscanf(position[i], "%d %d %c", &x, &y, &orientation);
        execute_commands(&x, &y, &orientation, commande[i], rows, cols, map, position, orientations, i, drone_indices);
    }
}

int main() {
    FILE *file = fopen("version1.txt", "r");
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
	int drone_indices[rows][cols];

    read_file(file, position, commande, &line_count);
    print_positions_and_commands(position, commande, line_count);

	char map[rows][cols];
	char orientations[MAX_LINES];
	initialize_map(rows, cols, map);
	add_random_hashes(rows, cols, map);
	place_drones(position, line_count, rows, cols, map);
	print_map(rows, cols, map);
	move_all_drones(position, commande, line_count, rows, cols, map, orientations, drone_indices);
	print_map(rows, cols, map);
	find_drones(rows, cols, map, drone_indices);

    fclose(file);
    return 0;
}