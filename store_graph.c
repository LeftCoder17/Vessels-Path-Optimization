/*
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    $$$$$$$$$$$$    STORE_GRAPH.C VERSION 4.0    $$$$$$$$$$$$
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

    - Compilation:
        >> gcc -o store store_graph.c libs/graph_management.c -lm

    - Usage:
        >> ./store data_input.csv shiptypes_counter.txt program_mode (+ additonal)
        >> For program mode:
            >> 0: data_output.bin
            >> 1: nshiptypes data_output_1.bin  shiptype1 data_output_2.bin shiptype2 ...
            >> 2: data_output

    - Output:
        >> The graph that is stored in data_output.bin. Only the paths with same shiptype as in the input command if indicated.
        >> A file shiptypes_counter.txt containing how many paths of every shiptype there are.

    - Comments:
        >> This program reads a csv file and stores the nodes and edges in a binary file.
        >> Optionally, in the command line it is possible to give the shiptype which will be stored specifically in the binary file.
        >> The node id and path id are given by order of appearance.
        >> Edges are unidirectional and are created only when two adjacent nodes belong to the same shipname.
        >> The path nodes are stored through a linked list.

    
    - Further development:
        >> 

    - Status:
        >> Finished.


    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libs/graph_management.h"

int main (int argc, char *argv[]) {
    if (argc < 5) ExitError("Inputs missing to the program", 1);

    // 1. Read the input file
    // 1.1. Open the file
    printf("Opening file...\n");
    FILE *csv_file;
    char *csv_filename;
    csv_filename = strdup(argv[1]);
    csv_file = fopen(csv_filename, "r");

    if (csv_file == NULL) ExitError("when opening the data file", 2);

    // 1.2. Count the number of nodes in the csv
    printf("Counting nodes...\n");
    unsigned long new_nnodes;
    new_nnodes = nnodes_in_csv(csv_file);
    rewind(csv_file);

    // 1.3. Compute the nodes, edges and paths
    printf("Computing the data of the file...\n");
    unsigned long nnodes, nedges, npaths;
    nnodes = 0;
    nedges = 0;
    npaths = 0;

    Node *nodes;
    Path *paths;

    nodes = (Node *) malloc(new_nnodes*sizeof(Node));
    paths = (Path *) malloc(new_nnodes*sizeof(Path));  // Maximum size of paths
    if (nodes == NULL) ExitError("when allocating memory for nodes", 3);
    if (paths == NULL) ExitError("when allocating memory for paths", 4);

    add_nodes_from_csv(nodes, paths, new_nnodes, &nnodes, &nedges, &npaths, csv_file);
    fclose(csv_file);

    // 3. Count the number of paths for every shiptype
    printf("Counting the number of paths for every shiptype...\n");
    ST_counter *head_ST, *curr_ST;
    head_ST = count_shiptypes(paths, npaths);

    // 4. Stores the number of paths for every shiptype
    printf("Storing the number of paths for every shiptype...\n");

    char *counter_filename;
    counter_filename = strdup(argv[2]);
    if (counter_filename == NULL) ExitError("when copying the counter filename", 5);
    
    FILE *counter_file;
    counter_file = fopen(counter_filename, "w");
    if (counter_file == NULL) ExitError("when opening the counter file", 6);

    curr_ST = head_ST;
    while (curr_ST != NULL) {
        fprintf(counter_file, "%d %lu\n", curr_ST->shiptype, curr_ST->npaths);
        curr_ST = curr_ST->next;
    }
    fclose(counter_file);
    
    // 5. Store the graph
    printf("Storing graph with %lu nodes, %lu edges and %lu paths...\n", nnodes, nedges, npaths);

    int program_mode = atoi(argv[3]);
    if (program_mode == 0) {
        printf("Program mode 0 selected...\n");
        char *bin_filename;
        bin_filename = strdup(argv[4]);
        if (bin_filename == NULL) ExitError("when copying the binary filename", 7);
        store_nodes(nodes, paths, nnodes, nedges, npaths, bin_filename);
        free(bin_filename);
    } else if (program_mode == 1) {
        printf("Program mode 1 selected...\n");
        char *bin_filename;
        int nshiptypes, i_shiptype, shiptype;
        nshiptypes = atoi(argv[4]);
        for (i_shiptype = 0; i_shiptype < nshiptypes; i_shiptype++) {
            bin_filename = strdup(argv[5 + i_shiptype*2]);
            if (bin_filename == NULL) ExitError("when copying the binary filename", 8);
            shiptype = atoi(argv[6 + i_shiptype*2]);
            store_nodes_shiptype(nodes, paths, shiptype, npaths, bin_filename);
            free(bin_filename);
        }
    } else if (program_mode == 2) {
        printf("Program mode 2 selected...\n");
        size_t model_len, format_len;
        char *model_filename;
        model_filename = strdup(argv[4]);
        if (model_filename == NULL) ExitError("when copying the model filename", 9);
        model_len = strlen(model_filename);
        format_len = strlen(".bin");

        curr_ST = head_ST;
        while (curr_ST != NULL) {
            char bin_filename[128];
            char shiptype_str[12];
            size_t shiptype_len;
            sprintf(shiptype_str, "%d", curr_ST->shiptype);
            shiptype_len = strlen(shiptype_str);
            size_t total_len;
            total_len = model_len + 1 + shiptype_len + format_len + 1;
            if (total_len > sizeof(bin_filename)) ExitError("binary filename too long", 10);
            snprintf(bin_filename, sizeof(bin_filename), "%s_%d.bin", model_filename, curr_ST->shiptype);
            
            store_nodes_shiptype(nodes, paths, curr_ST->shiptype, npaths, bin_filename);
            curr_ST = curr_ST->next;
        }
        free(model_filename);
    }

    // 5. Free allocated memory
    printf("Freeing memory...\n");

    free_paths(paths, npaths);
    free_nodes(nodes, nnodes);

    return 0;
}
