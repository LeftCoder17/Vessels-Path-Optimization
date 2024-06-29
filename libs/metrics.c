/*
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    $$$$$$$$$$$$$$    METRICS.C VERSION 1.0    $$$$$$$$$$$$$$$$
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

    - Usage:
        >> Through the header file "metrics.h"

    - Comments:
        >> 
    
    - Further development:
        >> 

    - Status:
        >> Finished.

    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
*/

#include <stdio.h>
#include "metrics.h"
#include "graph_management.h"

void store_heuristic_metrics(Heuristic_Metrics *metrics_ll, Heuristic_Metrics *metrics_bh, unsigned long npaths, char *filename) {
    FILE *file;
    file = fopen(filename, "w");
    if (file == NULL) ExitError("when opening the heuristic metrics file", 1);
    fprintf(file, "PATH ID,SEPARATION [km],SOLUTION COST LL [s],NNODES IN SOLUTION PATH LL,NNODES EXPANDED LL,CALCULUS TIME LL [s],SOLUTION COST BH [s],NNODES IN SOLUTION PATH BH,NNODES EXPANDED BH,CALCULUS TIME BH [s]\n");
    for (unsigned long index = 0; index < npaths; index++) {
        fprintf(file, "%lu,%g,%g,%lu,%lu,%g,%g,%lu,%lu,%g\n",
                metrics_ll[index].path_id, metrics_ll[index].sep_km,
                metrics_ll[index].solution_cost, metrics_ll[index].nsolution,
                metrics_ll[index].nexpanded, metrics_ll[index].calculus_time,
                metrics_bh[index].solution_cost, metrics_bh[index].nsolution,
                metrics_bh[index].nexpanded, metrics_bh[index].calculus_time);
    }
    fclose(file);
    return;
}


void store_pq_metrics(PQ_Metrics *metrics_ll, PQ_Metrics *metrics_bh, unsigned long npaths, char *filename) {
    FILE *file;
    file = fopen(filename, "w");
    if (file == NULL) ExitError("when opening the pq metrics file", 1);
    fprintf(file, "PATH ID,SEPARATION [km],NNODES IN SOLUTION PATH LL,TOTAL TIME LL [s],ENQUEUE TIME LL [s],DEQUEUE TIME LL [s],REQUEUE TIME LL [s],NNODES IN SOLUTION PATH BH,TOTAL TIME BH [s],ENQUEUE TIME BH [s],DEQUEUE TIME BH [s],REQUEUE TIME BH [s]\n");
    for (unsigned long index = 0; index < npaths; index++) {
        fprintf(file, "%lu,%g,%lu,%g,%g,%g,%g,%lu,%g,%g,%g,%g\n",
                metrics_ll[index].path_id, metrics_ll[index].sep_km,
                metrics_ll[index].nsolution, metrics_ll[index].total_time, metrics_ll[index].enqueue_time,
                metrics_ll[index].dequeue_time, metrics_ll[index].requeue_time,
                metrics_bh[index].nsolution, metrics_bh[index].total_time, metrics_bh[index].enqueue_time,
                metrics_bh[index].dequeue_time, metrics_bh[index].requeue_time);
    }
    fclose(file);

    return;
}
