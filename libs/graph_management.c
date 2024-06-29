/*
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    $$$$$$$$$    GRAPH_MANAGEMENT.C VERSION 2.0    $$$$$$$$$$
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

    - Usage:
        >> Through the header file "graph_management.h"

    - Comments:
        >> It doesn't read the ships with name "[SAT-AIS]"
    
    - Further development:
        >> In nnodes_in_csv, I check if there is a new node looking at the length of the line. Is it ok?
        >> In time_diff, add the milliseconds in the difference.

    - Status:
        >> Finished.

    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "graph_management.h"

/*
    FILES MANAGEMENT FUNCTIONS
*/

unsigned long nnodes_in_csv(FILE *csv_file) {
    unsigned long nnodes = 0;
    unsigned long nsat = 0;
    char *line = NULL;
    size_t len = 0;
    char *tmpline, *field;
    int i;
    char sat_name[] = "[SAT-AIS]";

    getline(&line, &len, csv_file);
    while (getline(&line, &len, csv_file) != -1) {
        if (len > 21) { // 21 is the number of commas separating the columns of a row
            tmpline = line;
            for (i = 0; i < 7; i++) field = strsep(&tmpline, ",");
            field = strip_quotes(field);
            if (strcmp(field, sat_name) != 0) nnodes++;
            else nsat++;
        }
    }
    printf("Number of nodes: %lu\n", nnodes);
    printf("Number of Satellites: %lu\n", nsat);
    free(line);
    return nnodes;
}

