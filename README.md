# Overview
This repository contains various C libraries and programs developed for my master thesis. The thesis focuses on creating a graph from vessel data through a series of steps. Each step is encapsulated in its own program and library, designed to handle specific tasks efficiently. It has been decided to create the graph in different steps because of the long time required to compute each one of the step for large amounts of data. In contrast, the intermedium graphs generated must be stored and open. However, the fraction of time used to do these actions compareed to the rest of the time needed is minimal, so it is not considered an issue.

## Table of Contents
1. [Main Programs](#1-main-programs)

    1.1. [Store the Graph](#11-store-the-graph)

    1.2. [Filter the Graph](#12-filter-the-graph)

    1.3. [Compute the Intersections](#13-compute-the-intersections)

    1.4. [Find Paths](#14-find-paths)

2. [Libraries](#2-libraries)

    2.1. [Graph Management](#21-graph-management)

    2.2. [Intersections](#22-intersections)

    2.3. [Algorithms](#23-algorithms)

    2.4. [Metrics](#24-metrics)


# 1. Main programs
## 1.1 Store the graph
### Description
This program reads a csv file containing the data and stores the nodes and edges of the different paths in a binary file or several binary files. 
The node and path identifications are assigned by order of appearence. The edges are considered to be unidirectional and are created only if and only if two adjacent nodes belong to the same ship. However, the nodes which correspond to a ship named "SAT-AIS" are ignored, as they represent satelites, not vessels.
The program has the modes: 0, 1 and 2. The mode "0" reads the csv file and stores the graph of all the different paths, whichever the value of the shiptype. The mode "1" is used to select which shiptypes have to be computed and are stored separately, while the mode "3" stores all the different shiptypes in different files.

### Compilation
```
gcc -o store_exe store_graph.c libs/graph_management.c -lm
```

### Usage
```
./store_exe data_input.csv shiptypes_counter.txt program_mode (+ additonal)
```
The additional inputs depend on the mode:

    >> 0: data_output.bin

    >> 1: nshiptypes data_output_1.bin  shiptype1 data_output_2.bin shiptype2 ...

    >> 2: data_output

### Outputs
On one hand, the main output are the binary files that contain the graphs. On the other hand, shiptypes_counter.txt contains the number of paths for every shiptype.

## 1.2. Filter the data
### Description
This programs removes all the paths that has less than the tolerance distance between the initial and final node stores the graph in a new binary file, adapting the identifications of nodes and paths if necessary.

### Compilation
```
gcc -o filter_exe filter_vessels.c libs/graph_management.c -lm
```

### Usage
```
./filter_exe data_input.bin data_output.bin tolerance_in_km
```

### Outputs
The filtered graph.

## 1.3. Compute the Intersections
### Description
This program opens a stored graph in a binary file and appends the intersections between edges as new nodes. The id of each new node is assigned by order of appearance. Thus, they are stored in an array in the position corresponding to their id. The edges are unidirectional. The intersections type 1 are implemented once they are detected, while the rest are not implemented yet, so they are ignored.

### Compilation
```
gcc -o add_int_exe add_intersections.c libs/graph_management.c libs/intersections.c -lm
```

### Usage
```
./add_int_exe data_input.bin data_output.bin counter_filename.txt
```

### Outputs
On one hand, it creates the graph with the intersections computed and stores it in a binary file. On the other hand, it also stores in a text file the number of intersections every path has.

## 1.4. Find paths
### Description
This program opens a stored graph in a binary file and finds the best path between the initial and final coordinates, depending on the program mode.

### Compilation
```
gcc -o path_exe path_finder.c libs/graph_management.c libs/algorithms.c -lm
```

### Usage
```
./path stored_graph.bin heuristic_code program_mode (+ additional args)
```
The additional inputs depend on the program_mode:

    >> 0: pq_code initial_lat initial_lon final_lat final_lon solution_filename_PATH.txt solution_filename_CONTROL.txt

    >> 1: pq_code path_id solution_filename_PATH.txt solution_filename_CONTROL.txt

    >> 2: heuristic_metrics.txt pq_metrics.txt

The heuristic code is used to select which equation will be used to compute the distance for the heuristic value:

    >> 0: h=0 like Dijkstra
    
    >> 1: Haversine

    >> 2: Spherical Law of Cosines
            
    >> 3: Equirectangular approximation

The pq_code select the Priority Queue that will be used:

    >> 0: Sorted Linked List

    >> 1: Binary Heap

### Output
For the case of the modes "0" and "1", the only difference is how to select the first and last nodes of the path to find. In case of mode "0", the coordenates must be selected, allowing to choose a great variety of options. In contrast, the mode "1" will find the path for the selected path of identification path_id. In both cases, however, the output consists of a file PATH that contains the coordenates of the nodes of the path, sorted, indicating the cost from the origin to every node. The second file CONTROL contains a list of the coordinates from the nodes that are from the paths, the ones that have been extended but doesn't conform the solution and the ones that were left in the Priority Queue.

For the case "2", the solution is computed for every path in the graph, both using a Linked List as a PQ and a Binary Heap, and stores some metrics about the heuristics and priority queues to analyse them.

# 2. Libraries
## 2.1. Graph Management
### Description
Library conformed by graph_management.h and graph_management.c. It contains functions that are related to the management of both the graphs and the paths.

## 2.2. Intersections
### Description
Library conformed by intersections.h and intersections.c. It contains functions that are related to identify and compute the intersections of a graph.

## 2.3. Algorithms
### Description
Library conformed by algorithms.h and algorithms.c. It contains functions that are related to A star algorithm. There are also defined the functions of the different heuristics and priority queues.

## 2.4. Metrics
### Description
Library conformed by metrics.h and metrics.c. It contains functions that stores different metrics about the heuristics and priority queue performance.
