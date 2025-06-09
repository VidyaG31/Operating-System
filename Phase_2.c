#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

// PCB Structure
typedef struct {
    int job_id;
    int TTL;    // Total Time Limit
    int TLL;    // Total Line Limit
    int TTC;    // Total Time Counter
    int LLC;    // Line Limit Counter
    int PTR;    // Page Table Register
    int IC;     // Instruction Counter
} PCB;

void execute();
void init();
void loadInstructionSetIntoMemory();
void checkErrors();

int getRandom(int min, int max) {
    int rd_num = rand() % (max - min + 1) + min;
    return rd_num;
}

char M[4][300], IR[4], R[4], BUFF[4][10], ttl[4], tll[4];
int m, IC, SI = 0, PCI=0, DCI=0, ptr, bp=0, c=1, temp=0, x=0, PI, errors;
int PTE, RA, ICR, ptl, toggle[300], v, pe;
FILE *input, *output;
char line[256], job[4], line2[256];
int randAd[30] = {14, 3, 25, 7, 0, 27, 9, 12, 21, 5, 17, 10, 28, 1, 23, 18, 19, 26, 2, 4, 15, 6, 13, 24, 8, 29, 22, 16, 11, 20};

PCB current_pcb; // Global PCB for the current process

int map(int va){
    int ra=0;
    for(int i=current_pcb.PTR; i<current_pcb.PTR+10; i++){
        if((M[0][i]-'0')*10 + (M[1][i] - '0') == va){
            ra = (M[2][i] - '0') *10 + (M[3][i] - '0');
            return ra;
        }
    }
    return ra;
}

void printMemory(){
    for(int i=0; i<300; i++){
        printf("%d  ", i);
        for(int j=0; j<4; j++){
            printf("%c ", M[j][i]);
        }
        printf("\n");
    }
}

void printBuffer(){
    for(int i=0; i<10; i++){
        printf("%d   ", i);
        for(int j=0; j<4; j++){
            printf("%c ", BUFF[j][i]);
        }
        printf("\n");
    }
}

void loadInstructionToBuffer(char*s, int x, int y){
    int ap=0;
    for(int i=40*(x-y); i<40+(40*(x-y)); i+=4){
        if(s[i] == '\n')break;
        BUFF[0][ap] = s[i];
        if(s[i] == 'H'){
            i-=3;
        }else{
            BUFF[1][ap] = s[i+1];
            BUFF[2][ap] = s[i+2];
            BUFF[3][ap] = s[i+3];
        }
        ap++;
    }
    loadInstructionSetIntoMemory();
    if(y > 0){
        loadInstructionToBuffer(s, x, y-1);
    }
}

void init(){
    // Initialize memory
    for(int i=0; i<4; i++){
        for(int j=0; j<300; j++){
            M[i][j] = '*';
            toggle[j] = 0;
        }
    }

    // Initialize registers and buffer
    for(int i=0; i<4; i++){
        IR[i] = '*';
        R[i] = '*';
        for(int j=0; j<10; j++){
            BUFF[j][i] = '*';
        }
    }

    // Initialize variables
    m=0, SI=0, IC=0, PI=0;
    ptr=1;
    errors = 0;
    ICR=0;
    ptl = 0;
    pe=0;

    // Initialize PCB
    current_pcb.TTC = 0;
    current_pcb.LLC = 0;
    current_pcb.IC = 0;
    current_pcb.PTR = randAd[0] * 10;
}

void MOS() {
    switch (SI) {
        case 1: {  // GD - Get Data
            if (BUFF[0][bp] == '*') {
                PI = 4;
                break;
            }

            int lf = 0, e = 0;
            int IC = (IR[2] - '0')*10 + (IR[3] - '0');
            int t = randAd[ptr - 1] * 10;

            while (lf == 0) {
                for (int i = 0; i < 4; i++) {
                    if (BUFF[i][bp] != '\n') {
                        M[i][t + e] = BUFF[i][bp];
                    } else {
                        lf = 1;
                    }
                }
                bp++;
                e++;
            }
            break;
        }

        case 2: {  // PD - Put Data
            int IC = (IR[2] - '0')*10 + (IR[3] - '0');
            int t = map(IC) * 10;

            output = fopen("output.txt", "a");

            for (int j = 0; j < 10; j++) {
                for (int i = 0; i < 4; i++) {
                    if (M[i][t + j] == '\n' || M[i][t + j] == '*') break;
                    fprintf(output, "%c", M[i][t + j]);
                }
            }
            fprintf(output, "\n");
            fclose(output);
            break;
        }

        case 3:  // H - Halt
            output = fopen("output.txt", "a");
            fprintf(output, "Job ID: %d\n", current_pcb.job_id);
            fprintf(output, "IC:%d IR:%s TTC:%d TTL:%d LLC:%d\n", ICR, IR, current_pcb.TTC, current_pcb.TTL, current_pcb.LLC);
            fprintf(output,"Program Terminated Normally\n\n");
            fclose(output);
            return;
    }

    SI = 0;  // Reset SI after handling
}

