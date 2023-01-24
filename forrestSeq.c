#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#define P 0 //probabilità p che un albero cresca in una cella vuota, 0 <= p <= 100
#define F 0 //probabilità f che un albero si incendii, 0 <= f <= 100
#define S 100 //numero di passi massimo
#define T "🌲"
#define E "💨"
#define B "🔥"

void inizializza_matrice(char *matrix, int m, int n);
void print_matrice(char* matrix, int m, int n);
void check_vicini(char* matrix, char* matrix2, int i, int j, int m, int n);

int main(int argc, char* argv[]){
    //variabili utili
    char *matrix, *matrix2, *temp;
    int m, n; //dimensioni matrice
    int ctr = 0; //per capire se matrice è tutta vuota
    double start, end;

    //inizializzo dimensioni
    if(argc >= 2){
        m = atoi(argv[1]);
        n = atoi(argv[2]);
    }else{
        printf("Inserire dimensioni matrice!\n");
        exit(0);
    }

    //alloco matrice
    matrix = (char*)malloc(m * n * sizeof(char));
    matrix2 = (char*)malloc(m * n * sizeof(char));

    //inizializzo matrice
    inizializza_matrice(matrix, m, n); 

    //stampa matrice
    if(m <= 15 && n <= 15){
        printf("Matrix: \n");
        print_matrice(matrix, m, n);
    }

    //calcolo tempo inizio
    start = MPI_Wtime();

    //avvio simulazione, S step
    for(int w = 0; w<S; w++){

        //scorro matrice
        for(int i = 0; i<m; i++){
            for(int j = 0; j<n; j++){
                if(matrix[(i*n) + j] == 'B'){ //se cella in fiamme diventa vuota
                    matrix2[(i*n) + j] = 'E';
                }else if(matrix[(i*n) + j] == 'E'){  //se cella vuota cresce albero con probabilità P
                    if((1 + rand() % 100) <= P){
                        matrix2[(i*n) + j] = 'T';
                    }else{
                        matrix2[(i*n) + j] = 'E';
                    }
                }else if(matrix[i*n +j] == 'T'){ //se cella con albero controlla vicini o brucia con probabilità F
                    check_vicini(matrix, matrix2, i, j, m, n);
                }

                if(matrix2[i*n + j] == 'E'){
                    ctr++;
                }
            }
        }

    
        //uso due matrici e le swappo per gestire S steps
        temp = matrix;
        matrix = matrix2;
        matrix2 = temp;

        if(ctr == m*n) break;
    }

    //calcolo tempo fine
    end = MPI_Wtime();

    
    //stampo risultati
    if(m <= 15 && n <= 15){
        printf("Matrix2: \n");
        print_matrice(matrix, m, n);
    }
    
    printf("Tempo totale: %1.4f secondi\n", end-start);

    free(matrix);
    free(matrix2);
    return 0;
}

void inizializza_matrice(char *matrix, int m, int n){
    for(int i = 0; i<m; i++){
        for(int j = 0; j<n; j++){
            int r = 1 + rand() % 100; //numero casuale tra 1 e 100
            if(r >= 85){ // 85 <= r <= 100
                matrix[i*n + j] = 'B';
            }else if(r <= 70){ // 1 <= r <= 70
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

void check_vicini(char* matrix, char* matrix2, int i, int j, int m, int n){ //implementa logica TOROIDALE per vedere i vicini
    //se almeno un vicino è B(urning), allora la cella diventa B(urning)
    //cella superiore
    if(matrix[(((i-1) % m + m)%m)*n + j] == 'B'){ //modulo
        matrix2[i*n + j] = 'B';
    }


    //cella inferiore
    if(matrix[(((i+1) % m + m)%m)*n + j] == 'B'){ //modulo
        matrix2[i*n + j] = 'B';
    }


    //cella sinistra
    if(matrix[(i*n) + (((j-1)%n + n)%n)] == 'B'){ //modulo
        matrix2[i*n +j] = 'B';
    }
    
    
    //cella destra
    if(matrix[(i*n) + (((j+1) % n + n)%n)] == 'B'){ //modulo
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