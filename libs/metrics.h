#ifndef METRICS_H
#define METRICS_H

// Stores different metrics about an heuristic
typedef struct {
    unsigned long path_id;
    double solution_cost;
    double sep_km;
    unsigned long nsolution;
    unsigned long nexpanded;
    double calculus_time;
} Heuristic_Metrics;

// Stores different metrics about a Priority Queue
typedef struct {
    unsigned long path_id;
    double total_time;
    double enqueue_time;
    double dequeue_time;
    double requeue_time;
    double sep_km;
    unsigned long nsolution;
} PQ_Metrics;

// Creates and stores a file with the heuristic metrics selected
void store_heuristic_metrics(Heuristic_Metrics *metrics_ll, Heuristic_Metrics *metrics_bh, unsigned long npaths, char *filename);

// Creates and stores a file with the Priority Queue metrics selected
void store_pq_metrics(PQ_Metrics *metrics_ll, PQ_Metrics *metrics_bh, unsigned long npaths, char *filename);

#endif