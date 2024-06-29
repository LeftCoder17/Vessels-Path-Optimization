#ifndef INTERSECTIONS_H
#define INTERSECTIONS_H

/*
    PATH MANAGAMENT
*/

// Check whether it is necessary or not to compute a pair of graphs intersections
unsigned short need_compute_paths(Path *paths, unsigned long i_path_1, unsigned long i_path_2);

// Add to the connecteds paths of Path 1 all the Paths until last Path 2
void checked_paths(Path *paths, unsigned long i_path_1, unsigned long last_i_path_2);


/*
    INTERSECTIONS MANAGEMENT
*/

/*
 * Return the intersection type between the edges p1-q1 and p2-q2.
 * It also stores the value of t and u where the pointers *t and *u indicate.
*/
unsigned short identify_intersection(Node *p1, Node *q1, Node *p2, Node *q2, double *t, double *u);

/*
 * Computes the intersection and creates the corresponding nodes and edges according to the intersection_type.
 * It also sums 1 to the length of both paths 1 and 2.
*/
void add_intersection(Node **nodes_ptr, unsigned long *nnodes, unsigned long *max_nnodes, unsigned long *nedges,
                    unsigned long p1_id, unsigned long q1_id, unsigned long p2_id, unsigned long q2_id, 
                    unsigned short intersection_type, double t, double u,
                    Path *paths, unsigned long i_path_1, unsigned long i_path_2,
                    Path_node *p1, Path_node *p2);


#endif