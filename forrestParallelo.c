#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#define T "🌲"
#define E "💨"
#define B "🔥"
#define S 50  //numero di iterazioni totali algoritmo
#define P 25  //albero cresce in cella vuota con probabilità P
#define F 20  //albero brucia con probabilità F se nessun vicino sta bruciando

void inizializza_matrice(char *matrix, int m, int n);
void print_matrice(char* matrix, int m, int n);
int mod(int a, int b);
void check_vicini(char* matrix, char* matrix2, int i, int j, int m, int n);
void check_bordi(char* submatrix, char* submatrix2, char* rigaUp, char* rigaDown, int i, int j, int nRighe, int n);


int main(int argc, char* argv[]){
    //rank processo, numero processi
    int me, np;

    //contatore per capire se matrice è vuota
    int ctr = 0; 

    //var per scatterv
    int r, sum = 0;
    int *sendcounts, *offset;

    //matrici
    char *MAT, *MAT2;

    //dimensioni matrici e controllo input
    int m, n;
    if(argc >= 2){
        m = atoi(argv[1]);
        n = atoi(argv[2]);
    }else{
        printf("Inserire dimensioni matrice!\n");
        exit(0);
    }

    //MPI
    MPI_Request request[2];
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &me);

    //----------processo 0-----------
    //alloca, inizializza e stampa matrice
    if(me == 0){
        //alloca
        MAT = (char*)malloc(m * n * sizeof(char));
        MAT2 = (char*)malloc(m * n * sizeof(char));

        //inizializza
        inizializza_matrice(MAT, m, n);

        //stampa matrice
        if(m <= 15 && n <= 15){
            printf("MATRICE PARTENZA MAT: \n");
            print_matrice(MAT, m, n);
        }

    }

    //----------altri processi---------
    //divido matrice per numero di processi
    r = m % np; //resto divisione righe per numero processi
    sendcounts = (int*)malloc(np * sizeof(int));
    offset = (int*)malloc(np * sizeof(int));

    //inizializzo sendcounts e offset per la scatterv
    for(int i = 0; i<np; i++){
        sendcounts[i] = m/np;
        if(r > 0){
            sendcounts[i]++;
            r--;
        }
        sendcounts[i] *= n;

        offset[i] = sum;
        sum += sendcounts[i];
    }

    /*
    //stampa sendcounts e offset per ogni processo
    if(me == 0){
        for (int i = 0; i < np; i++) {
            printf("sendcounts[%d] = %d\toffset[%d] = %d\n", i, sendcounts[i], i, offset[i]);
        }
    }
    */

    //dichiara SUBMAT e SUBMAT2 locale
    char *SUBMAT, *SUBMAT2, *TEMP;
    int nRighe = sendcounts[me]/n;
    //alloco SUBMAT e SUBMAT2 locale
    SUBMAT = (char*)malloc(sendcounts[me] * sizeof(char));
    SUBMAT2 = (char*)malloc(sendcounts[me] * sizeof(char));

    //dichiara e alloca 2 array corrispondenti alle due righe da ricevere(A_up e A_down)
    char *A_up, *A_down;
    A_up = (char*)malloc(n * sizeof(char));
    A_down = (char*)malloc(n * sizeof(char));

    //ricevi sottomatrice locale
    MPI_Scatterv(MAT, sendcounts, offset, MPI_CHAR, SUBMAT, sendcounts[me], MPI_CHAR, 0, MPI_COMM_WORLD);

    /*
    //stampa sottomatrice ricevuta
    printf("PROCESSO %d, SUBMAT: \n", me);
    print_matrice(SUBMAT, nRighe, n);
    */

    for(int i = 0; i<S; i++){
        //se ho solo una riga invio quella a p-1%np e p+1%np
        if(nRighe == 1){
            MPI_Isend(SUBMAT, n, MPI_CHAR, mod(me - 1, np), 0, MPI_COMM_WORLD, &request[0]);
            MPI_Irecv(A_down, n, MPI_CHAR, mod(me + 1, np), 0, MPI_COMM_WORLD, &request[0]);
            MPI_Isend(SUBMAT, n, MPI_CHAR, mod(me + 1, np), 0, MPI_COMM_WORLD, &request[1]);   
            MPI_Irecv(A_up, n, MPI_CHAR, mod(me - 1, np), 0, MPI_COMM_WORLD, &request[1]);
        }else{ //altrimenti se ho multiple righe
            //invia riga superiore matrice a p-1%np
            MPI_Isend(SUBMAT, n, MPI_CHAR, mod(me - 1, np), 0, MPI_COMM_WORLD, &request[0]);
            //ricevi riga inferiore da processo p+1%np memorizzandola in A_down
            MPI_Irecv(A_down, n, MPI_CHAR, mod(me + 1, np), 0, MPI_COMM_WORLD, &request[0]);
            //invia riga inferiore matrice a p+1%np
            MPI_Isend(&SUBMAT[sendcounts[me] - n], n, MPI_CHAR, mod(me + 1, np), 0, MPI_COMM_WORLD, &request[1]);
            //ricevi riga superiore da processo p-1%np memorizzandola in A_up
            MPI_Irecv(A_up, n, MPI_CHAR, mod(me - 1, np), 0, MPI_COMM_WORLD, &request[1]);
        } 

        
        //nel frattempo lavoro su righe interne, ammesso che la SUBMAT abbia almeno 3 righe, altrimenti siamo ai bordi
        if(nRighe >= 3){
            for(int i = 1; i <= nRighe-2; i++){
                for(int j = 0; j<n; j++){
                    if(SUBMAT[i * n + j] == 'B'){
                        SUBMAT2[i * n + j] = 'E';
                    }else if(SUBMAT[i * n + j] == 'E'){
                        if((1 + rand() % 100) <= P){
                            SUBMAT2[(i*n) + j] = 'T';
                        }else{
                            SUBMAT2[(i*n) + j] = 'E';
                        }
                    }else if(SUBMAT[i * n + j] == 'T'){
                        check_vicini(SUBMAT, SUBMAT2, i, j, nRighe, n);
                    }

                    if(SUBMAT2[i * n + j] == 'E'){
                        ctr++;
                    }
                }
            }
        }
        

        //Quando le send e le receive sono completate lavoro sui bordi
        MPI_Waitall(2, request, MPI_STATUSES_IGNORE);

        if(nRighe >= 1){
            //lavoro su bordo prima riga usando array ricevuto A_up
            for(int j = 0; j<n; j++){
                if(SUBMAT[0 * n + j] == 'B'){
                    SUBMAT2[0 * n + j] = 'E';
                }else if(SUBMAT[0 * n + j] == 'E'){
                    if((1 + rand() % 100) <= P){
                        SUBMAT2[0 * n + j] = 'T';
                    }else{
                        SUBMAT2[0 * n + j] = 'E';
                    }
                }else if(SUBMAT[0 * n + j] == 'T'){
                    check_bordi(SUBMAT, SUBMAT2, A_up, SUBMAT, 0, j, nRighe, n);
                }

                if(SUBMAT2[0 * n + j] == 'E'){
                    ctr++;
                }
            }

            //lavoro su bordo ultima riga usando array ricevuto A_down
            for(int j = 0; j<n; j++){
                if(SUBMAT[(nRighe - 1) * n + j] == 'B'){
                    SUBMAT2[(nRighe - 1) * n + j] = 'E';
                }else if(SUBMAT[(nRighe - 1) * n + j] == 'E'){
                    if((1 + rand() % 100) <= P){
                        SUBMAT2[(nRighe - 1) * n + j] = 'T';
                    }else{
                        SUBMAT2[(nRighe - 1) * n + j] = 'E';
                    }
                }else if(SUBMAT[(nRighe - 1) * n + j] == 'T'){
                    check_bordi(SUBMAT, SUBMAT2, SUBMAT, A_down, (nRighe-1), j, nRighe, n);
                }

                if(SUBMAT2[(nRighe - 1) * n + j] == 'E'){
                    ctr++;
                }
            }
            
        }

        /*
        //stampo array up e down ricevuti
        printf("Processo %d\n", me);
        printf("Riga UP \n");
        print_matrice(A_up, 1, n);
        printf("Riga DOWN \n");
        print_matrice(A_down, 1, n);
        
        printf("SUBMAT2 processo %d\n", me);
        print_matrice(SUBMAT2, nRighe, n);
        */
    }

    //gatherv delle sottomatrici al processo 0
    MPI_Gatherv(SUBMAT2, sendcounts[me], MPI_CHAR, MAT2, sendcounts, offset, MPI_CHAR, 0, MPI_COMM_WORLD);

    //stampa matrice totale p0
    if(me == 0){
        if(m <= 15 && n <= 15){
            printf("Matrice finale MAT2: \n");
            print_matrice(MAT2, m, n);
        }
        //printf("Tempo totale %1.4f secondi\n", end-start);
    }

    MPI_Finalize();
    return 0;
}

