/* CSV Reader project
 * short description:
 *Programme reads temperature data from csv file, asks user to choose a room and prints all temperature readings,
 *printing result with one decimal and a hor. bar (each - equals 0.5C, and if value < 0 >30, prints X
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEN 256
#define MAX_ROOM_NAME_LEN 64
#define MAX_ROOMS 50
#define MAX_TEMP 200


//trims leading and trailing space
void trim(char *s) {
    char *start = s;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    if (start != s) {
        memmove(s, start, strlen(start)+1);
    }
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len-1])) {
        s[len-1] = '\0';
        len--;
    }
}

//finds index of room by name
int find_room(const char rooms[][MAX_ROOM_NAME_LEN], const int count, const char *name) {
    for (int i = 0; i < count; i++) {
        if (strcmp(rooms[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

//reads file and stores data
int read_csv(const char *filename, char rooms[][MAX_ROOM_NAME_LEN], int *room_count, float temps[][MAX_TEMP], int temp_counts[]) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Could not open file %s\n", filename);
        return 1;
    }
    char line[MAX_LEN];
    int lineCount = 0;

    while (fgets(line, sizeof(line), fp)) {
        lineCount++;
        line[strcspn(line, "\r\n")] = 0;

        if (strstr(line, "Temperature")&& strstr(line,"Room")) {
            continue;
        }
        if (strlen(line) < 3) {
            continue;
        }
        float temp;
        char room_name[MAX_ROOM_NAME_LEN];
        if (sscanf(line, " %f, %63[^\n]", &temp, room_name) !=2) {
            continue;
        }
        trim(room_name);
        if (room_name[0] == '\0') {
            continue;
        }
        int idx = find_room(rooms, *room_count, room_name);
        if (idx == -1) {
            if (*room_count >= MAX_ROOMS) {
                fprintf(stderr, "Error: too many rooms (max %d)\n", MAX_ROOMS);
                fclose(fp);
                return 2;
            }
            strncpy(rooms[*room_count], room_name, MAX_ROOM_NAME_LEN - 1);
            rooms[*room_count][MAX_ROOM_NAME_LEN - 1] = '\0';
            idx = (*room_count)++;
            temp_counts[idx] = 0;
        }

        if (temp_counts[idx] >= MAX_TEMP) {
            continue;
        }
        temps[idx][temp_counts[idx]++] = temp;
    }
    fclose(fp);
    return 0;
}

//prints temperature line
void print_temp_line(float temp) {
    printf("%5.1f", temp);
    if (temp < 0.0f || temp > 30.0f) {
        printf(" X\n");
        return;
    }
    int dashes = (int)(temp * 2.0f + 0.5f);
    putchar(' ');
    for (int i = 0; i < dashes; i++) {
        putchar('-');
    }
    putchar('\n');
}

int main(void) {
    char filename[128];
    char rooms[MAX_ROOMS][MAX_ROOM_NAME_LEN];
    float temps[MAX_ROOMS][MAX_TEMP];
    int temp_counts[MAX_ROOMS];
    int rooms_count = 0;

    for (int i = 0; i < MAX_ROOMS; i++) {
        temp_counts[i] = 0;
    }

    printf("Enter CSV filename: ");
    if (!fgets(filename, sizeof(filename), stdin)) {
        fprintf(stderr, "Could not read file %s\n", filename);
        return 1;
    }
    filename[strcspn(filename, "\r\n")] = '\0';
    trim(filename);

    if (filename[0] == '\0') {
        fprintf(stderr, "No filename provided\n");
        return 1;
    }

    int rc = read_csv(filename, rooms, &rooms_count, temps, temp_counts);
    if (rc != 0) {
        return rc;
    }

    if (rooms_count == 0) {
        fprintf(stderr, "No rooms found\n");
        return 1;
    }

    printf("Rooms available: \n");
    for (int i = 0; i < rooms_count; i++) {
        printf(" %s\n", rooms[i]);
    }

    char choice[MAX_ROOM_NAME_LEN];
    printf("\nEnter room name: ");
    if (!fgets(choice, sizeof(choice), stdin)) {
        fprintf(stderr, "Error reading room name\n");
        return 1;
    }
    choice[strcspn(choice, "\r\n")] = '\0';
    trim(choice);

    int idx = find_room(rooms, rooms_count, choice);
    if (idx == -1) {
        fprintf(stderr, "No room found\n");
        return 1;
    }

    printf("\n%s \n", rooms[idx]);
    for (int i = 0; i < temp_counts[idx]; i++) {
        print_temp_line(temps[idx][i]);
    }
    return 0;
}