void add_nodes_from_csv(Node *nodes, Path *paths, unsigned long const new_nnodes,
                        unsigned long *nnodes, unsigned long *nedges, unsigned long *npaths,
                        FILE *csv_file) {
    // 1. Initialize auxiliar arrays
    char **shipnames, **scrapping_times;
    int *shiptypes;

    shipnames = (char **) malloc(new_nnodes*sizeof(char*));
    scrapping_times = (char **) malloc(new_nnodes*sizeof(char*));
    shiptypes = (int *) malloc(new_nnodes*sizeof(int));

    if (shipnames == NULL) ExitError("when allocating memory for shipnames\n", 1);
    if (scrapping_times == NULL) ExitError("when allocating memory for scrapping times\n", 2);
    if (shiptypes == NULL) ExitError("when allocating memory for shiptypes\n", 3);

    // 2. Read nodes
    char *line = NULL;
    size_t len;
    char *tmpline, *field;
    char *eptr;
    int i;
    char *lat, *lon, *speed;
    char sat_name[] = "[SAT-AIS]";

    unsigned long index = 0;
    getline(&line, &len, csv_file); // Pass header
    printf("Computing nodes...\n");
    while (getline(&line, &len, csv_file) != -1) {
        if (len > 5) {
            tmpline = line;
            lat = strsep(&tmpline, ",");
            lon = strsep(&tmpline, ",");
            speed = strsep(&tmpline, ",");
            speed = strip_quotes(speed);
            for (i = 0; i < 4; i++) field = strsep(&tmpline, ",");
            field = strip_quotes(field);
            if (strcmp(field, sat_name)) {
                nodes[*nnodes+index].id = index;
                nodes[*nnodes+index].lat = strtod(lat, &eptr);
                nodes[*nnodes+index].lon = strtod(lon, &eptr);
                nodes[*nnodes+index].speed = atoi(speed);
                shipnames[index] = strdup(field);
                for (i = 0; i < 6; i++) field = strsep(&tmpline, ",");
                scrapping_times[index] = strdup(field);
                for (i = 0; i < 7; i++) field = strsep(&tmpline, ",");
                shiptypes[index] = atoi(field+1);
                nodes[*nnodes+index].nedges = 0;
                nodes[*nnodes+index].max_edges = 0;
                index++;
            }
        }
    }

    // 3. Initialize first path of the new data
    unsigned long initial_nedges, initial_npaths;
    initial_nedges = *nedges;
    initial_npaths = *npaths;

    paths[*npaths].start_node.node_id = *nnodes;
    paths[*npaths].start_node.next = NULL;
    paths[*npaths].final_node = &paths[*npaths].start_node;
    paths[*npaths].id = *npaths;
    paths[*npaths].shiptype = shiptypes[0];
    paths[*npaths].len = 1;
    paths[*npaths].min_lon = nodes[*nnodes].lon;
    paths[*npaths].max_lon = nodes[*nnodes].lon;
    paths[*npaths].min_lat = nodes[*nnodes].lat;
    paths[*npaths].max_lat = nodes[*nnodes].lat;
    paths[*npaths].max_paths = 0;
    paths[*npaths].npaths = 0;
    paths[*npaths].to_paths = NULL;

    (*npaths)++;

    // 4. Compute new edges and paths
    char *prev_shipname;
    prev_shipname = strdup(shipnames[0]);
    if (prev_shipname == NULL) ExitError("when copying the previ shipname", 4);
    printf("Computing edges and paths...\n");
    for (index = 1; index < new_nnodes; index++) {
        if (strcmp(shipnames[index], prev_shipname) == 0) {
            nodes[*nnodes+index-1].max_edges = 1;
            nodes[*nnodes+index-1].nedges = 1;
            nodes[*nnodes+index-1].to_nodes = (unsigned long *) malloc(sizeof(unsigned long));
            nodes[*nnodes+index-1].to_times = (double *) malloc(sizeof(double));

            if (nodes[*nnodes+index-1].to_nodes == NULL) ExitError("when allocating memory for to_nodes", 5);
            if (nodes[*nnodes+index-1].to_times == NULL) ExitError("when allocating memory for to_times", 6);

            nodes[*nnodes+index-1].to_nodes[0] = *nnodes + index;
            nodes[*nnodes+index-1].to_times[0] = time_diff(scrapping_times[index-1], scrapping_times[index]);

            update_path_coordinates(&paths[*npaths - 1], &nodes[*nnodes+index]);
            add_path_node(&paths[*npaths - 1], *nnodes + index);
            paths[*npaths - 1].len++;

            (*nedges)++;
        } else {
            prev_shipname = shipnames[index];

            paths[*npaths].start_node.node_id = *nnodes + index;
            paths[*npaths].start_node.next = NULL;
            paths[*npaths].final_node = &paths[*npaths].start_node;
            paths[*npaths].id = *npaths;
            paths[*npaths].shiptype = shiptypes[index];
            paths[*npaths].len = 1;
            paths[*npaths].min_lon = nodes[*nnodes+index].lon;
            paths[*npaths].max_lon = nodes[*nnodes+index].lon;
            paths[*npaths].min_lat = nodes[*nnodes+index].lat;
            paths[*npaths].max_lat = nodes[*nnodes+index].lat;
            paths[*npaths].max_paths = 0;
            paths[*npaths].npaths = 0;
            paths[*npaths].to_paths = NULL;

            (*npaths)++;
        }
    }

    // 5. Update and free allocated memory
    *nnodes = *nnodes + new_nnodes;

    free(shipnames);
    free(scrapping_times);
    free(shiptypes);

    printf("Added %lu nodes, %lu edges and %lu paths.\n", new_nnodes, *nedges-initial_nedges, *npaths-initial_npaths);
    return;
}