void execute(){
    checkErrors();
    if(errors == 1) return;

    for(int i=0; i<4; i++){
        IR[i] = M[i][m];
    }

    switch(IR[0]){
        case 'G':
            // GD
            if(IR[1] != 'D'){
                PI=1;
                break;
            }
            ICR++;
            current_pcb.TTC+=2;
            IC = (IR[2] - '0')*10 + (IR[3] - '0');
            toggle[IC] = 1;
            M[0][current_pcb.PTR+ptl] = IC/10 + '0';
            M[1][current_pcb.PTR+ptl] = IC%10 + '0';
            int t = randAd[ptr] * 10;
            M[2][current_pcb.PTR+ptl] = randAd[ptr]/10 + '0';
            M[3][current_pcb.PTR+ptl] = randAd[ptr]%10 + '0';
            ptr++;
            ptl++;
            if(IC<0 || IC>99){
                PI=2;
                break;
            }

            if(BUFF[0][bp] == '*'){
                PI=4;
                break;
            }

            SI = 1;
            MOS();
            break;

        case 'P':
            // PD
            if(IR[1] != 'D'){
                PI=1;
                break;
            }
            ICR++;
            current_pcb.TTC++;
            IC = (IR[2] - '0')*10 + (IR[3] - '0');
            t = map(IC)*10;
            if(toggle[IC] == 0){
                PI=3;
                break;
            }
            if(IC<0 || IC>99){
                PI=2;
                break;
            }

            SI = 2;
            MOS();
            break;

        case 'L':
            // LR
            if(IR[1] != 'R'){
                PI=1;
                break;
            }
            ICR++;
            current_pcb.TTC++;
            IC = (IR[2] - '0')*10 + (IR[3] - '0');
            if(toggle[IC] == 0){
                PI=3;
                break;
            }
            t = map(IC/10 * 10)*10 + IC%10;
            if(IC<0 || IC>99){
                PI=2;
                break;
            }
            for(int i=0; i<4; i++){
                R[i] = M[i][t];
            }
            break;

        case 'S':
            // SR
            if(IR[1] != 'R'){
                PI=1;
                break;
            }
            ICR++;
            current_pcb.TTC+=2;
            IC = (IR[2] - '0')*10 + (IR[3] - '0');
            toggle[IC] = 1;
            if(IC<0 || IC>99){
                PI=2;
                break;
            }
            t = map(IC/10 * 10)*10 + IC%10;
            for(int i=0; i<4; i++){
                M[i][t] = R[i];
            }
            break;

        case 'C':
            // CR
            if(IR[1] != 'R'){
                PI=1;
                break;
            }
            ICR++;
            current_pcb.TTC++;
            IC = (IR[2] - '0')*10 + (IR[3] - '0');
            if(toggle[IC] == 0){
                PI=3;
                break;
            }
            t = map(IC/10 * 10)*10 + IC%10;
            if(IC<0 || IC>99){
                PI=2;
                break;
            }
            for(int i=0; i<4; i++){
                if(M[i][t] != R[i]){
                    c = 0;
                }
            }
            break;

        case 'B':
            // BT
            if(IR[1] != 'T'){
                PI=1;
                break;
            }
            ICR++;
            current_pcb.TTC++;
            IC = (IR[2] - '0')*10 + (IR[3] - '0');
            if(toggle[IC] == 0){
                PI=3;
                break;
            }
            if(IC<0 || IC>99){
                PI=2;
                break;
            }
            if(c == 1){
                m = IC-1;
            }
            c=1;
            break;

        case 'H':
            ICR++;
            current_pcb.TTC++;
            SI = 3;
            MOS();
            return;
            break;

        default:
            PI=1;
            break;
    }
    m++;
    if(v<10){
        v++;
        execute();
    }
}

