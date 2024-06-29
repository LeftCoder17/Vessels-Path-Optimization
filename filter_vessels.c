/*
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    $$$$$$$$$$$$$$$    FILTER_VESSELS.C VERSION 1.0    $$$$$$$$$$$$$$$$
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    
    - Compilation:
        >> gcc -o filter -W -Wall -Werror filter_vessels.c libs/graph_management.c -lm

    - Usage:
        >> ./filter stored_graph.bin filtered_stored_graph.bin tolerance_in_km

    - Output:
        >> The filtered graph in filtered_stored_graph.bin

    - Comments:
        >> This programs removes all the paths that has less than the tolerance distance
        >> between the initial and final node.
    
    - Further development:

    - Status:
        >> Unfinished

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libs/graph_management.h"

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

    // 2. Filter paths and store
    printf("Filtering and storing graph...\n");

    char *bin_new_filename;
    bin_new_filename = strdup(argv[2]);
    if (bin_new_filename == NULL) ExitError("when copying the binary new filename", 3);

    char *end_ptr;
    double tolerance;
    tolerance = strtod(argv[3], &end_ptr);
    
    store_nodes_filtered(nodes, paths, tolerance, npaths, bin_new_filename);

    // 3. Free allocated memory
    printf("Freeing memory...\n");

    free_paths(paths, npaths);
    free_nodes(nodes, nnodes);
    return 0;
}