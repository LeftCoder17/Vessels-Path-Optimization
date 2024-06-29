/*
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    $$$$$$$$$$$$$$    ASTAR.C VERSION 1.0    $$$$$$$$$$$$$$$$
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

    - Usage:
        >> Through the header file "astar.h"

    - Comments:
        >> 
    
    - Further development:
        >> The nodes distance function should take into account Earth geometry.

    - Status:
        >> Unfinished. Missing Vintenty formula
        >> In AStar_ll, I need to free all the linked list of the PQ when finished
        >> Possible little upgrades.

    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include <limits.h>

#include "graph_management.h"
#include "algorithms.h"


/*
    DISTANCES CALCULATIONS
*/

double nodes_squared_distance(double initial_lat, double initial_lon, double final_lat, double final_lon) {
    double total_distance_squared;
    total_distance_squared = pow(final_lat - initial_lat, 2) + pow(final_lon - initial_lon, 2);
    return total_distance_squared;
}

double heuristic(int heuristic_code, int speed, double initial_lat, double initial_lon, double final_lat, double final_lon) {
    if (heuristic_code == Dij) return heuristic_dijkstra();
    if (heuristic_code == Hav) return heuristic_haversine(speed, initial_lat, initial_lon, final_lat, final_lon);
    if (heuristic_code == SLC) return heuristic_cosines(speed, initial_lat, initial_lon, final_lat, final_lon);
    if (heuristic_code == EqApp) return heuristic_equirectangular(speed, initial_lat, initial_lon, final_lat, final_lon);
    else ExitError("wrong heuristic code", 1);
    return 0.;
}

double heuristic_dijkstra() {
    return 0.0;
}

double heuristic_haversine(int speed, double initial_lat, double initial_lon, double final_lat, double final_lon) {
    double R;
    double initial_lat_rad, final_lat_rad;
    double Delta_lat, Delta_lon;
    double a, c, dist;
    double speed_kms;
    double total_time;
    R = 6371; // km
    initial_lat_rad = initial_lat * M_PI / 180; // rad
    final_lat_rad = final_lat * M_PI / 180; // rad
    Delta_lat = (final_lat - initial_lat) * M_PI / 180; // rad
    Delta_lon = (final_lon - initial_lon) * M_PI / 180; // rad
    a = sin(Delta_lat/2) * sin(Delta_lat/2) +
        cos(initial_lat_rad) * cos(final_lat_rad) *
        sin(Delta_lon/2) * sin(Delta_lon/2);
    c = 2 * atan2(sqrt(a), sqrt(1-a));
    dist = R * c; // km
    speed_kms = speed * 0.514444; // kn to km/s
    total_time = dist / speed_kms;
    return total_time;
}

double heuristic_cosines(int speed, double initial_lat, double initial_lon, double final_lat, double final_lon) {
    double R;
    double dist;
    double speed_kms;
    double total_time;

    R = 6371; // km
    dist = acos(sin(initial_lat * M_PI / 180) * sin(final_lat * M_PI / 180) +
                cos(initial_lon * M_PI / 180) * cos(final_lon * M_PI / 180) * cos((final_lon - initial_lon) * M_PI / 180)
                ) * R;
    speed_kms = speed * 0.514444; // kn to km/s
    total_time = dist / speed_kms;
    return total_time;
}

double heuristic_equirectangular(int speed, double initial_lat, double initial_lon, double final_lat, double final_lon) {
    double x,y;
    double R;
    double dist;
    double speed_kms;
    double total_time;
    R = 6371; // km
    x = ((final_lon - initial_lon) * M_PI / 180) * cos(((final_lat - initial_lat) / 2) * M_PI / 180);
    y = (final_lat - initial_lat) * M_PI / 180;
    dist = sqrt(x*x + y*y) * R;
    speed_kms = speed * 0.514444; // kn to km/s
    total_time = dist / speed_kms;
    return total_time;
}


//double heuristic_vincenty(int speed, double initial_lat, double initial_lon, double final_lat, double final_lon) {}

/*
    LINKED LIST MANAGEMENT
*/

bool enqueue_ll(unsigned long node_id, Linked_Element_PQ **PQ, AStarControlData *Queue_control) {
    Linked_Element_PQ *new_elem = (Linked_Element_PQ *) malloc(sizeof(Linked_Element_PQ));
    if (new_elem == NULL) ExitError("when allocating memory for the Linked Element", 1);
    
    Queue_control[node_id].InPQ = 1;
    new_elem->node_id = node_id;
    double new_f = Queue_control[node_id].f;
    if ((*PQ) == NULL || new_f < Queue_control[(*PQ)->node_id].f) {
        new_elem->next = (*PQ);
        (*PQ) = new_elem;
        return true;
    }

    Linked_Element_PQ *current_elem;
    for (current_elem = (*PQ); current_elem->next && Queue_control[current_elem->next->node_id].f <= new_f; current_elem = current_elem->next);

    new_elem->next = current_elem->next;
    current_elem->next = new_elem;
    return true;
}

