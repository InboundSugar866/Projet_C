#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define LEPORT 2020
#define BUFFER_SIZE 1000024
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

char* print_map(int rows, int cols, char map[rows][cols]) {
    // Calculate the size of the output string
    // Each cell of the map contributes 1 character
    // Each row contributes an additional 1 character for the newline
    // The top and bottom borders contribute 'cols' characters each
    // Add 1 for the null terminator
    int size = rows * (cols + 1) + 2 * cols + 1;

    // Allocate memory for the output string
    char* result = malloc(size);
    if (result == NULL) {
        printf("Failed to allocate memory\n");
        return NULL;
    }

    // Start with an empty string
    result[0] = '\0';

    // Print the top border
    for(int i = 0; i < cols; i++) {
        strcat(result, "=");
    }
    strcat(result, "\n");

    // Print the map
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            char str[2] = {map[i][j], '\0'};  // Convert the character to a string
            strcat(result, str);  // Append the character to the result
        }
        strcat(result, "\n");  // Append a newline after each row
    }

    // Print the bottom border
    for(int i = 0; i < cols; i++) {
        strcat(result, "=");
    }
    strcat(result, "\n");

    return result;
}


char* print_positions_and_commands(char position[MAX_LINES][MAX_LINE_LENGTH], char commande[MAX_LINES][MAX_LINE_LENGTH], int line_count) {
    // Calculate the size of the output string
    // Each position and command contributes up to 256 characters (including the newline)
    // Add 1 for the null terminator
    int size = line_count * 2 * 256 + 1;

    // Allocate memory for the output string
    char* result = malloc(size);
    if (result == NULL) {
        printf("Failed to allocate memory\n");
        return NULL;
    }

    // Start with an empty string
    result[0] = '\0';

    for(int i = 0; i < line_count-1; i++) {
        // Append the position and command to the result
        char str[512];  // Temporary string to hold the position and command
        sprintf(str, "Position drone : %s\nCommande : %s\n", position[i], commande[i]);
        strcat(result, str);
    }

    return result;
}


char* find_drones(int rows, int cols, char map[rows][cols], int drone_indices[rows][cols]) {
    // Allocate memory for the output string
    // Each drone contributes up to 50 characters (including the newline)
    // Add 1 for the null terminator
    char* result = malloc(rows * cols * 50 + 1);
    if (result == NULL) {
        printf("Failed to allocate memory\n");
        return NULL;
    }

    // Start with an empty string
    result[0] = '\0';

    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            if(map[i][j] == '^' || map[i][j] == 'v' || map[i][j] == '>' || map[i][j] == '<') {
                char orientation;
                if(map[i][j] == '^') orientation = 'N';
                else if(map[i][j] == 'v') orientation = 'S';
                else if(map[i][j] == '>') orientation = 'E';
                else if(map[i][j] == '<') orientation = 'W';

                // Append the drone's position to the result
                char str[50];
                sprintf(str, "Position finale du drone %d: %d %d %c\n", drone_indices[i][j], i, j, orientation);
                strcat(result, str);
            }
        }
    }

    return result;
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
void move_all_drones(char position[MAX_LINES][MAX_LINE_LENGTH], char commande[MAX_LINES][MAX_LINE_LENGTH], int line_count, int rows, int cols, char map[rows][cols], char orientations[MAX_LINES], int drone_indices[rows][cols]) {
    int x, y;
    char orientation;
    for(int i = 0; i < line_count-1; i++) {
        sscanf(position[i], "%d %d %c", &x, &y, &orientation);
        execute_commands(&x, &y, &orientation, commande[i], rows, cols, map, position, orientations, i, drone_indices);
    }
}