void store_nodes(Node *nodes, Path *paths,
                unsigned long nnodes, unsigned long nedges, unsigned long npaths, 
                        char *bin_filename) {
    // 1. Open the binary file
    FILE *bin_file;
    bin_file = fopen(bin_filename, "wb");
    if (bin_file == NULL) ExitError("when opening the binary file", 1);
    printf("The graph to store contains %lu nodes, %lu edges and %lu paths\n", nnodes, nedges, npaths);

    // 2. Header: Number of nodes and edges and paths
    if (fwrite(&nnodes, sizeof(unsigned long), 1, bin_file) +
        fwrite(&nedges, sizeof(unsigned long), 1, bin_file) +
        fwrite(&npaths, sizeof(unsigned long), 1, bin_file)  != 3) {
            ExitError("when writing header to the output binary data file", 2);
        }
    
    // 3. Write nodes
    unsigned long index;
    for(index = 0; index < nnodes; index++) nodes[index].max_edges = nodes[index].nedges;
    if (fwrite(nodes, sizeof(Node), nnodes, bin_file) != nnodes) {
        ExitError("when writing nodes to the output binary data file", 3);
    }

    // 4. Write nodes connections in blocks
    for(index = 0; index < nnodes; index++) if(nodes[index].nedges) {
        if (fwrite(nodes[index].to_nodes, sizeof(unsigned long), nodes[index].nedges, bin_file) != nodes[index].nedges) {
            ExitError("when writing edges to the output binary data file", 4);
        }
    }
    // 5. Write travelling times in blocks
    for(index = 0; index < nnodes; index++) if(nodes[index].nedges) {
        if (fwrite(nodes[index].to_times, sizeof(double), nodes[index].nedges, bin_file) != nodes[index].nedges) {
            ExitError("when writing edges to the output binary data file", 5);
        }
    }
    
    // 6. Write paths
    if (fwrite(paths, sizeof(Path), npaths, bin_file) != npaths) {
        ExitError("when writing paths to the output binary data file", 6);
    }

    // 7. Write paths connections in blocks
    for (index = 0; index < npaths; index++) {
        paths[index].max_paths = paths[index].npaths;
        if (paths[index].npaths) {
            if (fwrite(paths[index].to_paths, sizeof(unsigned long), paths[index].npaths, bin_file) != paths[index].npaths) {
                ExitError("when writing paths connected to the output binary file", 7);
            }
        }
    }

    // 8. Write paths' linked lists
    Path_node *curr_node;
    for (index = 0; index < npaths; index++) {
        if (paths[index].len > 1) {
            curr_node = paths[index].start_node.next;
            do {
                if (fwrite(&curr_node->node_id, sizeof(unsigned long), 1, bin_file) != 1)
                    ExitError("when writing path nodes to the output binary data file", 8);
                curr_node = curr_node->next;
            } while (curr_node != NULL);
        }
    }
    fclose(bin_file);
    return;
}