unsigned long dequeue_ll(Linked_Element_PQ **PQ) {
    Linked_Element_PQ *first_elem = (*PQ);
    unsigned long first_node_id = first_elem->node_id;

    (*PQ) = (*PQ)->next;
    free(first_elem);
    return first_node_id;

}


void requeue_ll(unsigned long node_id, Linked_Element_PQ **PQ, AStarControlData *Queue_control) {
    if ((*PQ)->node_id == node_id) return;

    double node_f = Queue_control[node_id].f;
    if (node_f <= Queue_control[(*PQ)->node_id].f) {
        Linked_Element_PQ *prev_elem;
        // Look for the Linked element that currently points to the node linked element
        for (prev_elem = (*PQ); prev_elem->next->node_id != node_id; prev_elem = prev_elem->next);
        Linked_Element_PQ *aux_elem = (*PQ);
        (*PQ) = prev_elem->next;
        prev_elem->next = prev_elem->next->next;
        (*PQ)->next = aux_elem;
        return;
    }

    Linked_Element_PQ *aux_elem, *prev_elem;
    // Find the node that will point to the node linked element
    for (aux_elem = (*PQ); Queue_control[aux_elem->next->node_id].f < node_f; aux_elem = aux_elem->next);
    if (aux_elem->next->node_id == node_id) return;

    // Find the node that currently points to the node linked element
    for (prev_elem = aux_elem->next; prev_elem->next->node_id != node_id; prev_elem = prev_elem->next);
    Linked_Element_PQ *node_elem = prev_elem->next;
    prev_elem->next = node_elem->next;
    node_elem->next = aux_elem->next;
    aux_elem->next = node_elem;
    return;
}

void show_ll(Linked_Element_PQ *PQ, int n, AStarControlData *Queue_control)
{
    int i = 0;
    Linked_Element_PQ *curr_elem;
    curr_elem = PQ;
    printf("PQ is:  ");
    while (curr_elem != NULL && i < n)
    {
        printf("Node %lu (f = %g) --> ", curr_elem->node_id, Queue_control[curr_elem->node_id].f);
        curr_elem = curr_elem->next;
        i++;
    }
    printf("\n");
}


/*
    BINARY HEAP TREE MANAGEMENT
*/

bool enqueue_bh(unsigned long node_id, Binary_Heap_PQ *PQ, AStarControlData *Queue_control) {
    if (PQ->l == 0) {
        PQ->l++;
        PQ->r++;
        PQ->bh_tree[0] = node_id;
        Queue_control[node_id].InPQ = 1;
    } else {
        // If the last level is full, create a new one
        if (PQ->r == (unsigned long) (1 << (PQ->l - 1))) {
            PQ->l++;
            PQ->r = 1;
        } else {
            PQ->r++;
        }
        unsigned long d, p;
        d = PQ->l-1;
        p = PQ->r-1;
        PQ->bh_tree[dp(d,p)] = node_id;
        float cost_node = Queue_control[node_id].f;
        Queue_control[node_id].InPQ = 1;
        if (cost_node < Queue_control[PQ->bh_tree[parent(d,p)]].f) {
            heapify_up_bh(PQ, d, p, Queue_control);
        }
    }
    return true;
}

unsigned long dequeue_bh(Binary_Heap_PQ *PQ, AStarControlData *Queue_control) {
    unsigned long root_node_id = PQ->bh_tree[0];
    if (PQ->l == 1) {
        PQ->l--;
        PQ->r--;
        return root_node_id;
    } else {
        unsigned long d = PQ->l-1;
        unsigned long p = PQ->r-1;
        PQ->bh_tree[0] = PQ->bh_tree[dp(d, p)];
        PQ->r--;
        if (PQ->r == 0) {
            PQ->l--;
            PQ->r = (1 << (PQ->l - 1));
        }
    }
    heapify_down_bh(PQ, 0, 0, Queue_control);
    return root_node_id;
}

void requeue_bh(unsigned long node_id, Binary_Heap_PQ *PQ, AStarControlData *Queue_control) {
    // We search the position d,p of the node
    unsigned d, p;
    for (d = 0; d < PQ->l - 1; d++) {
        for (p = 0; p <= (unsigned) (1 << (d)) - 1; p++) {
            if (node_id == PQ->bh_tree[dp(d, p)]) {
                heapify_up_bh(PQ, d, p, Queue_control);
                return;
            }
        }
    }

    d = PQ->l - 1;
    for (p = 0; p <= PQ->r - 1; p++) {
        if (node_id == PQ->bh_tree[dp(d, p)]) {
            heapify_up_bh(PQ, d, p, Queue_control);
            return;
        }
    }
}