char* process_map(int rows, int cols, char map[rows][cols], char position[MAX_LINES][MAX_LINE_LENGTH], char commande[MAX_LINES][MAX_LINE_LENGTH], int line_count, char orientations[MAX_LINES], int drone_indices[rows][cols]) {
    // Call the modified functions and concatenate their results
	char* print_pos_com = print_positions_and_commands(position, commande, line_count);
    char* print_map1 = print_map(rows, cols, map);
    move_all_drones(position, commande, line_count, rows, cols, map, orientations, drone_indices);
    char* print_map2 = print_map(rows, cols, map);
    char* find_drones_result = find_drones(rows, cols, map, drone_indices);

    // Calculate the total length of the result
    int total_length = strlen(print_pos_com) + strlen(print_map1) + strlen(print_map2) + strlen(find_drones_result) + 1;

    // Allocate memory for the result
    char* result = malloc(total_length);
    if (result == NULL) {
        printf("Failed to allocate memory\n");
        return NULL;
    }

    // Concatenate the results
	strcpy(result,print_pos_com);
    strcat(result, print_map1);
    strcat(result, print_map2);
    strcat(result, find_drones_result);

    // Free the memory allocated for the intermediate results
    free(print_map1);
    free(print_map2);
    free(find_drones_result);

    return result;
}


int main() {
    int s = 0, taille, bd, error_number, lg = 40;
    char msg[BUFFER_SIZE];
    char msg1[40] = "message bien recu ";
    //char msg2[40] = "blabla";
    struct sockaddr_in *padin; //pointeur adresse internet locale
    struct sockaddr_in *p_exp; //pointeur adresse internet expediteur (recuperée de l'entete paquet UDP recu)
    
    s = socket(AF_INET, SOCK_DGRAM, AF_UNSPEC);
    if (s == -1) {
        perror("erreur creation socket");
        exit(EXIT_FAILURE);
    }
    printf("le socket est identifie par : %d \n", s);

    taille = sizeof(struct sockaddr_in);
    padin = (struct sockaddr_in *)(malloc(taille));
    bzero((char *)padin, taille);

    padin->sin_family = AF_INET;
    padin->sin_port = htons(LEPORT);

    bd = bind(s, (struct sockaddr *)padin, taille);
    if (bd == -1) {
        perror("Erreur d'attachement");
        exit(EXIT_FAILURE);
    }

    p_exp = (struct sockaddr_in *)(malloc(sizeof(struct sockaddr_in)));
    socklen_t p_lgexp = sizeof(struct sockaddr_in);

    while (1) {
        bd = recvfrom(s, msg, BUFFER_SIZE, 0, (struct sockaddr *)p_exp, &p_lgexp);
        if (bd == -1) {
            perror("Erreur receive");
            exit(EXIT_FAILURE);
        }
        msg[bd] = '\0';
        printf("Client :\n %s\n", msg);


        //printf("Quel est votre message ?  ");
        //fgets(msg2, 40, stdin);
		
		int rows, cols, line_count;
		
        FILE *file = fopen(msg, "r");
        if (file == NULL) {
            printf("Could not open file\n");
            return 1;
        }

        fscanf(file, "%d %d", &cols, &rows); // Use file pointer instead of msg

        char position[MAX_LINES][MAX_LINE_LENGTH];
        char commande[MAX_LINES][MAX_LINE_LENGTH];
        line_count = 0; // Reset line_count to 0 for each file
        int drone_indices[rows][cols];

        read_file(file, position, commande, &line_count);

        char map[rows][cols];
        char orientations[MAX_LINES];
        initialize_map(rows, cols, map);
        add_random_hashes(rows, cols, map);
        place_drones(position, line_count, rows, cols, map);
        
        // Call the process_map function
        char* result = process_map(rows, cols, map, position, commande, line_count, orientations, drone_indices);
		
		// Allocate memory for msg2
		char* msg2 = malloc(strlen(result) + 1);
		if (msg2 == NULL) {
			printf("Failed to allocate memory\n");
			return 1;
		}

		// Copy the string from result to msg2
		strncpy(msg2, result, strlen(result));

		// Don't forget to null-terminate msg2
		msg2[strlen(result)] = '\0';

		// Don't forget to free the memory after using the result
		free(result);

		fclose(file);

        bd = sendto(s, msg2, strlen(msg2), 0, (struct sockaddr *)p_exp, sizeof(*p_exp));
        if (bd == -1) {
            perror("Erreur send");
            exit(EXIT_FAILURE);
        }
		free(msg2);
        
    }

    // Free allocated memory
    free(padin);
    free(p_exp);

    
    close(s);
    return 0;
}