void store_nodes_shiptype(Node *nodes, Path *paths, int shiptype, unsigned long npaths, char *bin_filename) {
    // 1. Open the binary file
    FILE *bin_file;
    bin_file = fopen(bin_filename, "wb");
    if (bin_file == NULL) ExitError("when opening the binary file", 1);

    // 2. Copy the paths to store
    unsigned long index, nnodes2store, nedges2store, npaths2store;
    nnodes2store = 0;
    nedges2store = 0;
    npaths2store = 0;

    Path *paths2store;
    paths2store = (Path *) malloc(npaths * sizeof(Path));  // Allocating for all in case it is necessary
    if (paths2store == NULL) ExitError("when allocating memory for paths2store", 2);

    for (index = 0; index < npaths; index++) {
        if (paths[index].shiptype != shiptype) continue;
        paths2store[npaths2store] = paths[index];
        nnodes2store = nnodes2store + paths[index].len;
        nedges2store = nedges2store + paths[index].len - 1;
        npaths2store++;
    }

    printf("The graph to store contains %lu nodes, %lu edges and %lu paths\n", nnodes2store, nedges2store, npaths2store);

    // 3. Header: Number of nodes and edges and paths
    if (fwrite(&nnodes2store, sizeof(unsigned long), 1, bin_file) +
        fwrite(&nedges2store, sizeof(unsigned long), 1, bin_file) +
        fwrite(&npaths2store, sizeof(unsigned long), 1, bin_file)  != 3) {
            ExitError("when writing header to the output binary data file", 3);
    }

    // 4. Write nodes
    // 4.1. Update the ids of the nodes
    unsigned long nnodes_stored;
    unsigned long i_node;
    unsigned long *original_ids;
    Path *curr_path;
    Path_node *curr_path_node;

    original_ids = (unsigned long *) malloc (npaths2store * sizeof(unsigned long));
    if (original_ids == NULL) ExitError("When allocating memory for the original ids", 4);

    nnodes_stored = 0;
    for (index = 0; index < npaths2store; index++) {
        curr_path = &paths2store[index];
        curr_path_node = &curr_path->start_node;
        original_ids[index] = curr_path_node->node_id;
        for (i_node = 0; i_node < curr_path->len - 1; i_node++) {
            nodes[curr_path_node->node_id].id = nnodes_stored + i_node;
            nodes[curr_path_node->node_id].to_nodes[0] = nnodes_stored + i_node + 1;
            curr_path_node->node_id = nnodes_stored + i_node;
            curr_path_node = curr_path_node->next;
        }
        nodes[curr_path_node->node_id].id = nnodes_stored + curr_path->len - 1;
        curr_path_node->node_id = nnodes_stored + curr_path->len - 1;
        nnodes_stored = nnodes_stored + curr_path->len;

        if (fwrite(&nodes[original_ids[index]], sizeof(Node), curr_path->len, bin_file) != curr_path->len) {
            ExitError("when writing nodes to the output binary data file", 5);
        }
    }


    // 5. Write nodes connections in blocks
    for (index = 0; index < npaths2store; index++) {
        for (i_node = original_ids[index]; i_node < original_ids[index] + paths2store[index].len; i_node++) if (nodes[i_node].nedges) {
            if (fwrite(nodes[i_node].to_nodes, sizeof(unsigned long), nodes[i_node].nedges, bin_file) != nodes[i_node].nedges) {
                ExitError("when writing edges to the output binary data file", 6);
            }
        }
    }

    // 5. Write travelling times in blocks
    for (index = 0; index < npaths2store; index++) {
        for (i_node = original_ids[index]; i_node < original_ids[index] + paths2store[index].len; i_node++) if (nodes[i_node].nedges) {
            if (fwrite(nodes[i_node].to_times, sizeof(double), nodes[i_node].nedges, bin_file) != nodes[i_node].nedges) {
                ExitError("when writing times to the output binary data file", 7);
            }
        }
    }
    
    // 6. Write paths
    if (fwrite(paths2store, sizeof(Path), npaths2store, bin_file) != npaths2store) {
        ExitError("when writing paths to the output binary data file", 8);
    }

    // 7. Write paths connections in blocks
    // There can not be any connection yet.

    // 8. Write paths' linked lists
    for (index = 0; index < npaths2store; index++) {
        if (paths2store[index].len > 1) {
            curr_path = &paths2store[index];
            curr_path_node = curr_path->start_node.next;
            do {
                if (fwrite(&curr_path_node->node_id, sizeof(unsigned long), 1, bin_file) != 1)
                    ExitError("when writing path nodes to the output binary data file", 9);
                curr_path_node = curr_path_node->next;
            } while (curr_path_node != NULL);
        }
    }
    fclose(bin_file);
    return;
}