void heapify_up_bh(Binary_Heap_PQ *PQ, unsigned d, unsigned p, AStarControlData *Queue_control) {
    unsigned aux_node;
    while (d > 0 && Queue_control[PQ->bh_tree[dp(d, p)]].f < Queue_control[PQ->bh_tree[parent(d, p)]].f) {
        aux_node = PQ->bh_tree[dp(d, p)];
        PQ->bh_tree[dp(d, p)] = PQ->bh_tree[parent(d, p)];
        PQ->bh_tree[parent(d, p)] = aux_node;
        d = d - 1;
        p = (int) p / 2;
    }
}

void heapify_down_bh(Binary_Heap_PQ *PQ, unsigned d, unsigned p, AStarControlData *Queue_control) {
    while (d < PQ->l - 1 && ((d == PQ->l - 2) ? (p <= (PQ->r - 1) / 2) : 1)) {  // Exists a left child
        unsigned long smallest_child_id = PQ->bh_tree[lchild(d, p)];
        double smallest_cost = Queue_control[PQ->bh_tree[lchild(d, p)]].f;
        unsigned smallest_child = 1; // 1: left, 2: right

        if ((d == PQ->l - 2) ? (2*p + 1 < PQ->r) : 1) {  // Exists a right child
            if (Queue_control[PQ->bh_tree[lchild(d, p)]].f > Queue_control[PQ->bh_tree[rchild(d, p)]].f) {
                smallest_child_id = PQ->bh_tree[rchild(d, p)];
                smallest_cost = Queue_control[PQ->bh_tree[rchild(d, p)]].f;
                smallest_child = 2;
            }

        }
        if (Queue_control[PQ->bh_tree[dp(d, p)]].f <= smallest_cost) {
            return;
        }
        
        unsigned long i_aux = PQ->bh_tree[dp(d, p)];
        PQ->bh_tree[dp(d, p)] = smallest_child_id;
        if (smallest_child == 1) {
            PQ->bh_tree[lchild(d, p)] = i_aux;
            
            d = d + 1;
            p = 2*p;
        } else {
            PQ->bh_tree[rchild(d, p)] = i_aux;
            d = d + 1;
            p = 2*p + 1;
        }
    }
}

