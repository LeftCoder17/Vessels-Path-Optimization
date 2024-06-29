/*
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    $$$$$$$$$$$$$$    ADD_INTERSECTIONS.C VERSION 4.0    $$$$$$$$$$$$$$
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    
    - Compilation:
        >> gcc -o add_int -W -Wall -Werror add_intersections.c libs/graph_management.c libs/intersections.c -lm

    - Usage:
        >> ./add_int stored_graph.bin data_output.bin counter_filename.txt

    - Output:
        >> The graph that is stored in data_output.bin.
        >> The counter of intersections per each path in counter_filename.txt

    - Comments:
        >> This program opens a stored graph in a binary file and appends the intersections between edges as new nodes.
        >> The id of each node is assigned by order of appearance. Thus, they are stored in an array in the position corresponding to their id.
        >> The edges are unidirectional.
        >> The intersections type 1 are implemented once they are detected. 

    
    - Further development:
        >> Sort by coordinates the nodes and identify intersections with a Sweep line algorithm.

    - Status:
        >> Finished

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "libs/graph_management.h"
#include "libs/intersections.h"

int main (int argc, char *argv[]) {
    if (argc < 4) ExitError("Inputs missing to the program", 1);

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

    // 2. Compute intersections
    printf("Computing intersections...\n");

    unsigned long i_path_1, i_path_2, initial_path_2;
    unsigned short compute_paths;
    Path_node *p1, *p2, *q1, *q2;
    unsigned long i_path_p1;
    unsigned short intersection_type;
    double t, u;
    unsigned long intersections_computed, intersections_ignored, max_nnodes;
    unsigned long *int_per_path;

    int_per_path = (unsigned long *) malloc(npaths * sizeof(unsigned long));
    if (int_per_path == NULL) ExitError("when allocating memory for int_per_path", 3);
    for (i_path_1 = 0; i_path_1 < npaths; i_path_1++) int_per_path[i_path_1] = 0;

    intersections_computed = 0;
    intersections_ignored = 0;
    max_nnodes = nnodes;
    for (i_path_1 = 0; i_path_1 < npaths - 1; i_path_1++) {
        printf("\rPath 1: %lu out of %lu", i_path_1, npaths);
        if (paths[i_path_1].npaths) initial_path_2 = paths[i_path_1].npaths + 1;
        else initial_path_2 = i_path_1 + 1;
        if (initial_path_2 > npaths) break;
        for (i_path_2 = initial_path_2; i_path_2 < npaths; i_path_2++) {
            // 2.1. Check whether it is necessary or not to compute these pair of paths
            compute_paths = need_compute_paths(paths, i_path_1, i_path_2);
            if (compute_paths == 0) continue;

            // 2.2. Select edges p1-q1 and p2-q2 to compute
            p1 = &paths[i_path_1].start_node;
            for (i_path_p1 = 0; i_path_p1 < paths[i_path_1].len - 1; i_path_p1++) {
                q1 = p1->next;
                for (p2 = &paths[i_path_2].start_node; p2->next != NULL; p2 = q2) {
                    q2 = p2->next;
                    intersection_type = identify_intersection(&nodes[p1->node_id], &nodes[q1->node_id], &nodes[p2->node_id], &nodes[q2->node_id], &t, &u);
                    if (intersection_type != 1) {
                        if (intersection_type >1) {
                            intersections_ignored++;
                        }
                    } else {
                        add_intersection(&nodes, &nnodes, &max_nnodes, &nedges, p1->node_id, q1->node_id, p2->node_id, q2->node_id, 
                                        intersection_type, t, u, paths, i_path_1, i_path_2, p1, p2);

                        intersections_computed++;
                        int_per_path[i_path_1]++;
                        int_per_path[i_path_2]++;
                        q1 = p1->next;
                    }
                }
                p1 = p1->next;
            }
            checked_paths(paths, i_path_1, i_path_2);
        }
    }
    printf("\nComputed intersections: %lu\nIgnored intersections: %lu\n", intersections_computed, intersections_ignored);

    // 3. Store in a new binary file
    printf("Storing graph...\n");

    char *bin_new_filename;
    bin_new_filename = strdup(argv[2]);
    if (bin_new_filename == NULL) ExitError("when copying the binary new filename", 4);

    store_nodes(nodes, paths, nnodes, nedges, npaths, bin_new_filename);

    // 4. Store the counter of intersections per path
    printf("Storing intersections counter...\n");
    char *counter_filename;
    counter_filename = strdup(argv[3]);
    if (counter_filename == NULL) ExitError("when copying the counter filename", 5);
    
    FILE *counter_file;
    counter_file = fopen(counter_filename, "w");
    if (counter_file == NULL) ExitError("when opening the counter file", 6);

    for (i_path_1 = 0; i_path_1 < npaths; i_path_1++) {
        fprintf(counter_file, "%lu %lu\n", i_path_1, int_per_path[i_path_1]);
    }
    fclose(counter_file);

    // 5. Free allocated memory
    printf("Freeing memory...\n");

    free_paths(paths, npaths);
    free_nodes(nodes, nnodes);
    
    return 0;
}