void read_file(){
    input = fopen("input.txt", "r");
    while (fgets(line, sizeof(line), input) != NULL) {
        if(PCI == 1){
            x=0;
            for(int i=0; i<strlen(line); i++){
                if(line[i] != ' '){
                    line2[x] = line[i];
                    x++;
                }
            }

            loadInstructionToBuffer(line2, strlen(line2)/40, strlen(line2)/40);
            PCI=0;
        }

        if(DCI == 1){
            if(line[0] == '$'){
                DCI = 0;
                bp=0;
                m=((M[2][current_pcb.PTR] - '0') * 10 + (M[3][current_pcb.PTR] - '0')) * 10;
                v=0;
                execute();
            }else{
                int j=0;
                for(int i=0; i<strlen(line); i++){
                    if(j>3){
                        j=0;
                        bp++;
                    }
                    BUFF[j][bp] = line[i];
                    j++;
                }
                bp++;
            }
        }

        if(line[0] == '$'){
            switch(line[1]){
                case 'A':
                    init();
                    // Parse job information
                    for(int i=0; i<4; i++){
                        job[i] = line[4+i];
                        ttl[i] = line[8+i];
                        tll[i] = line[12+i];
                    }

                    // Convert job ID
                    current_pcb.job_id = 0;
                    for(int i=0; i<4; i++){
                        if(job[i] >= '0' && job[i] <= '9'){
                            current_pcb.job_id *= 10;
                            current_pcb.job_id += job[i] - '0';
                        }
                    }

                    // Convert TTL and TLL
                    current_pcb.TTL = 0;
                    current_pcb.TLL = 0;
                    for(int i=0; i<4; i++){
                        current_pcb.TTL *= 10;
                        current_pcb.TTL += ttl[i] - '0';
                        current_pcb.TLL *= 10;
                        current_pcb.TLL += tll[i] - '0';
                    }

                    PCI=1;
                    break;

                case 'D':
                    DCI=1;
                    bp=0;
                    break;

                default:
                    output = fopen("output.txt", "a");
                    fprintf(output, "\n\n");
                    fclose(output);
                    PCI=0, DCI=0;
            }
        }
    }
}

void loadInstructionSetIntoMemory(){
    m=randAd[ptr] * 10;
    M[2][current_pcb.PTR+ptl] = (randAd[ptr]/10) + '0';
    M[3][current_pcb.PTR+ptl] = (randAd[ptr]%10) + '0';
    ptr++;
    ptl++;
    int y=0;

    while(BUFF[0][y] != '*' && y<10){
        M[0][m] = BUFF[0][y];
        if(BUFF[0][y] == 'P' && BUFF[1][y] == 'D') current_pcb.LLC++;
        if(BUFF[0][y] != 'H'){
            M[1][m] = BUFF[1][y];
            M[2][m] = BUFF[2][y];
            M[3][m] = BUFF[3][y];
        }
        y++;
        m++;
    }

    for(int i=0; i<10; i++){
        for(int j=0; j<4; j++){
            BUFF[j][i] = '*';
        }
    }
    m=0;
}

void checkErrors(){
    if(current_pcb.TTC+1 > current_pcb.TTL){
        errors = 1;
        output = fopen("output.txt", "a");
        fprintf(output, "Job ID: %d\n", current_pcb.job_id);
        fprintf(output, "IC:%d IR:%s TTC:%d TTL:%d LLC:%d\n", ICR, IR, current_pcb.TTC, current_pcb.TTL, current_pcb.LLC);
        fprintf(output, "Program terminated abnormally because of TIME LIMIT EXCEEDED error\n\n");
        fclose(output);
    }
    if(current_pcb.LLC > current_pcb.TLL){
        errors = 1;
        output = fopen("output.txt", "a");
        fprintf(output, "Job ID: %d\n", current_pcb.job_id);
        fprintf(output, "IC:%d IR:%s TTC:%d TTL:%d LLC:%d\n", ICR, IR, current_pcb.TTC, current_pcb.TTL, current_pcb.LLC);
        fprintf(output, "Program terminated abnormally because of LINE LIMIT EXCEEDED error\n\n");
        fclose(output);
    }
    if(PI>0){
        errors = 1;
        output = fopen("output.txt", "a");
        fprintf(output, "Job ID: %d\n", current_pcb.job_id);
        fprintf(output, "IC:%d IR:%s TTC:%d TTL:%d LLC:%d\n", ICR, IR, current_pcb.TTC, current_pcb.TTL, current_pcb.LLC);
        if(PI==1){
            fprintf(output, "Program terminated abnormally because of OPCODE error\n\n");
        }
        if(PI==2){
            fprintf(output, "Program terminated abnormally because of OPERAND error\n\n");
        }
        if(PI==3){
            fprintf(output, "Program terminated abnormally because of INVALID PAGE FAULT error\n\n");
        }
        if(PI==4){
            fprintf(output, "Program terminated abnormally because of OUT OF DATA error\n\n");
        }
        fclose(output);
    }
}

int main(){
    read_file();
    printMemory();
    fclose(input);
    return 0;
}