void store_nodes_filtered(Node *nodes, Path *paths, double tolerance, unsigned long npaths, char *bin_filename) {
    // 1. Open the binary file
    FILE *bin_file;
    bin_file = fopen(bin_filename, "wb");
    if (bin_file == NULL) ExitError("when opening the binary file", 1);

    // 2. Copy the paths to store
    unsigned long index, nnodes2store, nedges2store, npaths2store;
    nnodes2store = 0;
    nedges2store = 0;
    npaths2store = 0;

    Path *paths2store;
    paths2store = (Path *) malloc(npaths * sizeof(Path));  // Allocating for all in case it is necessary
    if (paths2store == NULL) ExitError("when allocating memory for paths2store", 2);

    double lon_1, lat_1, lon_2, lat_2;
    double dist;
    for (index = 0; index < npaths; index++) {
        lon_1 = nodes[paths[index].start_node.node_id].lon;
        lat_1 = nodes[paths[index].start_node.node_id].lat;
        lon_2 = nodes[paths[index].final_node->node_id].lon;
        lat_2 = nodes[paths[index].final_node->node_id].lat;
        dist = distance_km(lon_1, lat_1, lon_2, lat_2);
        if (dist < tolerance) {
            continue;
        }
        paths2store[npaths2store] = paths[index];
        nnodes2store = nnodes2store + paths[index].len;
        nedges2store = nedges2store + paths[index].len - 1;
        npaths2store++;
    }

    printf("The graph to store contains %lu nodes, %lu edges and %lu paths\n", nnodes2store, nedges2store, npaths2store);

    // 3. Header: Number of nodes and edges and paths
    if (fwrite(&nnodes2store, sizeof(unsigned long), 1, bin_file) +
        fwrite(&nedges2store, sizeof(unsigned long), 1, bin_file) +
        fwrite(&npaths2store, sizeof(unsigned long), 1, bin_file)  != 3) {
            ExitError("when writing header to the output binary data file", 3);
    }

    // 4. Write nodes
    // 4.1. Update the ids of the nodes
    unsigned long nnodes_stored;
    unsigned long i_node;
    unsigned long *original_ids;
    Path *curr_path;
    Path_node *curr_path_node;

    original_ids = (unsigned long *) malloc (npaths2store * sizeof(unsigned long));
    if (original_ids == NULL) ExitError("When allocating memory for the original ids", 4);

    nnodes_stored = 0;
    for (index = 0; index < npaths2store; index++) {
        curr_path = &paths2store[index];
        curr_path_node = &curr_path->start_node;
        original_ids[index] = curr_path_node->node_id;
        for (i_node = 0; i_node < curr_path->len - 1; i_node++) {
            nodes[curr_path_node->node_id].id = nnodes_stored + i_node;
            nodes[curr_path_node->node_id].to_nodes[0] = nnodes_stored + i_node + 1;
            curr_path_node->node_id = nnodes_stored + i_node;
            curr_path_node = curr_path_node->next;
        }
        nodes[curr_path_node->node_id].id = nnodes_stored + curr_path->len - 1;
        curr_path_node->node_id = nnodes_stored + curr_path->len - 1;
        nnodes_stored = nnodes_stored + curr_path->len;

        if (fwrite(&nodes[original_ids[index]], sizeof(Node), curr_path->len, bin_file) != curr_path->len) {
            ExitError("when writing nodes to the output binary data file", 5);
        }
    }

    // 5. Write nodes connections in blocks
    for (index = 0; index < npaths2store; index++) {
        for (i_node = original_ids[index]; i_node < original_ids[index] + paths2store[index].len; i_node++) if (nodes[i_node].nedges) {
            if (fwrite(nodes[i_node].to_nodes, sizeof(unsigned long), nodes[i_node].nedges, bin_file) != nodes[i_node].nedges) {
                ExitError("when writing edges to the output binary data file", 6);
            }
        }
    }

    // 5. Write travelling times in blocks
    for (index = 0; index < npaths2store; index++) {
        for (i_node = original_ids[index]; i_node < original_ids[index] + paths2store[index].len; i_node++) if (nodes[i_node].nedges) {
            if (fwrite(nodes[i_node].to_times, sizeof(double), nodes[i_node].nedges, bin_file) != nodes[i_node].nedges) {
                ExitError("when writing times to the output binary data file", 7);
            }
        }
    }
    
    // 6. Write paths
    if (fwrite(paths2store, sizeof(Path), npaths2store, bin_file) != npaths2store) {
        ExitError("when writing paths to the output binary data file", 8);
    }

    // 7. Write paths connections in blocks
    // There can not be any connection yet.

    // 8. Write paths' linked lists
    for (index = 0; index < npaths2store; index++) {
        if (paths2store[index].len > 1) {
            curr_path = &paths2store[index];
            curr_path_node = curr_path->start_node.next;
            do {
                if (fwrite(&curr_path_node->node_id, sizeof(unsigned long), 1, bin_file) != 1)
                    ExitError("when writing path nodes to the output binary data file", 9);
                curr_path_node = curr_path_node->next;
            } while (curr_path_node != NULL);
        }
    }
    fclose(bin_file);
    return;
}