void show_bh(Binary_Heap_PQ *PQ, FILE *file, AStarControlData *Queue_control) {
    // Shows only the first 5 levels
    if (PQ->l > 0) {
        fprintf(file, "Level 1:                                            %lu (%g)\n",
                            PQ->bh_tree[dp(0, 0)], Queue_control[PQ->bh_tree[dp(0, 0)]].f);
    }
    if (PQ->l > 1) {
        if (PQ->l == 2 && PQ->r == 1) {
                    fprintf(file, "Level 2:                       %lu (%g)\n",
                                PQ->bh_tree[dp(1, 0)], Queue_control[PQ->bh_tree[dp(1, 0)]].f);
        } else {
                    fprintf(file, "Level 2:                       %lu (%g)                                            %lu (%g)\n",
                        PQ->bh_tree[dp(1, 0)], Queue_control[PQ->bh_tree[dp(1, 0)]].f,
                        PQ->bh_tree[dp(1, 1)], Queue_control[PQ->bh_tree[dp(1, 1)]].f);
        }
    }
    if (PQ->l > 2) {
        if (PQ->l == 3 && PQ->r == 1) {
            fprintf(file, "Level 3:          %lu (%g)\n",
                            PQ->bh_tree[dp(2, 0)], Queue_control[PQ->bh_tree[dp(2, 0)]].f);
        } else if (PQ->l == 3 && PQ->r == 2) {
            fprintf(file, "Level 3:          %lu (%g)                  %lu (%g)\n",
                            PQ->bh_tree[dp(2, 0)], Queue_control[PQ->bh_tree[dp(2, 0)]].f,
                            PQ->bh_tree[dp(2, 1)], Queue_control[PQ->bh_tree[dp(2, 1)]].f);

        } else if (PQ->l == 3 && PQ->r == 3) {
            fprintf(file, "Level 3:          %lu (%g)                  %lu (%g)      ----       %lu (%g)\n",
                            PQ->bh_tree[dp(2, 0)], Queue_control[PQ->bh_tree[dp(2, 0)]].f,
                            PQ->bh_tree[dp(2, 1)], Queue_control[PQ->bh_tree[dp(2, 1)]].f,
                            PQ->bh_tree[dp(2, 2)], Queue_control[PQ->bh_tree[dp(2, 2)]].f);

        } else {
            fprintf(file, "Level 3:          %lu (%g)                  %lu (%g)      ----       %lu (%g)                   %lu (%g)\n",
                            PQ->bh_tree[dp(2, 0)], Queue_control[PQ->bh_tree[dp(2, 0)]].f,
                            PQ->bh_tree[dp(2, 1)], Queue_control[PQ->bh_tree[dp(2, 1)]].f,
                            PQ->bh_tree[dp(2, 2)], Queue_control[PQ->bh_tree[dp(2, 2)]].f,
                            PQ->bh_tree[dp(2, 3)], Queue_control[PQ->bh_tree[dp(2, 3)]].f);
        }
    
    }
    if (PQ->l > 3) {
        if (PQ->l == 4 && PQ->r == 1) {
            fprintf(file, "Level 4: %lu (%g)\n",
                            PQ->bh_tree[dp(3, 0)], Queue_control[PQ->bh_tree[dp(3, 0)]].f);
        } else if (PQ->l == 4 && PQ->r == 2) {
            fprintf(file, "Level 4: %lu (%g)      %lu (%g)\n",
                            PQ->bh_tree[dp(3, 0)], Queue_control[PQ->bh_tree[dp(3, 0)]].f,
                            PQ->bh_tree[dp(3, 1)], Queue_control[PQ->bh_tree[dp(3, 1)]].f);
        } else if (PQ->l == 4 && PQ->r == 3) {
            fprintf(file, "Level 4: %lu (%g)      %lu (%g)  --  %lu (%g)\n",
                            PQ->bh_tree[dp(3, 0)], Queue_control[PQ->bh_tree[dp(3, 0)]].f,
                            PQ->bh_tree[dp(3, 1)], Queue_control[PQ->bh_tree[dp(3, 1)]].f,
                            PQ->bh_tree[dp(3, 2)], Queue_control[PQ->bh_tree[dp(3, 2)]].f);
        } else if (PQ->l == 4 && PQ->r == 4) {
            fprintf(file, "Level 4: %lu (%g)      %lu (%g)  --  %lu (%g)      %lu (%g)\n",
                            PQ->bh_tree[dp(3, 0)], Queue_control[PQ->bh_tree[dp(3, 0)]].f,
                            PQ->bh_tree[dp(3, 1)], Queue_control[PQ->bh_tree[dp(3, 1)]].f,
                            PQ->bh_tree[dp(3, 2)], Queue_control[PQ->bh_tree[dp(3, 2)]].f,
                            PQ->bh_tree[dp(3, 3)], Queue_control[PQ->bh_tree[dp(3, 3)]].f);
        } else if (PQ->l == 4 && PQ->r == 5) {
            fprintf(file, "Level 4: %lu (%g)      %lu (%g)  --  %lu (%g)             %lu (%g)  --  %lu (%g)\n",
                            PQ->bh_tree[dp(3, 0)], Queue_control[PQ->bh_tree[dp(3, 0)]].f,
                            PQ->bh_tree[dp(3, 1)], Queue_control[PQ->bh_tree[dp(3, 1)]].f,
                            PQ->bh_tree[dp(3, 2)], Queue_control[PQ->bh_tree[dp(3, 2)]].f,
                            PQ->bh_tree[dp(3, 3)], Queue_control[PQ->bh_tree[dp(3, 3)]].f,
                            PQ->bh_tree[dp(3, 4)], Queue_control[PQ->bh_tree[dp(3, 4)]].f);
        } else if (PQ->l == 4 && PQ->r == 6) {
            fprintf(file, "Level 4: %lu (%g)      %lu (%g)  --  %lu (%g)             %lu (%g)  --  %lu (%g)      %lu (%g)\n",
                            PQ->bh_tree[dp(3, 0)], Queue_control[PQ->bh_tree[dp(3, 0)]].f,
                            PQ->bh_tree[dp(3, 1)], Queue_control[PQ->bh_tree[dp(3, 1)]].f,
                            PQ->bh_tree[dp(3, 2)], Queue_control[PQ->bh_tree[dp(3, 2)]].f,
                            PQ->bh_tree[dp(3, 3)], Queue_control[PQ->bh_tree[dp(3, 3)]].f,
                            PQ->bh_tree[dp(3, 4)], Queue_control[PQ->bh_tree[dp(3, 4)]].f,
                            PQ->bh_tree[dp(3, 5)], Queue_control[PQ->bh_tree[dp(3, 5)]].f);
        } else if (PQ->l == 4 && PQ->r == 7) {
            fprintf(file, "Level 4: %lu (%g)      %lu (%g)  --  %lu (%g)             %lu (%g)  --  %lu (%g)      %lu (%g)  --  %lu (%g)\n",
                            PQ->bh_tree[dp(3, 0)], Queue_control[PQ->bh_tree[dp(3, 0)]].f,
                            PQ->bh_tree[dp(3, 1)], Queue_control[PQ->bh_tree[dp(3, 1)]].f,
                            PQ->bh_tree[dp(3, 2)], Queue_control[PQ->bh_tree[dp(3, 2)]].f,
                            PQ->bh_tree[dp(3, 3)], Queue_control[PQ->bh_tree[dp(3, 3)]].f,
                            PQ->bh_tree[dp(3, 4)], Queue_control[PQ->bh_tree[dp(3, 4)]].f,
                            PQ->bh_tree[dp(3, 5)], Queue_control[PQ->bh_tree[dp(3, 5)]].f,
                            PQ->bh_tree[dp(3, 6)], Queue_control[PQ->bh_tree[dp(3, 6)]].f);

        } else {
            fprintf(file, "Level 4: %lu (%g)      %lu (%g)  --  %lu (%g)      %lu (%g)  --  %lu (%g)      %lu (%g)  --  %lu (%g)      %lu (%g)\n",
                            PQ->bh_tree[dp(3, 0)], Queue_control[PQ->bh_tree[dp(3, 0)]].f,
                            PQ->bh_tree[dp(3, 1)], Queue_control[PQ->bh_tree[dp(3, 1)]].f,
                            PQ->bh_tree[dp(3, 2)], Queue_control[PQ->bh_tree[dp(3, 2)]].f,
                            PQ->bh_tree[dp(3, 3)], Queue_control[PQ->bh_tree[dp(3, 3)]].f,
                            PQ->bh_tree[dp(3, 4)], Queue_control[PQ->bh_tree[dp(3, 4)]].f,
                            PQ->bh_tree[dp(3, 5)], Queue_control[PQ->bh_tree[dp(3, 5)]].f,
                            PQ->bh_tree[dp(3, 6)], Queue_control[PQ->bh_tree[dp(3, 6)]].f,
                            PQ->bh_tree[dp(3, 7)], Queue_control[PQ->bh_tree[dp(3, 7)]].f);
        }

        
    }
    return;
}


