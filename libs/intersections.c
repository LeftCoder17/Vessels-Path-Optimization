/*
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    $$$$$$$$$$$    INTERSECTIONS.C VERSION 1.0    $$$$$$$$$$$
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

    - Usage:
        >> Through the header file "intersections.h"

    - Comments:
        >> Only intersection type 1 is implemented.
        >> Intersection point computation: https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect/1201356#1201356
        >> Two edges can intersect if and only if they belong to the same shiptype, different path and have not been joined before.
            Thus, it is possible to append new data and compute again the intersections without making redundant comprovations.
        >> The new edge takes the slowest speed.
        >> The intersections type 1 are implemented once they are detected. 
    
    - Further development:
        >> Implement more intersections types.
        >> Check if it is necessary to take into account any cosinus factor computing the orientation, as depending on the latitude, the longitude lines are closer. Euclidean space, not the plane.
        >> Check if the travel times need any modification due to the euclidean space, that is, checking that the proportions are the same as in a 2D plane.

    - Status:
        >> Unfinished
        >> It doesn't compute intersections type 0.

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
#include "intersections.h"

/*
    PATH MANAGEMENT
*/

unsigned short need_compute_paths(Path *paths, unsigned long i_path_1, unsigned long i_path_2) {
    if (paths[i_path_1].shiptype != paths[i_path_2].shiptype) {
        return 0;
    }
    if (paths[i_path_2].id > paths[i_path_1].npaths) {
        if (paths[i_path_1].min_lon > paths[i_path_2].max_lon ||
            paths[i_path_1].max_lon < paths[i_path_2].min_lon ||
            paths[i_path_1].min_lat > paths[i_path_2].max_lat ||
            paths[i_path_1].max_lat < paths[i_path_2].min_lat ) {
            return 0;
        } else return 1;
    } else {
        printf("Path %lu npaths: %lu\n", i_path_1, paths[i_path_1].npaths);
        printf("Checking Path %lu\n", i_path_2);
        ExitError("when checking whether it is necessary to compute paths intersections or not", 1);
    }
    return 0;
}

void checked_paths(Path *paths, unsigned long i_path_1, unsigned long last_i_path_2) {
    unsigned long index;
    if (paths[i_path_1].npaths == 0) {
        paths[i_path_1].max_paths = last_i_path_2;
        paths[i_path_1].to_paths = (unsigned long *) malloc (paths[i_path_1].max_paths * sizeof(unsigned long));
        if (paths[i_path_1].to_paths == NULL) ExitError("when allocating memory for the connected paths", 1);
        
        for (index = 0; index < i_path_1; index++) paths[i_path_1].to_paths[index] = index;
        for (index = i_path_1 + 1; index <= last_i_path_2; index++) paths[i_path_1].to_paths[index-1] = index;
        paths[i_path_1].npaths = last_i_path_2;
    } else {
        paths[i_path_1].max_paths = last_i_path_2;
        paths[i_path_1].to_paths = (unsigned long *) realloc(paths[i_path_1].to_paths, paths[i_path_1].max_paths * sizeof(unsigned long));
        if (paths[i_path_1].to_paths == NULL) ExitError("when reallocating memory for the connected paths", 2);

        for (index = paths[i_path_1].npaths + 1; index <= last_i_path_2; index++) paths[i_path_1].to_paths[index - 1] = index;
        paths[i_path_1].npaths = last_i_path_2;
    }
}

/*
    INTERSECTIONS MANAGEMENT
*/

unsigned short identify_intersection(Node *p1, Node *q1, Node *p2, Node *q2, double *t, double *u) {
    // 1. Check p1 is not a common node with either p2 or q2.
    if (p1->id == p2->id || p1->id == q2->id) return 0;

    // 2. Identify intersection
    double p1q1Xp2q2, p1p2Xp2q2;
    p1q1Xp2q2 = (q1->lon - p1->lon) * (q2->lat - p2->lat) 
                - (q1->lat - p1->lat) * (q2->lon - p2->lon);

    p1p2Xp2q2 = (p2->lon - p1->lon) * (q2->lat - p2->lat)
                - (p2->lat - p1->lat) * (q2->lon - p2->lon);

    if (p1q1Xp2q2 != 0) { // Not parallel
        double p1p2Xp1q1;
        p1p2Xp1q1 = (p2->lon - p1->lon) * (q1->lat - p1->lat)
                    - (p2->lat - p1->lat) * (q1->lon - p1->lon);
        *t = p1p2Xp2q2 / p1q1Xp2q2;
        *u = p1p2Xp1q1 / p1q1Xp2q2;

        if (*t >= 0 && *t <= 1 && *u >= 0 && *u <= 1) {
            if (*t > 0 && *t < 1 && *u > 0 && *u < 1) return 1;
            else if (*t > 0 && *t < 1 && *u == 0) return 2;
            else if (*t > 0 && *t < 1 && *u == 1) return 3;
            else if (*t == 0 && *u > 0 && *u < 1) return 4;
            else if (*t == 1 && *u > 0 && *u < 1) return 5;
            else if (*t == 0 && *u == 0) return 6;
            else if (*t == 1 && *u == 0) return 7;
            else if (*t == 0 && *u == 1) return 8;
            else return 9; // if (*t == 1 && *u == 1)
        } else return 0;

    } else { // Parallel
        if (p1p2Xp2q2 == 0) { // colinear
            double p1q1_dot_p1q1 = (q1->lon - p1->lon) * (q1->lon - p1->lon) + (q1->lat - p1->lat) * (q1->lat - p1->lat);
            double p2q2_dot_p1q1 = (q2->lon - p2->lon) * (q1->lon - p1->lon) + (q2->lat - p2->lat) * (q1->lat - p1->lat);
            double p1p2_dot_p1q1 = (p2->lon - p1->lon) * (q1->lon - p1->lon) + (p2->lat - p1->lat) * (q1->lat - p1->lat);
            double t0 = p1p2_dot_p1q1 / p1q1_dot_p1q1;
            double t1 = t0 + p2q2_dot_p1q1 / p1q1_dot_p1q1;
            *t = t0;
            *u = t1;
            
            if (t0 > 0 && t0 < 1 && t1 > 1) return 10;
            else if (t0 > 0 && t0 < 1 && t1 < 0) return 11;
            else if (t0 > 1 && t1 > 0 && t1 < 1) return 12;
            else if (t0 < 0 && t1 > 0 && t1 < 1) return 13;
            else if (t0 > 0 && t0 < 1 && t1 > 0 && t1 < 1 && t1 > t0) return 14;
            else if (t0 > 0 && t0 < 1 && t1 > 0 && t1 < 1 && t1 < t0) return 15;
            else if (t0 < 0 && t1 > 1) return 16;
            else if (t0 > 1 && t1 < 0) return 17;
            else return 0; // disjoint

        } else return 0; // Parallel but not colinear
    }
}

