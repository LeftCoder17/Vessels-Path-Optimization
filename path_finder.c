/*
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    $$$$$$$$$$$$    PATH_FINDER.C VERSION 3.0    $$$$$$$$$$$$
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

    - Compilation:
        >> gcc -o path path_finder.c libs/graph_management.c libs/algorithms.c -lm

    - Usage:
        >> ./path stored_graph.bin heuristic_code program_mode (+ additional args depending on program_mode)
            Program modes:
            >> 0: pq_code initial_lat initial_lon final_lat final_lon solution_filename_PATH.txt solution_filename_CONTROL.txt
            >> 1: pq_code path_id solution_filename_PATH.txt solution_filename_CONTROL.txt
            >> 2: heuristic_metrics.txt pq_metrics.txt

    - Output:
        >> For program mode 0:
            >> The path in solution_filename_PATH.txt
            >> The final states of the nodes in solution_filename_CONTROL.txt
        >> For program mode 1:
            >> The path in solution_filename_PATH.txt
            >> The final states of the nodes in solution_filename_CONTROL.txt
        >> For program mode 2:
            >> Different metrics to analyse the heuristics and priority queues in, respectively, heuristic_metrics.txt and pq_metrics.txt 

    - Comments:
        >> This program opens a stored graph in a binary file and finds the best path between the initial and final coordinates, depending on the program mode.
        >> Different heuristic codes can be used:
            >> 0: h = 0 is like using Dijkstra algorithm.
            >> 1: Haversine
            >> 2: Spherical Law of Cosines
            >> 3: Equirectangular approximation
        >> Different Priority Queues can be used:
            >> 0: Linked List
            >> 1: Binary Heap
        >> Different program_modes can be used:
            >> 0: finds the solution taking initial and final coordinates
            >> 1: finds the solution for every path in the binary file and stores the a summary of the solution
            >> 2: execute both PQ for a chosen heuristic for every path and stores different performance metrics

        >> Speed of the ships in [kn].
    
    - Further development:
        >> Add a cost that manages not only the fastest path, but also the oil consumption and time waiting at the harbor, depending at the time they need to arrive.
    
    - Status:
        >> Finished.

    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>

#include "libs/graph_management.h"
#include "libs/algorithms.h"

int main (int argc, char **argv) {
    if (argc < 5) ExitError("Missing arguments", 1);

    // 1. Read the binary file
    printf("Reading bin file...\n");

    Node *nodes;
    Path *paths;
    unsigned long nnodes, nedges, npaths;
    char *bin_filename;
    
    nodes = NULL;
    paths = NULL;
    bin_filename = strdup(argv[1]);
    if (bin_filename == NULL) ExitError("when copying the binary filename", 2);
    read_nodes(&nodes, &paths, &nnodes, &nedges, &npaths, bin_filename);

    free(bin_filename);

    // 2. Set the program mode
    int heuristic_code = atoi(argv[2]);
    int program_mode = atoi(argv[3]);

    if (program_mode == 0) {
        // 3.0.1. Choose initial node and final node
        printf("Program mode 0. Chossing initial and final nodes...\n");

        double initial_lat, initial_lon, final_lat, final_lon;
        unsigned long initial_node, final_node;
        double minimum_distance, aux_distance;
        unsigned long index;

        initial_lat = atof(argv[5]);
        initial_lon = atof(argv[6]);
        final_lat = atof(argv[7]);
        final_lon = atof(argv[8]);

        initial_node = 0;
        minimum_distance = nodes_squared_distance(initial_lat, initial_lon, nodes[0].lat, nodes[0].lon);
        for (index = 1; index < nnodes; index++) {
            aux_distance = nodes_squared_distance(initial_lat, initial_lon, nodes[index].lat, nodes[index].lon);
            if (aux_distance < minimum_distance) {
                initial_node = index;
                minimum_distance = aux_distance;
            }
        }

        final_node = 0;
        minimum_distance = nodes_squared_distance(final_lat, final_lon, nodes[0].lat, nodes[0].lon);
        for (index = 1; index < nnodes; index++) {
            aux_distance = nodes_squared_distance(final_lat, final_lon, nodes[index].lat, nodes[index].lon);
            if (aux_distance < minimum_distance) {
                final_node = index;
                minimum_distance = aux_distance;
            }
        }

        // 3.0.2. A* algorithm
        printf("Finding path...\n");

        AStarPath *Sol_path;
        Sol_path = (AStarPath *) malloc(nnodes * sizeof(AStarPath));
        if (Sol_path == NULL) ExitError("when allocating memory for the AStarPath", 3);

        AStarControlData *Queue_control;
        Queue_control = (AStarControlData *) malloc(nnodes * sizeof(AStarControlData));
        if (Queue_control == NULL) ExitError("when allocating memory for the AStar Control Data vector", 4);
        
        for (index = 0; index < nnodes; index++) {
            Sol_path[index].g = DBL_MAX;
            Queue_control[index].InPQ = false;
            Queue_control[index].extended = false;
        }
        int pq_code = atoi(argv[4]);
        int result;
        if (pq_code == 0) {
            result = AStar_ll(nodes, Sol_path, Queue_control, initial_node, final_node, heuristic_code);
        } else if (pq_code == 1) {
            result = AStar_bh(nodes, Sol_path, Queue_control, nnodes, initial_node, final_node, heuristic_code);
        } else {
            ExitError("wrong pq_code. Must be 0 or 1", 5);
        }
        

        if (result == -1) ExitError("in allocating memory for the PQ list in AStar", 6);
        else if(result == 0) ExitError("no solution found in AStar", 7);

        // 3.0.3. Store the solution
        printf("Storing solution...\n");

        unsigned long nnodes_path;
        char *path_filename;
        path_filename = strdup(argv[9]);
        if (path_filename == NULL) ExitError("when copying the path filename", 8);
        nnodes_path = store_solution(nodes, Sol_path, initial_node, final_node, path_filename);

        // 3.0.4. Store Control state
        printf("Storing Control state...\n");

        char *control_filename;
        control_filename = strdup(argv[10]);
        Queue_control[final_node].InPQ = false;
        Queue_control[final_node].extended = true;
        store_control(nodes, Sol_path, Queue_control, initial_node, nnodes, nnodes_path, control_filename);

        // 3.0.5. Free allocated memory
        free(path_filename);
        free(control_filename);
        free(Sol_path);
        free(Queue_control);
    } else if (program_mode == 1) {
        printf("Program mode 1. Chossing initial and final nodes...\n");
        unsigned long selected_path;
        selected_path = strtoul(argv[5], NULL, 10);

        unsigned long initial_node, final_node;
        initial_node = paths[selected_path].start_node.node_id;
        final_node = paths[selected_path].final_node->node_id;
        
        // 3.1.1. A* algorithm
        printf("Finding path...\n");

        AStarPath *Sol_path;
        Sol_path = (AStarPath *) malloc(nnodes * sizeof(AStarPath));
        if (Sol_path == NULL) ExitError("when allocating memory for the AStarPath", 9);

        AStarControlData *Queue_control;
        Queue_control = (AStarControlData *) malloc(nnodes * sizeof(AStarControlData));
        if (Queue_control == NULL) ExitError("when allocating memory for the AStar Control Data vector", 10);
        
        unsigned long index;
        for (index = 0; index < nnodes; index++) {
            Sol_path[index].g = DBL_MAX;
            Queue_control[index].InPQ = false;
            Queue_control[index].extended = false;
        }
        int pq_code = atoi(argv[4]);
        int result;
        if (pq_code == 0) {
            result = AStar_ll(nodes, Sol_path, Queue_control, initial_node, final_node, heuristic_code);
        } else if (pq_code == 1) {
            result = AStar_bh(nodes, Sol_path, Queue_control, nnodes, initial_node, final_node, heuristic_code);
        } else {
            ExitError("wrong pq_code. Must be 0 or 1", 11);
        }

        if (result == -1) ExitError("in allocating memory for the PQ list in AStar", 12);
        else if(result == 0) ExitError("no solution found in AStar", 13);

        // 3.1.2. Store the solution
        printf("Storing solution...\n");

        unsigned long nnodes_path;
        char *path_filename;
        path_filename = strdup(argv[6]);
        if (path_filename == NULL) ExitError("when copying the path filename", 14);
        nnodes_path = store_solution(nodes, Sol_path, initial_node, final_node, path_filename);

        // 3.1.3. Store Control state
        printf("Storing Control state...\n");

        char *control_filename;
        control_filename = strdup(argv[7]);
        if (control_filename == NULL) ExitError("when copying the control filename", 15);
        store_control(nodes, Sol_path, Queue_control, initial_node, nnodes, nnodes_path, control_filename);

        // 3.1.4. Free allocated memory
        free(path_filename);
        free(control_filename);
        free(Sol_path);
        free(Queue_control);

    } else if (program_mode == 2) {
        printf("Program mode 2. Preparing memory...\n");
        unsigned long index, path_index;
        unsigned long initial_node, final_node;

        Heuristic_Metrics *heuristic_metrics_ll;
        Heuristic_Metrics *heuristic_metrics_bh;
        PQ_Metrics *pq_metrics_ll;
        PQ_Metrics *pq_metrics_bh;
        heuristic_metrics_ll = (Heuristic_Metrics *) malloc(npaths * sizeof(Heuristic_Metrics));
        heuristic_metrics_bh = (Heuristic_Metrics *) malloc(npaths * sizeof(Heuristic_Metrics));
        pq_metrics_ll = (PQ_Metrics *) malloc(npaths * sizeof(PQ_Metrics));
        pq_metrics_bh = (PQ_Metrics *) malloc(npaths * sizeof(PQ_Metrics));
        if (heuristic_metrics_ll == NULL) ExitError("when allocating memory for heuristic metrics ll", 16);
        if (heuristic_metrics_bh == NULL) ExitError("when allocating memory for heuristic metrics bh", 17);
        if (pq_metrics_ll == NULL) ExitError("when allocating memory for pq metrics ll", 18);
        if (pq_metrics_bh == NULL) ExitError("when allocating memory for pq metrics bh", 19);


        printf("Iterating over every path...\n");
        for (path_index = 0; path_index < npaths; path_index++) {
            // 3.2.1. Choose initial node and final node
            initial_node = paths[path_index].start_node.node_id;
            final_node = paths[path_index].final_node->node_id;

            // 3.2.2. A* algorithm
            printf("\rFinding solution for path %lu out of %lu", path_index + 1, npaths);

            // 3.2.3.1. Initialize metrics
            heuristic_metrics_ll[path_index].path_id = paths[path_index].id;
            heuristic_metrics_bh[path_index].path_id = paths[path_index].id;
            // Distance computed with haversine formula
            double sep_km = distance_km(nodes[initial_node].lon, nodes[initial_node].lat, nodes[final_node].lon, nodes[final_node].lat);
            heuristic_metrics_ll[path_index].sep_km = sep_km;
            heuristic_metrics_bh[path_index].sep_km = sep_km;
            heuristic_metrics_ll[path_index].nexpanded = 0;
            heuristic_metrics_bh[path_index].nexpanded = 0;
            heuristic_metrics_ll[path_index].calculus_time = 0.;
            heuristic_metrics_bh[path_index].calculus_time = 0.;

            pq_metrics_ll[path_index].path_id = paths[path_index].id;
            pq_metrics_bh[path_index].path_id = paths[path_index].id;
            pq_metrics_ll[path_index].enqueue_time = 0.;
            pq_metrics_bh[path_index].enqueue_time = 0.;
            pq_metrics_ll[path_index].dequeue_time = 0.;
            pq_metrics_bh[path_index].dequeue_time = 0.;
            pq_metrics_ll[path_index].requeue_time = 0.;
            pq_metrics_bh[path_index].requeue_time = 0.;
            pq_metrics_ll[path_index].sep_km = sep_km;
            pq_metrics_bh[path_index].sep_km = sep_km;

            // 3.2.3.2. Initialize AStar structures
            AStarPath *Sol_path_ll;
            AStarPath *Sol_path_bh;
            Sol_path_ll = (AStarPath *) malloc(nnodes * sizeof(AStarPath));
            Sol_path_bh = (AStarPath *) malloc(nnodes * sizeof(AStarPath));
            if (Sol_path_ll == NULL) ExitError("when allocating memory for the AStarPath Linked List", 20);
            if (Sol_path_bh == NULL) ExitError("when allocating memory for the AStarPath Binary Heap", 21);

            AStarControlData *Queue_control_ll;
            AStarControlData *Queue_control_bh;
            Queue_control_ll = (AStarControlData *) malloc(nnodes * sizeof(AStarControlData));
            Queue_control_bh = (AStarControlData *) malloc(nnodes * sizeof(AStarControlData));
            if (Queue_control_ll == NULL) ExitError("when allocating memory for the AStar Control Data vector Linked List", 22);
            if (Queue_control_bh == NULL) ExitError("when allocating memory for the AStar Control Data vector Binary Heap", 23);

            for (index = 0; index < nnodes; index++) {
                Sol_path_ll[index].g = DBL_MAX;
                Sol_path_bh[index].g = DBL_MAX;
                Queue_control_ll[index].InPQ = false;
                Queue_control_bh[index].InPQ = false;
                Queue_control_ll[index].extended = false;
                Queue_control_bh[index].extended = false;
            }

            // 3.2.3.3. With Linked List Priority Queue
            int result;
            unsigned long curr_node;
            result = AStar_ll_metrics(nodes, Sol_path_ll, Queue_control_ll, initial_node, final_node, heuristic_code, &heuristic_metrics_ll[path_index], &pq_metrics_ll[path_index]);
            if (result == -1) ExitError("in allocating memory for the PQ list in AStar Linked List", 24);
            else if(result == 0) ExitError("no solution found in AStar", 25);
            heuristic_metrics_ll[path_index].solution_cost = Sol_path_ll[final_node].g;
            curr_node = final_node;
            while (curr_node != initial_node) {
                heuristic_metrics_ll[path_index].nsolution += 1;
                curr_node = Sol_path_ll[curr_node].parent;
            }
            heuristic_metrics_ll[path_index].nsolution += 1;
            pq_metrics_ll[path_index].nsolution = heuristic_metrics_ll[path_index].nsolution;

            // 3.2.3.4. With Binary Heap Priority Queue
            result = AStar_bh_metrics(nodes, Sol_path_bh, Queue_control_bh, nnodes, initial_node, final_node, heuristic_code, &heuristic_metrics_bh[path_index], &pq_metrics_bh[path_index]);
            if (result == -1) ExitError("in allocating memory for the PQ list in AStar Binary Heap", 26);
            else if(result == 0) ExitError("no solution found in AStar", 27);
            heuristic_metrics_bh[path_index].solution_cost = Sol_path_bh[final_node].g;
            curr_node = final_node;
            while (curr_node != initial_node) {
                heuristic_metrics_bh[path_index].nsolution += 1;
                curr_node = Sol_path_bh[curr_node].parent;
            }
            heuristic_metrics_bh[path_index].nsolution += 1;
            pq_metrics_bh[path_index].nsolution = heuristic_metrics_bh[path_index].nsolution;

            // 3.2.3.5. Free allocated memory
            free(Sol_path_ll);
            free(Queue_control_ll);
            free(Sol_path_bh);
            free(Queue_control_bh);
        }

        // 3.1.5. Store the metrics
        printf("\nStoring heuristic metrics...\n");
        char *heuristic_filename;
        heuristic_filename = strdup(argv[4]);
        if (heuristic_filename == NULL) ExitError("when copying the name of the heuristic metrics file", 28);

        store_heuristic_metrics(heuristic_metrics_ll, heuristic_metrics_bh, npaths, heuristic_filename);

        printf("\nStoring Priority Queue metrics...\n");
        char *pq_filename;
        pq_filename = strdup(argv[5]);
        if (pq_filename == NULL) ExitError("when copying the name of the pq metrics file", 28);

        store_pq_metrics(pq_metrics_ll, pq_metrics_bh, npaths, pq_filename);

        // 3.1.6. Free allocated memory
        free(heuristic_metrics_ll);
        free(heuristic_metrics_bh);
        free(pq_metrics_ll);
        free(pq_metrics_bh);
    } else {
        ExitError("wrong program mode", 29);
    }
    
    // 4. Free allocated memory
    printf("Freeing memory...\n");

    free_paths(paths, npaths);
    free_nodes(nodes, nnodes);

    return 0;
}