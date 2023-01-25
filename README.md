# Forest-fire-problem-MPI


## Introduzione
In matematica applicata il forest-fire model è un automa cellulare su una griglia di NxN celle consistente nella simulazione di un incendio di una foresta, cui ciascuna cella può essere vuota, occupata da un albero o in fiamme.
Lo stato di una cella può essere: vuota (E), albero (T), brucia (B).
Secondo il modello di Drossel e Schwabl (1992), ci sono 4 regole eseguite simultaneamente:
 1. Una cella in fiamme diventa una cella vuota
 2. Un albero si incendia se almeno un vicino è in fiamme
 3. Un albero si infiamma con probabilità F se nessun vicino è in fiamme
 4. Una cella vuota si riempie con un albero con probabilità P  
 
La simulazione termina se si raggiunge un numero massimo di iterazioni (steps) S, oppure se tutte le celle sono vuote.  

![](https://github.com/gaetanodigenio/Forest-fire-problem-MPI/blob/main/img/Forest_fire_model.gif)

## Descrizione della soluzione



## Correttezza
Per dimostrare la correttezza di questa soluzione sono stati realizzati dei test cases, in ognuno dei quali si da' in input al programma una matrice inizializzata in un certo modo e si controlla l'output anche variando il numero dei processori (in particolare si testerà per 2, 4 e 8 core), in modo da dimostrare che il programma si comporta in modo corretto indipendentemente dal numero dei processori. 
Le probabilità F e P saranno settate a 0 in modo da non rendere pseudocasuale il programma.  
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
In questo test case di mette alla prova il funzionamento del programma, dando in input una matrice 8x8 inizializzata in modo particolare e verificando che correttamente si svolgano i passi di computazione tenendo in considerazione la logica toroidale che la matrice deve seguire -> la riga 0 ha come riga superiore la riga 7, e viceversa la riga 7 ha come riga inferiore la riga 0, mentre la colonna 0 ha come colonna sinistra la colonna 7, e viceversa la colonna 7 ha come colonna destra la colonna 0.  
Matrice input:  






## Benchmarks
### Scalabilità forte
dim matrice 8000 x 8000

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


### Scalabilità debole
Ad ogni processore è assegnata una matrice di 150 * 6000 = 900000 elementi.

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