void read_nodes(Node **nodes_ptr, Path **paths_ptr,
                unsigned long *nnodes, unsigned long *nedges, unsigned long *npaths, 
                char *bin_filename) {
    // 1. Open the binary file
    FILE *bin_file;
    bin_file = fopen(bin_filename, "rb");
    if (bin_file == NULL) ExitError("when opening the binary file", 1);

    // 2. Header: Number of nodes, edges and paths
    if (fread(nnodes, sizeof(unsigned long), 1, bin_file) + 
        fread(nedges, sizeof(unsigned long), 1, bin_file) +
        fread(npaths, sizeof(unsigned long), 1, bin_file) != 3){
        ExitError("when reading the header of the binary data file", 2);
    }
    
    // 3. Read nodes
    Node *nodes;
    nodes = (Node *) malloc(*nnodes * sizeof(Node));
    if (nodes == NULL) ExitError("when allocating memory for nodes", 3);

    if (fread(nodes, sizeof(Node), (*nnodes), bin_file) != (*nnodes)) {
        ExitError("when reading nodes from the input binary data file", 4);
    }

    // 4. Read nodes connections in blocks
    unsigned long index;
    for(index = 0; index < (*nnodes); index++) if(nodes[index].nedges) {
        nodes[index].to_nodes = (unsigned long *) malloc(nodes[index].nedges * sizeof(unsigned long));
        if (nodes[index].to_nodes == NULL) ExitError("when allocating memory for the connected nodes vector", 5);
        if (fread(nodes[index].to_nodes, sizeof(unsigned long), nodes[index].nedges, bin_file) != nodes[index].nedges)
            ExitError("when reading edges from the input binary data file", 6);
    }
    // 5. Read travelling times in blocks
    for(index = 0; index < (*nnodes); index++) if(nodes[index].nedges) {
        nodes[index].to_times = (double *) malloc(nodes[index].nedges * sizeof(double));
        if (nodes[index].to_times == NULL) ExitError("when allocating memory for the travelling times vector", 7);
        if (fread(nodes[index].to_times, sizeof(double), nodes[index].nedges, bin_file) != nodes[index].nedges)
            ExitError("when reading times from the input binary data file", 8);
    }
    
    // 6. Read paths
    Path *paths;
    paths = (Path *) malloc(*npaths * sizeof(Path));
    if (paths == NULL) ExitError("when allocating memory for nodes", 9);
    if (fread(paths, sizeof(Path), (*npaths), bin_file) != (*npaths)) {
        ExitError("when reading paths from the input binary data file", 10);
    }

    // 7. Read paths connections in blocks
    for (index = 0; index < *npaths; index++) {
        if (paths[index].npaths) {
            paths[index].to_paths = (unsigned long *) malloc(paths[index].npaths*sizeof(unsigned long));
            if (paths[index].to_paths == NULL) ExitError("when allocating memory for the paths connected", 11);
            if (fread(paths[index].to_paths, sizeof(unsigned long), paths[index].npaths, bin_file) != paths[index].npaths) {
                ExitError("when reading paths connected from the input binary data file", 12);
            }
        }
    }

    // 8. Read paths' linked lists
    unsigned long node_id, i;
    for (index = 0; index < *npaths; index++) {
        paths[index].final_node = &paths[index].start_node;
        if (paths[index].len > 1) {
            for (i = 0; i < paths[index].len - 1; i++) {
                if (fread(&node_id, sizeof(unsigned long), 1, bin_file) != 1) {
                    ExitError("when reading path nodes from the input binary data file", 13);
                }
                add_path_node(&paths[index], node_id);
            }
        }
    }

    fclose(bin_file);
    *nodes_ptr = nodes;
    *paths_ptr = paths;
    return;
}

/*
    NODES MANAGEMENT AND TESTING
*/

void free_nodes(Node *nodes, unsigned long nnodes) {
    unsigned long index;
    for (index = 0; index < nnodes; index++) if (nodes[index].nedges) {
        free(nodes[index].to_nodes);
        free(nodes[index].to_times);
    }
    free(nodes);
    return;
}

void node_info(Node *nodes, unsigned long index) {
    unsigned i;
    printf("The node %lu has:\n", index);
    printf("    - id: %lu\n    - lat, lon: %g, %g\n    - speed: %d\n    - nedges: %hu\n    - max_edges: %hu\n",
        nodes[index].id, nodes[index].lat, nodes[index].lon, nodes[index].speed, nodes[index].nedges, nodes[index].max_edges);
    if (nodes[index].nedges) {
        printf("            Connected to:    ");
        for (i = 0; i < nodes[index].nedges; i++) printf("%lu    ", nodes[index].to_nodes[i]);
        printf("\n            With respective travel times:    ");
        for (i = 0; i < nodes[index].nedges; i++) printf("%g    ", nodes[index].to_times[i]);
    }
    printf("\n");
    return;
}