/*
    A* ALGORITHM
*/

int AStar_ll(Node *nodes, AStarPath *Sol_path, AStarControlData *Queue_control,
            unsigned long initial_node, unsigned long final_node, int heuristic_code) {

    // 1. Initialize Linked List
    Linked_Element_PQ *PQ;
    PQ = NULL;

    // 2. Start the algorithm
    // 2.1. Set the initial values
    double initial_node_h;

    Sol_path[initial_node].g = 0.0;
    Sol_path[initial_node].parent = ULONG_MAX;

    initial_node_h = heuristic(heuristic_code, nodes[initial_node].speed,
                                nodes[initial_node].lat, nodes[initial_node].lon,
                                nodes[final_node].lat, nodes[final_node].lon);
    Queue_control[initial_node].f = initial_node_h;

    if (!enqueue_ll(initial_node, &PQ, Queue_control)) return -1;

    // 2.2. Iterate until the solution is found or the are not more nodes left in the Queue
    unsigned long curr_node, succ_node, index;
    double succ_g, succ_h, f_aux;
    while (PQ != NULL) {
        // 2.2.1. Check whether the solution has been found or not
        if ((curr_node = dequeue_ll(&PQ)) == final_node) {
            free(PQ);
            return 1;
        }

        // 2.2.2. Iterate through all the connected nodes of the current node
        if (nodes[curr_node].nedges) {
            for (index = 0; index < nodes[curr_node].nedges; index++) {
                succ_node = nodes[curr_node].to_nodes[index];
                if (Queue_control[succ_node].extended) continue;

                succ_g = nodes[curr_node].to_times[index];
                succ_h = heuristic(heuristic_code, nodes[succ_node].speed,
                                    nodes[succ_node].lat, nodes[succ_node].lon,
                                    nodes[final_node].lat, nodes[final_node].lon);                                                    
                
                f_aux = Sol_path[curr_node].g + succ_g + succ_h;

                if (!Queue_control[succ_node].InPQ)
                {
                    Sol_path[succ_node].parent = curr_node;
                    Sol_path[succ_node].g = Sol_path[curr_node].g + succ_g;
                    Queue_control[succ_node].f = f_aux;
                    if (!enqueue_ll(succ_node, &PQ, Queue_control)) return -1;
                } else if (f_aux < Queue_control[succ_node].f)
                {
                    Sol_path[succ_node].parent = curr_node;
                    Sol_path[succ_node].g = Sol_path[curr_node].g + succ_g;
                    Queue_control[succ_node].f = f_aux;
                    requeue_ll(succ_node, &PQ, Queue_control);
                }
            }
        }

        // 2.2.3. Modify Queue control of the used node
        Queue_control[curr_node].InPQ = 0;
        Queue_control[curr_node].extended = 1;
    }
    return 0;
}

