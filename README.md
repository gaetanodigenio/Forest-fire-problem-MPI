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

## Descrizione della soluzione



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



