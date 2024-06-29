#ifndef GRAPH_MANAGEMENT_H
#define GRAPH_MANAGEMENT_H
/*
    STRUCTURES TO STORE DATA
*/
// Stores all the relevant information about a node
typedef struct {
    unsigned long id;
    double lat, lon;
    int speed;
    unsigned nedges;
    unsigned max_edges;
    unsigned long *to_nodes;
    double *to_times;
} Node;

// Stores the elements of the linked list with the nodes of a path.
typedef struct path_node {
    unsigned long node_id;
    struct path_node *next;
} Path_node;

// Stores all the relevant information about a path.
typedef struct path_start {
    Path_node start_node;
    Path_node *final_node;
    int shiptype;
    unsigned long id;
    double min_lon, max_lon;
    double min_lat, max_lat;
    unsigned long npaths;
    unsigned long max_paths;
    unsigned long *to_paths;
    unsigned long len;
} Path;

// Stores the shiptype and the number of paths of that shiptype
typedef struct shiptype_counter {
    int shiptype;
    unsigned long npaths;
    struct shiptype_counter *next;
} ST_counter;

/*
    FILES MANAGEMENT FUNCTIONS
*/
// Count the number of nodes in a csv file and returns the number.
unsigned long nnodes_in_csv(FILE *csv_file);

/*
Computes the new nodes from a csv file and appends from position nnodes the new_nnodes from the file.
Computes the new paths of nodes and adds them to paths.
It updates the value of nnodes, npaths and nedges.
*/
void add_nodes_from_csv(Node *nodes, Path *paths, unsigned long const new_nnodes,
                        unsigned long *nnodes, unsigned long *nedges, unsigned long *npaths,
                        FILE *csv_file);

// Stores all the nodes and paths in a binary file
void store_nodes(Node *nodes, Path *paths,
                unsigned long nnodes, unsigned long nedges, unsigned long npaths, 
                        char *bin_filename);

// Stores all the nodes and paths of the shiptype in a binary file. Only applicable for not crossed paths graphs.
void store_nodes_shiptype(Node *nodes, Path *paths, int shiptype, unsigned long npaths, char *bin_filename);

// Stores all the nodes and paths of the shiptype in a binary file whose distance between
// the initial and final node of the path is greater than a tolerance. Only applicable for not crossed paths graphs.
void store_nodes_filtered(Node *nodes, Path *paths, double tolerance, unsigned long npaths, char *bin_filename);

// Reads a stored graph in bin_filename and stores it in nodes and path.
void read_nodes(Node **nodes_ptr, Path **paths_ptr,
                unsigned long *nnodes, unsigned long *nedges, unsigned long *npaths, 
                char *bin_filename);

/*
    NODES MANAGEMENT AND TESTING
*/
// Free all the memory allocated related to the nodes
void free_nodes(Node *nodes, unsigned long nnodes);

// Prints all the info of the node in position index
void node_info(Node *nodes, unsigned long index);

// Prints all the info of the node in position index and its connected nodes
void node_edge_verification(Node *nodes, unsigned long index);

/*
    PATHS MANAGEMENT
*/
// Updates the max/min coordinates of the path
void update_path_coordinates(Path *path, Node *new_node);

// Creates a new Path_node with the new_node_id and connects the curr_path_node to this new one
void add_path_node(Path *curr_path, unsigned long new_node_id);

// Count the shiptypes among all the paths
ST_counter *count_shiptypes(Path *paths, unsigned long npaths);

// Free all the memory allocated related to the paths
void free_paths(Path *paths, unsigned long npaths);

// Prints all the info of the Path in position index
void path_info(Path *paths, unsigned long index);

// Prints the current node id to, at least, the next n nodes.
void path_nodes_info(Path_node *curr_node, int n);

/*
    CALCULATION FUNCTIONS
*/

// Returns the seconds of difference between two times in "%year-%month-%day %h:%min:%s.%ms" format
double time_diff(char *initial_scraping_time, char *final_scraping_time);

// Returns the distance, in km, between two coordinates, using the haversie formula
double distance_km(double lon_1, double lat_1, double lon_2, double lat_2);


/*
    AUXILIAR FUNCTIONS
*/
// It prints error message indicating the code and exits the program
void ExitError(const char *error_message, int error_code);

// It strips the " symbols at the beggining and end of a string
char *strip_quotes(char *str);

// Updates every 1 % and prints the current state fraction done of the whole iterations to do.
void print_progress_bar (unsigned long iteration, unsigned long total_iterations, unsigned short *last_pc);

#endif