void inizializza_matrice(char *matrix, int m, int n){
    for(int i = 0; i<m; i++){
        for(int j = 0; j<n; j++){
            int r = 1 + rand() % 100; //numero casuale tra 1 e 100
            if(r >= 85){ // 85 <= r <= 100
                matrix[i*n + j] = 'B';
            }else if(r <= 60){ // 1 <= r <= 60
                matrix[i*n + j] = 'T';
            }else{ // 71 <= r <= 84
                matrix[i*n + j] = 'E';
            }
        }
    }
}

void print_matrice(char* matrice, int m, int n){
    for(int i = 0; i<m; i++){
        for(int j = 0; j<n; j++){
            if(matrice[i*n + j] == 'T'){
                printf("%s", T);
            }else if(matrice[i*n +j] == 'B'){
                printf("%s", B);
            }else{
                printf("%s", E);
            }
        }
        printf("\n");
    }
    printf("\n");
}

int mod(int a, int b){
    return (a % b + b) % b;
}

void check_vicini(char* matrix, char* matrix2, int i, int j, int m, int n){ //implementa logica per vedere i vicini
    //se almeno un vicino è B(urning), allora la cella diventa B(urning)
    //cella superiore
    if(matrix[(i-1)*n + j] == 'B'){ 
        matrix2[i*n + j] = 'B';
    }


    //cella inferiore
    if(matrix[(i+1)*n + j] == 'B'){ 
        matrix2[i*n + j] = 'B';
    }


    //cella sinistra
    if(matrix[(i*n) + mod((j-1), n)] == 'B'){ //modulo
        matrix2[i*n +j] = 'B';
    }
    
    
    //cella destra
    if(matrix[(i*n) + mod((j+1), n)] == 'B'){ //modulo
        matrix2[i*n +j] = 'B';
    }

    //se nessun vicino è in stato B, cella attuale diventa B con probabilità F
    int r = 1+rand()%100;
    if(r<=F || matrix2[i*n + j] == 'B'){
        matrix2[i*n + j] = 'B';
    }else{
        matrix2[i*n + j] = 'T';
    }
}

