# Forest-fire-problem-MPI 

## Introduzione
In matematica applicata il forest-fire model è un automa cellulare su una griglia di NxN celle consistente nella simulazione di un incendio di una foresta.
Lo stato di ogni cella della matrice può essere: vuota (E), albero (T), brucia (B).
Secondo il modello di Drossel e Schwabl (1992), ci sono 4 regole eseguite simultaneamente:
 1. Una cella in fiamme diventa una cella vuota
 2. Un albero si incendia se almeno un vicino è in fiamme
 3. Un albero si infiamma con probabilità F se nessun vicino è in fiamme
 4. Una cella vuota si riempie con un albero con probabilità P  
 
La simulazione termina se si raggiunge un numero massimo di iterazioni (steps) S, oppure se tutte le celle sono vuote.  

![](https://github.com/gaetanodigenio/Forest-fire-problem-MPI/blob/main/img/Forest_fire_model.gif)

## Descrizione breve della soluzione
Si ha un processo master che crea, inizializza e divide matrice equamente tra gli np processori.  
Ogni processore si ritroverà con un numero di righe pari a m/np (eventualmente si assegna una riga in più), dove m è il numero di righe ed np il numero di processori.  
La matrice segue una logica toroidale, sia verticalmente (la prima riga e l'ultima riga sono considerate vicine), sia orizzontalmente (la prima colonna e l'ultima colonna sono considerate vicine).   
Inoltre per mantenere le informazioni di scambio ai bordi si useranno 2 array ausiliari, in cui uno mantiene l'ultima riga del processo precedente, l'altro mantiene la prima riga del processo successivo.  
Si crea infine un meccanismo per capire quando la matrice è vuota e quindi interrompere l'esecuzione e si utilizzano 2 matrici alternandole ad ogni fine di passo di esecuzione per risparmiare spazio in memoria.  
Nello schema seguente si mostra la suddivisione della matrice e gli array A_up e A_down per ogni processo:  

<img src="https://github.com/gaetanodigenio/Forest-fire-problem-MPI/blob/main/img/descrizione.jpg" width="680" >

### Vicini
I vicini di un nodo generico sono raffigurati di seguito:  
<img src="https://github.com/gaetanodigenio/Forest-fire-problem-MPI/blob/main/img/vicini-generics.png" width="200" >

I vicini di un nodo ai bordi (il nodo azzurro) sono raffigurati di seguito:  
<img src="https://github.com/gaetanodigenio/Forest-fire-problem-MPI/blob/main/img/vicini%20bordi.png" width="300" >



## Dettagli implementativi
### Divisione della matrice
Inizialmente si definiscono le probabilità P, F ed il numero di iterazioni S:
``` 
#define S 50  //numero di iterazioni totali algoritmo
#define P 10  //albero cresce in cella vuota con probabilità P
#define F 10 //albero brucia con probabilità F se nessun vicino sta bruciando 
```

Si calcola quante righe dare ad ogni processo (sendcounts e offset utili per la funzione MPI_Scatterv).
Ogni processore riceve m/np righe (+1 eventualmente), dove m è il numero di righe, np il numero di processori:
```
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
```
Ogni processore riceverà quindi m/np righe, ed in più allocherà 2 array: A_up manterrà i valori dell'ultima riga del processo precedente, A_down manterrà i valori della prima riga del processo successivo.  
```
    //dichiara SUBMAT e SUBMAT2 locale
    char *SUBMAT, *SUBMAT2, *TEMP;
    int nRighe = sendcounts[me]/n;
    //alloco SUBMAT e SUBMAT2 locale
    SUBMAT = (char*)malloc(sendcounts[me] * sizeof(char));
    SUBMAT2 = (char*)malloc(sendcounts[me] * sizeof(char));
    TEMP = (char*)malloc(sendcounts[me] * sizeof(char));
    //dichiara e alloca 2 array corrispondenti alle due righe da ricevere(A_up e A_down)
    char *A_up, *A_down;
    A_up = (char*)malloc(n * sizeof(char));
    A_down = (char*)malloc(n * sizeof(char));

    //ricevi sottomatrice locale
    MPI_Scatterv(MAT, sendcounts, offset, MPI_CHAR, SUBMAT, sendcounts[me], MPI_CHAR, 0, MPI_COMM_WORLD);
```



### Comunicazione
Si è scelto di utilizzare send e receive non bloccanti, in quanto la comunicazione è necessaria soltanto ai bordi della matrice (prima riga ed ultima riga) per ogni processo.  
Per questo motivo, se il numero di righe della sottomatrice per ogni processo è almeno 3, io posso da subito lavorare sulle righe centrali (da riga 1 a riga nRighe - 2, cioè dalla seconda alla penultima), solo quando saranno terminate le send e le receive lavorerò ai bordi.  
In questo modo si migliora l'efficienza del programma in quanto non aspetterà che si concludino prima tutte le send e receive per iniziare a lavorare ma si anticiperà del lavoro nel frattempo agendo sulle righe centrali.  
Di seguito il codice:  
```
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

        //variabili usate per il meccanismo di riconoscimento matrice vuota
        vuota = 0;
        emptCtr = 0;

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
                        vuota++;
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
                    vuota++;
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
                    vuota++;
                }
            }
            
        }
```
Infine si switcheranno le due matrici usate per portare avanti la computazione e si implementa il meccanismo per riconoscere se la matrice è vuota nel seguente modo: ogni processore utilizza una variabile 'vuota' che si incrementa ogni volta che si riconosce una cella vuota.  
Se dopo aver terminato una iterazione si nota che il valore in 'vuota' è uguale al numero totale di elementi nella sottomatrice per ogni processo, allora significa che quella sottomatrice è vuota -> si setta una variabile 'emptCtr a 1'.  
Si fa una MPI_Reduce delle variabili 'emptCtr' da tutti i processi al processo master che controlla che la somma di queste variabili sia uguale al numero di processori.  
In questo modo vuol dire che l'intera matrice è vuota, per cui il processo 0 avviserà tutti inviando in broadcast una variabile 'ok' settata a 1, e si esce dalle iterazioni.  
Di seguito il codice:  
```
        TEMP = SUBMAT;
        SUBMAT = SUBMAT2;
        SUBMAT2 = TEMP;

        if(nRighe == 1){
            vuota = vuota / 2;
        }
        //se un processo ha sottomatrice vuota, setta emptCtr a 1 altrimenti a 0
        if(vuota == sendcounts[me]){
            emptCtr = 1;
        }

        //reduce degli emptCtr al processo master 0
        MPI_Reduce(&emptCtr, &emptMaster, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        if(me == 0){
            //se tutti i processori hanno settato emptMaster a 1
            if(emptMaster == np){
                ok = 1;
                printf("Foresta vuota! Iterazioni totali: %d\n", i+1);
            }
        }
        //valore ok
        MPI_Bcast(&ok, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if(ok == 1){
            break;
        }
```


## Istruzioni per l'esecuzione
### Esecuzione locale
- Fare il git clone della repository.
- Ci sono 2 file .c, forrestParallelo.c è la versione concorrente del programma, il file forrestSeq.c quella sequenziale.
- Per compilare la versione sequenziale eseguire il comando ``` mpicc -o sequenziale forrestSeq.c ```, per la versione parallela ```mpicc -o parallelo forrestParallelo.c ```, per fare tests sulla correttezza ``` mpicc -o tests correttezzaParallelo.c ``` .
- Una volta compilati si esegue la versione sequenziale con comando ``` mpirun -np 1 sequenziale M N ```, dove al posto di M ed N si inseriscono rispettivamente  numero di righe e numero di colonne per la matrice.
- Si esegue la versione parallela con comando ``` mpirun --allow-run-as-root -np NP parallelo M N ``` , dove al posto di NP si inserisce il numero di processori ed al posto di M ed N numero di righe e di colonne.
- Si esegue la versione di tests con comando ``` mpirun -np NP tests M N ```.

### Esecuzione su cluster/cloud
- Creare n macchine virtuali e farne il set up seguendo la guida ``` https://github.com/spagnuolocarmine/ubuntu-openmpi-openmp ```  
- Fare il git clone della repository  
- Compilare su ogni macchina ``` mpicc -o parallelo forrestParallelo.c ``` per la versione parallela
- Si esegue sulla macchina master ``` mpirun --allow-run-as-root -np NP --hostfile hostfile parallelo M N ```, dove hostfile è il nome dell'hostfile creato
- Per la versione sequenziale si compila sulla macchina master ``` mpicc -o sequenziale forrestSeq.c ``` 
- Eseguire su macchina master ``` mpirun --allow-run-as-root -np NP sequenziale M N ```  
- Eseguire su macchina master ``` mpirun --allow-run-as-root -np NP tests M N ```




## Correttezza
Per dimostrare la correttezza di questa soluzione sono stati realizzati dei test cases, in ognuno dei quali si da' in input al programma una matrice inizializzata in un certo modo e si controlla l'output anche variando il numero dei processori (in particolare si testerà per 2, 4 e 8 core), in modo da dimostrare che il programma si comporta in modo corretto indipendentemente dal numero dei processori. 
Le probabilità F e P saranno settate a 0 in modo da non rendere pseudocasuale il programma.  
Per poter effettuare dei tests in proprio è presente il file correttezzaParallelo.c che stampa ad ogni iterazione la matrice ammesso che il numero di righe e colonne non sia superiore a 10.  

### Test case 1   
Si parte con una matrice composta da sole celle vuote 8x8 (naturalmente può essere di qualsiasi dimensione), ci aspettiamo che il programma riconosca la matrice vuota e che termini dopo una sola iterazione (sono impostate S = 50 iterazioni).  
La scelta di usare una matrice 8x8 è strategica: la si può stampare e visualizzare comodamente, inoltre impostando per ogni test il numero di processori rispettivamente a 2, 4, 8 si può verificare che il programma funzioni sia quando ad un processore spettano più righe (2 e 4 processori), sia quando spetta una sola riga (8 processori).   

**Per 2 processori:**  
<img src="https://github.com/gaetanodigenio/Forest-fire-problem-MPI/blob/main/img/Test_mat_vuota_2_proc.png" width="400" >

**Per 4 processori:**  
<img src="https://github.com/gaetanodigenio/Forest-fire-problem-MPI/blob/main/img/Test-mat-vuota-4-proc.png" width="400" >

**Per 8 processori:**  
<img src="https://github.com/gaetanodigenio/Forest-fire-problem-MPI/blob/main/img/Test-mat-vuota-8-proc.png" width="400" >


### Test case 2
Si da' in input una matrice (sempre 8x8) inizializzata nel seguente modo:  

<img src="https://github.com/gaetanodigenio/Forest-fire-problem-MPI/blob/main/img/Test2-Matrice.png" width="300">  

Data la composizione della matrice ci si aspetta che dopo la prima iterazione le righe 0, 2, 4, 6 da stato brucia passino a stato vuoto, mentre le righe 1, 3, 5, 7 da stato albero passino a stato brucia.  
Alla iterazione 2 invece la matrice diventi completamente vuota e si fermi l'esecuzione.  

**Per 2 processori:**  
<img src="https://github.com/gaetanodigenio/Forest-fire-problem-MPI/blob/main/img/test-case2-2-proc.png" width="650" >

**Per 4 processori:**  
<img src="https://github.com/gaetanodigenio/Forest-fire-problem-MPI/blob/main/img/test-case2-2-proc.png" width="650" >

**Per 8 processori:**  
<img src="https://github.com/gaetanodigenio/Forest-fire-problem-MPI/blob/main/img/test-case2-2-proc.png" width="650" >
 
### Test case 3
In questo test case si da' in input una matrice 8x8 inizializzata in modo particolare e si verifica che correttamente si svolgano i passi di computazione tenendo in considerazione la logica toroidale che la matrice deve seguire -> la riga 0 ha come riga superiore la riga 7, e viceversa la riga 7 ha come riga inferiore la riga 0, mentre la colonna 0 ha come colonna sinistra la colonna 7, e viceversa la colonna 7 ha come colonna destra la colonna 0.  
Matrice input:    
<img src="https://github.com/gaetanodigenio/Forest-fire-problem-MPI/blob/main/img/test_case_3_mat.png" width="250" >

La scelta della matrice input, ancora una volta, è strategica.  
In particolare ci si aspetta che gli alberi posti sulla prima riga e sull'ultima riga brucino poiché ragionando in maniera toroidale i vicini stanno bruciando.  
Allo stesso modo l'albero posto sulla 3 e 5 riga brucia poiché il vicino sulla colonna opposta sta bruciando.  
Dopo 2 iterazioni ci si aspetta che tutta la matrice sia vuota:  

**Per 2 processori:**  

<img src="https://github.com/gaetanodigenio/Forest-fire-problem-MPI/blob/main/img/test_case_3.jpg" width="350" >

**Per 4 processori:**  

<img src="https://github.com/gaetanodigenio/Forest-fire-problem-MPI/blob/main/img/test_case_3.jpg" width="350" >

**Per 8 processori:**  

<img src="https://github.com/gaetanodigenio/Forest-fire-problem-MPI/blob/main/img/test_case_3.jpg" width="350" >


Con questi 3 test case si dimostra la correttezza del programma indipendentemente dal numero di processori e che la logica toroidale funzioni allo stesso modo.  




## Benchmarks
### Scalabilità forte
Si sceglie come dimensione della matrice 8000 x 8000, con un numero di iterazioni S pari a 100.  
La scalabilità forte prevede una dimensione dell'input fissata ed il numero di processori che varia.  
Tutti i test sono stati svolti su un cluster di 6 macchine e2-standard-4, ognuno con 4 vCPU e 16gb di RAM, per un totale di 24vCPU.  
Lo speedup è assoluto, calcolato come il rapporto del tempo di esecuzione del programma sequenziale forrestSeq.c ed il tempo di esecuzione sul programma parallelo forrestParallelo.c .  
I test sono stati ripetuti più volte per avere un risultato più attendibile, di seguito sono mostrati i risultati:  

| Numero processori | Tempo esecuzione(s) | Speedup | Efficienza |
| ----------------- | ----------------    | ------- | ---------- |
|1                  |    66.194           |     /   |    /       |
|2                  |    38.512   |   1.718 | 0.859 |
|3 | 34.944 | 1.89 | 0.63 |
|4 | 30.236 | 2.189 | 0.547 |
|5 | 25.781 | 2.567 | 0.5134 | 
|6 | 19.919 | 3.323 | 0.553 |
|7 | 21.204 | 3.121 | 0.445 |
|8 | 19.788 | 3.345 | 0.418 |
|9 | 17.522 | 3.777 | 0.419 |
|10| 15.747 | 4.203 | 0.420 |
|11|  16.234 | 4.077 | 0.370 |
|12| 16.758 | 3.949 | 0.329 |
|13| 15.621 | 4.237 | 0.325 |
|14| 16.081 | 4.116 | 0.294 |
|15|  15.511 | 4.267 | 0.284 |
|16|  15.245 | 4.342 | 0.271 |
|17|  14.786  | 4.476 | 0.263 |
|18|  14.214  | 4.656 | 0.258 |
|19|  13.723  | 4.823 | 0.253 |
|20|  13.311  | 4.972 | 0.248 |
|21|  12.581  | 5.261 | 0.250 |
|22|  11.835 | 5.593 | 0.254 |
|23|  12.024  | 5.505 | 0.239 |
|24|  11.116  | 5.954 | 0.25 |

<img src="https://github.com/gaetanodigenio/Forest-fire-problem-MPI/blob/main/img/strong_scal.png" width="450" >


Si nota come il tempo di esecuzione diminuisce molto rapidamente ma tende, ad un certo punto, a rimanere piuttosto piatto non riuscendo a scendere sotto gli 11 secondi di tempo di esecuzione anche utilizzando 24 vCPU.  
L'overhead di comunicazione incide in maniera significativa, ed inoltre anche l'efficienza tende a scendere e ad appiattirsi, indice che all'aumentare dei processori il parallelismo si sfrutta sempre di meno.  


### Scalabilità debole
Ad ogni processore è assegnata una matrice di 150 * 6000 = 900000 elementi.  
La scalabilità debole prevede una dimensione dell'input costante per singolo processore, per cui per ogni test si da' in input (np * 150) * 6000 elementi, dove np è il numero dei processori.  
Di seguito i risultati:  

| Numero di processori | Tempo di esecuzione (s) |
| -------------------- | ----------------------- |
|1  | 1.647 |
|2  | 1.668 |
|3  | 1.715 |
|4  | 1.705 |
|5  | 2.490 |
|6  | 2.514 |
|7  | 3.375 |
|8  | 4.641 |
|9  | 6.406 |
|10 | 6.328 |
|11 | 6.441 |
|12 | 7.76  |
|13 | 8.153 |
|14 | 8.430 |
|15 | 8.705 |
|16 | 8.739 |
|17 | 8.982 |
|18 | 8.830 |
|19 | 9.133 |
|20 | 9.990 |
|21 | 9.582 |
|22 | 9.845 |
|23 | 10.244 |
|24 | 10.607 |

<img src="https://github.com/gaetanodigenio/Forest-fire-problem-MPI/blob/main/img/weak_scal.png" width="450" >

Nonostante le performance dell'algoritmo migliorino quando si utilizza la parallelizzazione, i risultati non sono eccelsi come ci si aspetta.  
L'overhead di comunicazione che ad ogni iterazione si ha quando si scambiano le righe o quando si controlla che la matrice sia vuota porta ad un miglioramento dello speedup non altissimo (speedup sotto al 6 utilizzando 24 processori).  
Allo stesso modo nella weak scalability si nota un incremento del tempo di esecuzione piuttosto importante, è da tenere in considerazione però che la dimensione della matrice data in input era piuttosto grande in tutti questi benchmark.  





