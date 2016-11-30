/****************************************************************
 This file implements the connectivity of a instance position map
******************************************************************/

#include <unistd.h>
#include <dirent.h>  /* header file propossing directory */
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

#include "config.h"
#include "contact.h"


/*****************************************************************/
/* This function read the trace of all nodes into a list.        *
 * Input: the directory path where the nodes' traces are stored. *
 * Note that, this directory should no other file except trace.  */
struct Node_Entry * read_nodes_trace (char *dir_name, long int cutdown_time);

/*****************************************************************/
/* this function gets the GPS location of all nodes at geiven time instance */
struct Instance_Map * get_instance_position_map (struct Node_Entry *nodes_list, long int time);
/*****************************************************************/
/* this function gets the connected node set for a specified node
 * Input: the nodes position Map; and the reference node  
 * Output: the list of connected nodes for the reference */
int get_connected_nodes_for_specified_node(struct Instance_Map *map, struct Instance_Map *spec_node);

/*****************************************************************/
/* this function gets the connected node set for all nodes in the system */
int get_map_connected_graph (struct Instance_Map *nodes_map);


int CHECK_nodes_in_entry_list (struct Node_Entry *list, char *source, char *dest);

void FREE_Node_Entry_list (struct Node_Entry * list);

void FREE_Record_List (struct Record_List *list);

void FREE_Instance_Map_list_node (struct Instance_Map *list);

void FREE_Node_List (struct Node_List *list);