int AStar_bh(Node *nodes, AStarPath *Sol_path, AStarControlData *Queue_control, unsigned long nnodes,
            unsigned long initial_node, unsigned long final_node, int heuristic_code) {
    // 1. Initialize Binary Tree
    Binary_Heap_PQ PQ;
    PQ.l = 0;
    PQ.r = 0;
    PQ.bh_tree = (unsigned long *) malloc(nnodes * sizeof(unsigned long));
    if (PQ.bh_tree == NULL) ExitError("when allocating memory for the BH tree Data vector", 1);

    // 2. Start the algorithm
    // 2.1. Set the initial values
    double initial_node_h;

    Sol_path[initial_node].g = 0.0;
    Sol_path[initial_node].parent = ULONG_MAX;
    initial_node_h = heuristic(heuristic_code, nodes[initial_node].speed,
                                nodes[initial_node].lat, nodes[initial_node].lon,
                                nodes[final_node].lat, nodes[final_node].lon);
    Queue_control[initial_node].f = initial_node_h;
    if (!enqueue_bh(initial_node, &PQ, Queue_control)) return -1;

    // 2.2. Iterate until the solution is found or the are not more nodes left in the Queue
    unsigned long curr_node, succ_node, index;
    double succ_g, succ_h, f_aux;

    while (PQ.l != 0) {
        // 2.2.1. Check whether the solution has been found or not
        if ((curr_node = dequeue_bh(&PQ, Queue_control)) == final_node) {
            free(PQ.bh_tree);
            return 1;
        }

        // 2.2.2. Iterate through all the connected nodes of the current node
        if (nodes[curr_node].nedges) {
            for (index = 0; index < nodes[curr_node].nedges; index++) {
                succ_node = nodes[curr_node].to_nodes[index];
                if (Queue_control[succ_node].extended) continue;

                succ_g = nodes[curr_node].to_times[index];
                succ_h = heuristic(heuristic_code, nodes[succ_node].speed,
                                    nodes[succ_node].lat, nodes[succ_node].lon,
                                    nodes[final_node].lat, nodes[final_node].lon);
                
                f_aux = Sol_path[curr_node].g + succ_g + succ_h;

                if (f_aux < Queue_control[succ_node].f || !Queue_control[succ_node].InPQ) {
                    Sol_path[succ_node].parent = curr_node;
                    Sol_path[succ_node].g = Sol_path[curr_node].g + succ_g;
                    Queue_control[succ_node].f = f_aux;
                }

                if (!Queue_control[succ_node].InPQ) {
                    if (!enqueue_bh(succ_node, &PQ, Queue_control)) return -1;
                } else if (f_aux < Queue_control[succ_node].f) {
                    requeue_bh(succ_node, &PQ, Queue_control);
                }
            }
        }


        // 2.2.3. Modify Queue control of the used node
        Queue_control[curr_node].InPQ = false;
        Queue_control[curr_node].extended = true;
    }
    return 0;
}

int AStar_ll_metrics(Node *nodes, AStarPath *Sol_path, AStarControlData *Queue_control,
            unsigned long initial_node, unsigned long final_node, int heuristic_code,
            Heuristic_Metrics *heuristic_metrics, PQ_Metrics *pq_metrics) {

    // 1. Initialize Linked List and set up timers
    Linked_Element_PQ *PQ;
    PQ = NULL;

    clock_t start_total_time, start_time;
    start_total_time = clock();

    // 2. Start the algorithm
    // 2.1. Set the initial values
    double initial_node_h;

    Sol_path[initial_node].g = 0.0;
    Sol_path[initial_node].parent = ULONG_MAX;

    start_time = clock();
    initial_node_h = heuristic(heuristic_code, nodes[initial_node].speed,
                                nodes[initial_node].lat, nodes[initial_node].lon,
                                nodes[final_node].lat, nodes[final_node].lon);
    heuristic_metrics->calculus_time += (double) (clock() - start_time) / CLOCKS_PER_SEC;
    Queue_control[initial_node].f = initial_node_h;

    start_time = clock();
    if (!enqueue_ll(initial_node, &PQ, Queue_control)) return -1;
    pq_metrics->enqueue_time += (double) (clock() - start_time) / CLOCKS_PER_SEC;

    // 2.2. Iterate until the solution is found or the are not more nodes left in the Queue
    unsigned long curr_node, succ_node, index;
    double succ_g, succ_h, f_aux;
    while (PQ != NULL) {
        // 2.2.1. Check whether the solution has been found or not
        start_time = clock();
        if ((curr_node = dequeue_ll(&PQ)) == final_node) {
            pq_metrics->dequeue_time += (double) (clock() - start_time) / CLOCKS_PER_SEC;
            pq_metrics->total_time += (double) (clock() - start_total_time) / CLOCKS_PER_SEC;
            free(PQ);
            return 1;
        }
        pq_metrics->dequeue_time += (double) (clock() - start_time) / CLOCKS_PER_SEC;

        // 2.2.2. Iterate through all the connected nodes of the current node
        if (nodes[curr_node].nedges) {
            for (index = 0; index < nodes[curr_node].nedges; index++) {
                succ_node = nodes[curr_node].to_nodes[index];
                if (Queue_control[succ_node].extended) continue;

                succ_g = nodes[curr_node].to_times[index];
                start_time = clock();
                succ_h = heuristic(heuristic_code, nodes[succ_node].speed,
                                    nodes[succ_node].lat, nodes[succ_node].lon,
                                    nodes[final_node].lat, nodes[final_node].lon);
                heuristic_metrics->calculus_time += (double) (clock() - start_time) / CLOCKS_PER_SEC;
                                                    
                
                f_aux = Sol_path[curr_node].g + succ_g + succ_h;

                if (!Queue_control[succ_node].InPQ)
                {
                    Sol_path[succ_node].parent = curr_node;
                    Sol_path[succ_node].g = Sol_path[curr_node].g + succ_g;
                    Queue_control[succ_node].f = f_aux;
                    start_time = clock();
                    if (!enqueue_ll(succ_node, &PQ, Queue_control)) return -1;
                    pq_metrics->enqueue_time += (double) (clock() - start_time) / CLOCKS_PER_SEC;
                } else if (f_aux < Queue_control[succ_node].f)
                {
                    Sol_path[succ_node].parent = curr_node;
                    Sol_path[succ_node].g = Sol_path[curr_node].g + succ_g;
                    Queue_control[succ_node].f = f_aux;
                    start_time = clock();
                    requeue_ll(succ_node, &PQ, Queue_control);
                    pq_metrics->requeue_time += (double) (clock() - start_time) / CLOCKS_PER_SEC;
                }
            }
        }

        // 2.2.3. Modify Queue control of the used node
        Queue_control[curr_node].InPQ = 0;
        Queue_control[curr_node].extended = 1;
        heuristic_metrics->nexpanded += 1;
    }
    pq_metrics->total_time += (double) (clock() - start_total_time) / CLOCKS_PER_SEC;
    return 0;
}