void add_intersection(Node **nodes_ptr, unsigned long *nnodes, unsigned long *max_nnodes, unsigned long *nedges,
                    unsigned long p1_id, unsigned long q1_id, unsigned long p2_id, unsigned long q2_id, 
                    unsigned short intersection_type, double t, double u,
                    Path *paths, unsigned long i_path_1, unsigned long i_path_2,
                    Path_node *p1, Path_node *p2) {
    if (intersection_type != 1) return;
    else {
        // 1. Select nodes and initialize the new one
        Node *node_p1 = &(*nodes_ptr)[p1_id];
        Node *node_q1 = &(*nodes_ptr)[q1_id];
        Node *node_p2 = &(*nodes_ptr)[p2_id];
        Node *node_q2 = &(*nodes_ptr)[q2_id];
        Node *new_node;
        new_node = (Node *) malloc(sizeof(Node));
        if (new_node == NULL) ExitError("when allocating memory for the new node", 1);

        // 2. Assign parameters to the new node
        new_node->id = *nnodes;
        new_node->lat = node_p1->lat + t * (node_q1->lat - node_p1->lat);
        new_node->lon = node_p1->lon + t * (node_q1->lon - node_p1->lon);
        new_node->speed = node_p1->speed > node_p2->speed ? node_p1->speed : node_p2->speed; // Canviar a quedarte amb la maxima.
        new_node->nedges = 2;
        new_node->max_edges = 2;
        new_node->to_nodes = (unsigned long *) malloc(new_node->max_edges * sizeof(unsigned long));
        new_node->to_times = (double *) malloc(new_node->max_edges * sizeof(double));
        if (new_node->to_nodes == NULL) ExitError("when allocating memory for the new node connected nodes", 2);
        if (new_node->to_times == NULL) ExitError("when allocating memory for the new node travelling times", 3);

        new_node->to_nodes[0] = node_q1->id;
        new_node->to_nodes[1] = node_q2->id;

        int i, j;
        i = 0;
        while (node_p1->to_nodes[i] != q1_id) i++;
        j = 0;
        while (node_p2->to_nodes[j] != q2_id) j++;

        double edge1 = node_p1->to_times[i];
        double edge2 = node_p2->to_times[j];

        new_node->to_times[0] = (1 - t) * node_p1->to_times[i];
        new_node->to_times[1] = (1 - u) * node_p2->to_times[j];

        node_p1->to_nodes[i] = new_node->id;
        node_p2->to_nodes[j] = new_node->id;
        node_p1->to_times[i] = t * node_p1->to_times[i];
        node_p2->to_times[j] = u * node_p2->to_times[j];

        // 3. Update paths 1 and 2
        
        paths[i_path_1].len++;
        paths[i_path_2].len++;

        Path_node *new_path_node_1, *new_path_node_2;
        new_path_node_1 = (Path_node *) malloc(sizeof(Path_node));
        new_path_node_2 = (Path_node *) malloc(sizeof(Path_node));
        if (new_path_node_1 == NULL) ExitError("when allocating memory for the new path node", 4);
        if (new_path_node_2 == NULL) ExitError("when allocating memory for the new path node", 5);
        
        new_path_node_1->node_id = new_node->id;
        new_path_node_1->next = p1->next;
        p1->next = new_path_node_1;

        new_path_node_2->node_id = new_node->id;
        new_path_node_2->next = p2->next;
        p2->next = new_path_node_2;

        // 4. Update nnodes and nedges
        *nedges = *nedges + 2;
        (*nnodes)++;

        // 5. Add the new node to nodes
        if (*nnodes > *max_nnodes) {
            *max_nnodes = *max_nnodes * 2;
            (*nodes_ptr) = (Node *) realloc((*nodes_ptr), *max_nnodes * sizeof(Node));
            if (*nodes_ptr == NULL) ExitError("when reallocating memory for the nodes array", 6);
        }
        (*nodes_ptr)[new_node->id] = *new_node;
    }
}