void check_bordi(char* submatrix, char* submatrix2, char* rigaUp, char* rigaDown, int i, int j, int nRighe, int n){
    //se almeno un vicino è B(urning), allora la cella diventa B(urning)
    //cella superiore
    if(i == 0){ //se considero la riga 0 di SUBMAT, la mia riga superiore è A_up
        if(rigaUp[i * n + j] == 'B'){
            submatrix2[i * n + j] = 'B';
        }
    }else if(i == (nRighe - 1)){ //se considero riga ultima di SUBMAT, la mia riga superiore è la riga i-1 di SUBMAT
        if(rigaUp[(i-1) * n + j] == 'B'){
            submatrix2[i * n + j] = 'B';
        }
    }


    //cella inferiore
    if(i == (nRighe - 1)){ //se considero riga ultima di SUBMAT, la mia riga inferiore è A_down
        if(rigaDown[i * n + j] == 'B'){ 
            submatrix2[i * n + j] = 'B';
        }
    }else if(i == 0){ //se considero riga 0 di SUBMAT, la sua riga inferiore è la riga i+1 di SUBMAT
        if(rigaDown[(i+1) * n + j] == 'B'){ 
            submatrix2[i * n + j] = 'B';
        }
    }
    

    //cella sinistra
    if(submatrix[(i*n) + mod((j-1), n)] == 'B'){ //modulo
        submatrix2[i*n +j] = 'B';
    }
    
    //cella destra
    if(submatrix[(i*n) + mod((j+1), n)] == 'B'){ //modulo
        submatrix2[i*n +j] = 'B';
    }

    //se nessun vicino è in stato B, cella attuale diventa B con probabilità F
    int r = 1+rand()%100;
    if(r<=F || submatrix2[i*n + j] == 'B'){
        submatrix2[i*n + j] = 'B';
    }else{
        submatrix2[i*n + j] = 'T';
    }

}