int AStar_bh_metrics(Node *nodes, AStarPath *Sol_path, AStarControlData *Queue_control, unsigned long nnodes,
            unsigned long initial_node, unsigned long final_node, int heuristic_code,
            Heuristic_Metrics *heuristic_metrics, PQ_Metrics *pq_metrics) {
    // 1. Initialize Binary Tree and set up the timers
    Binary_Heap_PQ PQ;
    PQ.l = 0;
    PQ.r = 0;
    PQ.bh_tree = (unsigned long *) malloc(nnodes * sizeof(unsigned long));
    if (PQ.bh_tree == NULL) ExitError("when allocating memory for the BH tree Data vector", 1);

    clock_t start_total_time, start_time;
    start_total_time = clock();

    // 2. Start the algorithm
    // 2.1. Set the initial values
    double initial_node_h;

    Sol_path[initial_node].g = 0.0;
    Sol_path[initial_node].parent = ULONG_MAX;
    start_time = clock();
    initial_node_h = heuristic(heuristic_code, nodes[initial_node].speed,
                                nodes[initial_node].lat, nodes[initial_node].lon,
                                nodes[final_node].lat, nodes[final_node].lon);
    heuristic_metrics->calculus_time += (double) (clock() - start_time) / CLOCKS_PER_SEC;
    
    Queue_control[initial_node].f = initial_node_h;
    start_time = clock();
    if (!enqueue_bh(initial_node, &PQ, Queue_control)) return -1;
    pq_metrics->enqueue_time += (double) (clock() - start_time) / CLOCKS_PER_SEC;

    // 2.2. Iterate until the solution is found or the are not more nodes left in the Queue
    unsigned long curr_node, succ_node, index;
    double succ_g, succ_h, f_aux;

    while (PQ.l != 0) {
        // 2.2.1. Check whether the solution has been found or not
        start_time = clock();
        if ((curr_node = dequeue_bh(&PQ, Queue_control)) == final_node) {
            pq_metrics->dequeue_time += (double) (clock() - start_time) / CLOCKS_PER_SEC;
            pq_metrics->total_time += (double) (clock() - start_total_time) / CLOCKS_PER_SEC;
            free(PQ.bh_tree);
            return 1;
        }
        pq_metrics->dequeue_time += (double) (clock() - start_time) / CLOCKS_PER_SEC;

        // 2.2.2. Iterate through all the connected nodes of the current node
        if (nodes[curr_node].nedges) {
            for (index = 0; index < nodes[curr_node].nedges; index++) {
                succ_node = nodes[curr_node].to_nodes[index];
                if (Queue_control[succ_node].extended) continue;

                succ_g = nodes[curr_node].to_times[index];
                start_time = clock();
                succ_h = heuristic(heuristic_code, nodes[succ_node].speed,
                                    nodes[succ_node].lat, nodes[succ_node].lon,
                                    nodes[final_node].lat, nodes[final_node].lon);
                heuristic_metrics->calculus_time += (double) (clock() - start_time) / CLOCKS_PER_SEC;
                
                f_aux = Sol_path[curr_node].g + succ_g + succ_h;

                if (f_aux < Queue_control[succ_node].f || !Queue_control[succ_node].InPQ) {
                    Sol_path[succ_node].parent = curr_node;
                    Sol_path[succ_node].g = Sol_path[curr_node].g + succ_g;
                    Queue_control[succ_node].f = f_aux;
                }

                if (!Queue_control[succ_node].InPQ) {
                    start_time = clock();
                    if (!enqueue_bh(succ_node, &PQ, Queue_control)) return -1;
                    pq_metrics->enqueue_time += (double) (clock() - start_time) / CLOCKS_PER_SEC;
                } else if (f_aux < Queue_control[succ_node].f) {
                    start_time = clock();
                    requeue_bh(succ_node, &PQ, Queue_control);
                    pq_metrics->requeue_time += (double) (clock() - start_time) / CLOCKS_PER_SEC;
                }
            }
        }


        // 2.2.3. Modify Queue control of the used node
        Queue_control[curr_node].InPQ = false;
        Queue_control[curr_node].extended = true;
        heuristic_metrics->nexpanded += 1;
    }
    pq_metrics->total_time += (double) (clock() - start_total_time) / CLOCKS_PER_SEC;
    return 0;
}

