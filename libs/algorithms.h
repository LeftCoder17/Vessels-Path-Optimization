#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <stdbool.h>
#include "metrics.h"

/*
    ENUMERATIONS
*/
// This enumeration the heuristic codes
enum Heuristic {Dij, Hav, SLC, EqApp, Vinc};

// This enumeration the possible final states of a node
enum Final_state {NotVis, InPQ, Ext, InSol};

/*
    STRUCTURES TO MANAGE THE ASTAR ALGORITHM
*/
// This structure stores, for every element in the solution path, its parent and the distance to it.
typedef struct {
    double g;
    unsigned long parent;
} AStarPath;

// This structure stores information about a node in order to control the state of the elements in the Priority Queue.
typedef struct {
    double f;
    bool InPQ;
    bool extended;
} AStarControlData;

/*
    STRUCTURES TO MANAGE THE PRIORITY QUEUE
*/

typedef struct Linked_Element_PQ {
    unsigned long node_id;
    struct Linked_Element_PQ *next;
} Linked_Element_PQ;


// This structure stores the information about the current state of the Binary Heap.
typedef struct {
    unsigned l, r;
    unsigned long  *bh_tree;
} Binary_Heap_PQ;


/*
    MACROS FOR THE BINARY HEAP TREE
*/
// Returns the id of the node stored in position d,p in the Binary Heap.
#define dp(d, p)     ((unsigned long)((1UL<< (d)) - 1UL) + p)

// Returns the id of the parent of the node stored in position d,p in the Binary Heap.
#define parent(d, p) ((unsigned long)((1UL<< (d-1)) - 1UL) + (int) p/2)

// Returns the id of the left child of the node stored in position d,p in the Binary Heap.
#define lchild(d, p) ((unsigned long)((1UL<< (d+1)) - 1UL) + 2*p)

// Returns the id of the right child of the node stored in position d,p in the Binary Heap.
#define rchild(d, p) ((unsigned long)((1UL<< (d+1)) - 1UL) + 2*p+1)

/*
    DISTANCES CALCULATIONS
*/

// Returns the squared distance between two positions in the globe.
double nodes_squared_distance(double initial_lat, double initial_lon, double final_lat, double final_lon);

// Applies the heuristic function corresponding to the code of the input.
double heuristic(int heuristic_code, int speed, double initial_lat, double initial_lon, double final_lat, double final_lon);

// Returns the heuristic value = 0, as using the Dijkstra algorithm.
double heuristic_dijkstra();

// Return the heuristic value using Haversine's formula
double heuristic_haversine(int speed, double initial_lat, double initial_lon, double final_lat, double final_lon);

// Return the heuristic value using Spherical Law of Cosines' formula
double heuristic_cosines(int speed, double initial_lat, double initial_lon, double final_lat, double final_lon);

// Return the heuristic value using Equirectangular approximation' formula
double heuristic_equirectangular(int speed, double initial_lat, double initial_lon, double final_lat, double final_lon);

// Return the heuristic value using Vincenty's formula
//double heuristic_vincenty(int speed, double initial_lat, double initial_lon, double final_lat, double final_lon);

/*
    LINKED LIST MANAGEMENT
*/
// Enqueues the id of the new node in the Linked list Priority Queue.
bool enqueue_ll(unsigned long node_id, Linked_Element_PQ **PQ, AStarControlData *Queue_control);

// Dequeues the id of the first node in the Linked list Priority Queue and returns this id.
unsigned long dequeue_ll(Linked_Element_PQ **PQ);

// Moves an already enqueued node in the Linked list Priority Queue to its new position after changing its cost.
void requeue_ll(unsigned long node_id, Linked_Element_PQ **PQ, AStarControlData *Queue_control);

// Show the first N enqueued nodes
void show_ll(Linked_Element_PQ *PQ, int n, AStarControlData *Queue_control);

/*
    BINARY HEAP TREE MANAGEMENT
*/
// Enqueues the id of the new node in the Binary Heap Priority Queue.
bool enqueue_bh(unsigned long i_node, Binary_Heap_PQ *PQ, AStarControlData *Queue_control);

// Dequeues the id of the first node in the Binary Heap Priority Queue and returns this id.
unsigned long dequeue_bh(Binary_Heap_PQ *PQ, AStarControlData *Queue_control);

// Moves an already enqueued node in the Binary Heap Priority Queue to its new position after changing its cost.
void requeue_bh(unsigned long i_node, Binary_Heap_PQ *PQ, AStarControlData *Queue_control);

// Hapifies up the node in position d,p in the Binary Heap Priority Queue.
void heapify_up_bh(Binary_Heap_PQ *PQ, unsigned d, unsigned p, AStarControlData *Queue_control);

// Hapifies down the node in position d,p in the Binary Heap Priority Queue.
void heapify_down_bh(Binary_Heap_PQ *PQ, unsigned d, unsigned p, AStarControlData *Queue_control);

// Function to show the binary heap structure and connections
void show_bh(Binary_Heap_PQ *PQ, FILE *file, AStarControlData *Queue_control);
/*
    A* ALGORITHM
*/

// Runs the AStar algorithm to find the shortest path between the initial and final node.
// It stores the resulting solution in Sol_path. Uses a Linked list as a Priority Queue
int AStar_ll(Node *nodes, AStarPath *Sol_path, AStarControlData *Queue_control,
            unsigned long initial_node, unsigned long final_node, int heuristic_code);

// Runs the AStar algorithm to find the shortest path between the initial and final node.
// It stores the resulting solution in Sol_path. Uses a Binary Heap as a Priority Queue
int AStar_bh(Node *nodes, AStarPath *Sol_path, AStarControlData *Queue_control, unsigned long nnodes, 
        unsigned long initial_node, unsigned long final_node, int heuristic_code);

// Runs the AStar algorithm to find the shortest path between the initial and final node.
// It stores the resulting solution in Sol_path. Uses a Linked list as a Priority Queue and stores different metrics
int AStar_ll_metrics(Node *nodes, AStarPath *Sol_path, AStarControlData *Queue_control,
            unsigned long initial_node, unsigned long final_node, int heuristic_code,
            Heuristic_Metrics *heuristic_metrics, PQ_Metrics *pq_metrics);

// Runs the AStar algorithm to find the shortest path between the initial and final node.
// It stores the resulting solution in Sol_path. Uses a Binary Heap as a Priority Queue and stores different metrics
int AStar_bh_metrics(Node *nodes, AStarPath *Sol_path, AStarControlData *Queue_control, unsigned long nnodes, 
        unsigned long initial_node, unsigned long final_node, int heuristic_code,
        Heuristic_Metrics *heuristic_metrics, PQ_Metrics *pq_metrics);

/*
    SOLUTIONS MANAGEMENT
*/
// Stores the solution in a txt file and returns the number of nodes in the solution path
unsigned long store_solution(Node *nodes, AStarPath *Sol_path, unsigned long initial_node, unsigned long final_node, char *path_filename);

// Stores the resulting Control state in a txt file
void store_control(Node *nodes, AStarPath *Sol_path, AStarControlData *Queue_control, unsigned long initial_node,
                    unsigned long nnodes, unsigned long nnodes_path,char *control_filename);

#endif