void node_edge_verification(Node *nodes, unsigned long index) {
    unsigned i;
    node_info(nodes, index);
    if (nodes[index].nedges) {
        printf("        CONNECTED NODES INFORMATION:\n");
        for (i = 0; i < nodes[index].nedges; i++) node_info(nodes, nodes[index].to_nodes[i]);
    }
    return;
}
/*
    PATHS MANAGEMENT AND TESTING
*/
void update_path_coordinates(Path *path, Node *new_node) {
    if (new_node->lon < path->min_lon) {
        path->min_lon = new_node->lon;
    } else if (new_node->lon > path->max_lon) {
        path->max_lon = new_node->lon;
    }

    if (new_node->lat < path->min_lat) {
        path->min_lat = new_node->lat;
    } else if (new_node->lat > path->max_lat) {
        path->max_lat = new_node->lat;
    }

    return;
}

void add_path_node(Path *curr_path, unsigned long new_node_id) {
    Path_node *new_path_node;
    new_path_node = (Path_node *) malloc(sizeof(Path_node));
    if (new_path_node == NULL) ExitError("when allocating memory for new_path_node", 1);
    new_path_node->node_id = new_node_id;
    new_path_node->next = NULL;
    curr_path->final_node->next = new_path_node;
    curr_path->final_node = new_path_node;
    return;
}

ST_counter *count_shiptypes(Path *paths, unsigned long npaths) {
    ST_counter *first_ST, *head_ST, *curr_ST;

    first_ST = (ST_counter *) malloc(sizeof(ST_counter));
    if (first_ST == NULL) ExitError("when allocating memory for the first ST counter", 1);

    first_ST->shiptype = paths[0].shiptype;
    first_ST->npaths = 1;
    first_ST->next = NULL;
    head_ST = first_ST;

    unsigned long index;
    int new_shiptype;
    for (index = 1; index < npaths; index++) {
        new_shiptype = paths[index].shiptype;
        if (new_shiptype < head_ST->shiptype) {
            ST_counter *new_ST;
            new_ST = (ST_counter *) malloc(sizeof(ST_counter));
            if (new_ST == NULL) ExitError("when allocating memory for a new ST counter", 2);
            new_ST->shiptype = new_shiptype;
            new_ST->npaths = 1;
            new_ST->next = head_ST;
            head_ST = new_ST;
            continue;
        }

        curr_ST = head_ST;
        while (curr_ST != NULL) {
            if (new_shiptype == curr_ST->shiptype) {
                curr_ST->npaths++;
                break;
            }

            if (curr_ST->next == NULL || new_shiptype < curr_ST->next->shiptype) {
                ST_counter *new_ST;
                new_ST = (ST_counter *) malloc(sizeof(ST_counter));
                if (new_ST == NULL) ExitError("when allocating memory for a new ST counter", 3);
                new_ST->shiptype = new_shiptype;
                new_ST->npaths = 1;
                new_ST->next = curr_ST->next;
                curr_ST->next = new_ST;
                break;
            }

            curr_ST = curr_ST->next;
        }
    }
    return head_ST;
}


void free_paths(Path *paths, unsigned long npaths) {
    Path_node *curr_path_node;
    unsigned long index;

    for (index = 0; index < npaths; index++) {
        curr_path_node = paths[index].start_node.next;
        while (curr_path_node != NULL) {
            Path_node *aux_path_node = curr_path_node;
            curr_path_node = curr_path_node->next;
            free(aux_path_node);
        }
        if (paths[index].npaths) free(paths[index].to_paths);
    }
    free(paths);
    return;
}