/*
    SOLUTIONS MANAGEMENT
*/
unsigned long store_solution(Node *nodes, AStarPath *Sol_path, unsigned long initial_node, unsigned long final_node, char *path_filename) {
    // 1. Create the file where the path is stored
    FILE *path_file = fopen(path_filename, "w");
    if (path_file == NULL) ExitError("when creating the path file", 1);

    // 2. Reverse the path
    unsigned long son, parent, grandparent;
    unsigned long nnodes_path;

    son = final_node;
    parent = Sol_path[son].parent;
    Sol_path[final_node].parent = ULONG_MAX;
    nnodes_path = 1;
    while (son != initial_node) {
        grandparent = Sol_path[parent].parent;
        Sol_path[parent].parent = son;
        son = parent;
        parent = grandparent;
        nnodes_path++;
    }

    // 3. Write path
    unsigned long index;
    index = 0;
    fprintf(path_file, "id = %lu | %g | %g | travelling_time = Source\n", nodes[initial_node].id, nodes[initial_node].lat, nodes[initial_node].lon);
    for (parent = Sol_path[initial_node].parent; parent != ULONG_MAX; parent = Sol_path[parent].parent) {
        fprintf(path_file, "id = %lu | %g | %g | travelling_time = %7.3f\n", nodes[parent].id, nodes[parent].lat, nodes[parent].lon, Sol_path[parent].g);
        index++;
    }
    fclose(path_file);

    return nnodes_path;
}

void store_control(Node *nodes, AStarPath *Sol_path, AStarControlData *Queue_control, unsigned long initial_node,
                    unsigned long nnodes, unsigned long nnodes_path, char *control_filename) {
    // 1. Counts the number of nodes in the extended, in the Queue and not visited
    unsigned long next, npq;
    next = 0;
    npq = 0;

    enum Final_state *final_states;
    final_states = (enum Final_state *) malloc(nnodes * sizeof(enum Final_state));
    if (final_states == NULL) ExitError("when allocating memory for final state", 1);

    unsigned long index;
    for (index = 0; index < nnodes; index++) {
        if (Queue_control[index].InPQ) {
            final_states[index] = InPQ;
            npq++;
        } else if (Queue_control[index].extended) {
            final_states[index] = Ext;
            next++;
        } else {
            final_states[index] = NotVis;
        }
    }

    unsigned long node_id;
    for (node_id = initial_node; node_id != ULONG_MAX; node_id = Sol_path[node_id].parent) {
        if (final_states[node_id] == InPQ) {
            npq--;
        } else if (final_states[node_id] == Ext) {
            next--;
        }
        final_states[node_id] = InSol;
    }

    // 2. Create the file where the control is stored
    FILE *control_file = fopen(control_filename, "w");
    if (control_file == NULL) ExitError("when creating the control file", 2);

    // 3. Write the header
    if (fprintf(control_file, "nnodes_path,next,npq\n") < 0) {
        ExitError("when writing the header text in the control file", 3);
    }
    if (fprintf(control_file, "%lu,%lu,%lu\n", nnodes_path, next, npq) < 0) {
        ExitError("when writing the header in the control file", 4);
    }

    // 4. Write the nodes and their coordinates of each list
    if (fprintf(control_file, "Nodes in path\n") < 0) {
        ExitError("when writing the path text in the control file", 5);
    }
    for (unsigned long index = 0; index < nnodes; index++) {
        if (final_states[index] == InSol) {
            if (fprintf(control_file, "%g,%g\n", nodes[index].lon, nodes[index].lat) < 0) {
                ExitError("when writing the path coordinates in the control file", 6);
            }
        }
    }

    if (fprintf(control_file, "Nodes extended\n") < 0) {
        ExitError("when writing the extended coordinates text in the control file", 7);
    }
    for (unsigned long index = 0; index < nnodes; index++) {
        if (final_states[index] == Ext) {
            if (fprintf(control_file, "%g,%g\n", nodes[index].lon, nodes[index].lat) < 0) {
                ExitError("when writing the extended coordinates in the control file", 8);
            }
        }
    }

    if (fprintf(control_file, "Nodes in PQ\n") < 0) {
        ExitError("when writing the PQ list text in the control file", 9);
    }
    for (unsigned long index = 0; index < nnodes; index++) {
        if (final_states[index] == InPQ) {
            if (fprintf(control_file, "%g,%g\n", nodes[index].lon, nodes[index].lat) < 0) {
                ExitError("when writing the PQ coordinates in the control file", 10);
            }
        }
    }

    free(final_states);
    fclose(control_file);
}