#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char memory[100][4]; // Memory (100 words, 4 characters each)
char R[4];           // Register
char IR[4];          // Instruction Register
int C;               // Condition Flag
int IC;              // Instruction Counter
int SI = 0;
FILE *outputFile;    // Output file pointer
char dataBuffer[100][40]; // Buffer to store data card lines (max 40 chars per line)
int dataLineIndex = 0; // Index to track current line in data buffer
int dataLineCount = 0; // Number of lines in data buffer

// Function to initialize memory and registers
void INIT() {
    memset(R, '*', sizeof(R));
    memset(IR, '*', sizeof(IR));
    C = 0;
    IC = 0;
    for (int i = 0; i < 100; i++) {
        memset(memory[i], '*', 4);  // Fill memory with ''
    }
    dataLineIndex = 0;
    dataLineCount = 0;
}

// Function to print memory with better formatting
void print_memory() {
    printf("Memory contents:\n");

    for (int i = 0; i < 100; i++) {
        printf("    -----------------\n");
        printf("%02d: ", i);  // Print index in 2-digit format

        // Check if the row is empty
        int empty = 1;
        for (int j = 0; j < 4; j++) {
            if (memory[i][j] != '*') {
                empty = 0;
                break;
            }
        }

        // Print * * * * if empty
        if (empty) {
            printf("| * | * | * | * |");
        } else {
            // Print actual values
            for (int j = 0; j < 4; j++) {
                printf("| %c ", memory[i][j]);
            }
            printf("|");
        }

        printf("\n");

        if ((i + 1) % 10 == 0)  // Space after every 10 rows
            printf("    -----------------\n");
            // printf("\n");
    }

}



// READ function with corrected memory alignment
void READ() {

    int startAddress = ((IR[2] - '0') * 10) + (IR[3] - '0');
    if (dataLineIndex >= dataLineCount) return; // No more data lines

    char *line = dataBuffer[dataLineIndex++]; // Get next data line
    int charIndex = 0;

    for (int row = startAddress; row < 100 && line[charIndex] != '\0' && line[charIndex] != '\n'; row++) {
        for (int col = 0; col < 4 && line[charIndex] != '\0' && line[charIndex] != '\n'; col++) {
            memory[row][col] = line[charIndex++];
        }
    }
}

// WRITE function with proper formatting
void WRITE() {
    int startAddress = ((IR[2] - '0') * 10) + (IR[3] - '0');

    for (int i = 0; i < 10 && (startAddress + i) < 100; i++) {
        for (int j = 0; j < 4; j++) {
            if (memory[startAddress + i][j] == '*') continue;
            fprintf(outputFile, "%c", memory[startAddress + i][j]);
        }
    }
    fprintf(outputFile, "\n"); // Ensures proper formatting

}

void TERMINATE() {
print_memory();
printf("Program Executed Successfully\n");
fprintf(outputFile, "--------------------\n--------------------\n");
}

// Function to execute instructions
void EUP() {
    while (IC < 100) {
        strncpy(IR, memory[IC], 4);
        IC++;

        if (strncmp(IR, "GD", 2) == 0) {
            // READ();
            SI = 1;
        } else if (strncmp(IR, "PD", 2) == 0) {
            // WRITE();
            SI = 2;
        } else if (strncmp(IR, "LR", 2) == 0) {
            int addr = ((IR[2] - '0') * 10) + (IR[3] - '0');
            strncpy(R, memory[addr], 4);
        } else if (strncmp(IR, "SR", 2) == 0) {
            int addr = ((IR[2] - '0') * 10) + (IR[3] - '0');
            strncpy(memory[addr], R, 4);
        } else if (strncmp(IR, "CR", 2) == 0) {
            int addr = ((IR[2] - '0') * 10) + (IR[3] - '0');
            C = (strncmp(R, memory[addr], 4) == 0) ? 1 : 0;
        } else if (strncmp(IR, "BT", 2) == 0) {
            if (C == 1) {
                IC = ((IR[2] - '0') * 10) + (IR[3] - '0');
            }
        } else if (IR[0] == 'H') {
            SI = 3;
            // break;


        }

        switch(SI) {
            case 1:  // GD (Read)
                READ();
                SI = 0;
                break;
            case 2:  // PD (Write)
                WRITE();
                SI = 0;
                break;
            case 3:  // H (Halt/Terminate)
                TERMINATE();
                return;
            default:
                break;

        }
    }
}


void loadModule(FILE *f) {
    char line[41]; // Buffer to read a line (max 40 chars + null)
    int memIndex = 0;

    while (fgets(line, sizeof(line), f)) {
        // Remove newline characters at the end if present
        line[strcspn(line, "\r\n")] = '\0';

        if (strncmp(line, "$AMJ", 4) == 0) {
            INIT();
            memIndex = 0;
        }
        else if (strncmp(line, "$DTA", 4) == 0) {
            // Start reading data after $DTA

          while (fgets(line, sizeof(line), f) && strncmp(line, "$END", 4) != 0) {
           line[strcspn(line, "\r\n")] = '\0'; // Trim newline characters
           strcpy(dataBuffer[dataLineCount++], line);
          }
            EUP();  // Start execution
        }
        else if (strncmp(line, "$END", 4) == 0) {
            break;
        }
        else {
            int len = strlen(line);
            int i = 0;
            while (i < len && memIndex < 100) {
                memset(memory[memIndex], '*', 4); // Initialize memory with ''
                strncpy(memory[memIndex], &line[i], 4);
                i += 4;
                memIndex++;
            }
        }
    }
}


// Main function
int main() {
    FILE *f = fopen("input.txt", "r");
    if (!f) {
        printf("Unable to open input file!\n");
        return 1;
    }

    outputFile = fopen("output.txt", "w");
    if (!outputFile) {
        printf("Unable to create output file!\n");
        fclose(f);
        return 1;
    }

    loadModule(f);
    fclose(f);
    fclose(outputFile);
    // print_memory();

    printf("Execution completed. Check 'output.txt' for results.\n");
    return 0;
}