void path_info(Path *paths, unsigned long index) {
    unsigned i;
    if (paths[index].len == 0) {
        printf("Path %lu doesn't exist\n", index);
    } else {
        printf("\n- Path id: %lu\n  - Starts at node: %lu\n  - Shiptype: %d\n  - Min/Max Long: %g/%g\n  - Min/Max Lat: %g/%g\n  - npaths: %lu\n  - max_paths: %lu\n  - len: %lu\n",
                    paths[index].id, paths[index].start_node.node_id, paths[index].shiptype, 
                    paths[index].min_lon, paths[index].max_lon, paths[index].min_lat, paths[index].max_lat,
                    paths[index].npaths, paths[index].max_paths, paths[index].len);
        if (paths[index].npaths) {
            printf("     - Connected to:  ");
            for (i = 0;  i < paths[index].npaths; i++) printf("%lu  ", paths[index].to_paths[i]);
            printf("\n");
        }
    }
    return;
}

void path_nodes_info(Path_node *curr_node, int n) {
    int i = 0;
    do {
        printf("%lu ---> ", curr_node->node_id);
        curr_node = curr_node->next;
        i++;
    } while (curr_node != NULL && i < n);
    printf("\n");
    return;
}

/*
    CALCULATION FUNCTIONS
*/

double time_diff(char *initial_scraping_time, char *final_scraping_time) {
    struct tm initial_time = {0}, final_time = {0};
    float initial_ms, final_ms;

    sscanf(initial_scraping_time, "%d-%d-%d %d:%d:%d.%f", 
            &initial_time.tm_year, &initial_time.tm_mon, &initial_time.tm_mday,
            &initial_time.tm_hour, &initial_time.tm_min, &initial_time.tm_sec, &initial_ms);

    sscanf(final_scraping_time, "%d-%d-%d %d:%d:%d.%f", 
            &final_time.tm_year, &final_time.tm_mon, &final_time.tm_mday,
            &final_time.tm_hour, &final_time.tm_min, &final_time.tm_sec, &final_ms);

    // Adjust struct tm fields
    initial_time.tm_year -= 1900;
    initial_time.tm_mon--;
    final_time.tm_year -= 1900;
    final_time.tm_mon--;

    // Compute time difference in seconds
    time_t initial_time_seconds = mktime(&initial_time);
    time_t final_time_seconds = mktime(&final_time);

    double time_difference = difftime(final_time_seconds, initial_time_seconds);

    return time_difference;
}

double distance_km(double lon_1, double lat_1, double lon_2, double lat_2) {
    double R;
    double lat_1_rad, lat_2_rad;
    double Delta_lat, Delta_lon;
    double a, c, dist;

    R = 6371; // km
    lat_1_rad = lat_1 * M_PI / 180; // rad
    lat_2_rad = lat_2 * M_PI / 180; // rad
    Delta_lat = (lat_2 - lat_1) * M_PI / 180; // rad
    Delta_lon = (lon_2 - lon_1) * M_PI / 180; // rad
    a = sin(Delta_lat/2) * sin(Delta_lat/2) +
        cos(lat_1_rad) * cos(lat_2_rad) *
        sin(Delta_lon/2) * sin(Delta_lon/2);
    c = 2 * atan2(sqrt(a), sqrt(1-a));
    dist = R * c; // km

    return dist;
}


/*
    AUXILIAR FUNCTIONS
*/

void ExitError(const char *message, int exit_code) {
    fprintf(stderr, "Error: %s\n", message);
    exit(exit_code);
}

char *strip_quotes(char *str) {
    size_t len = strlen(str);
    if (len > 1 && str[0] == '"' && str[len - 1] == '"') {
        str[len - 1] = '\0';
        str++;
    }
    return str;
}

void print_progress_bar (unsigned long iteration, unsigned long total_iterations, unsigned short *last_pc) {
    unsigned short pc;
    int i;

    if (total_iterations == 0) return;
    pc = 100 * iteration / total_iterations;
    if (pc == 0) {
        printf("\r");
        for (i = 100; i > pc; i--) printf("-");
        printf("  [%d %%]", pc);
    } else if (pc > *last_pc) {
        printf("\r");
        for (i = 0; i <= pc; i++) printf("#");
        for (i = 100; i > pc; i--) printf("-");
        printf("  [%d %%]", pc);
        *last_pc = pc;
    }
    if (pc == 100) printf("\n");
    return;
}
