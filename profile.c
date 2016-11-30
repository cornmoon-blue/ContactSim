/********************************************
  This file impletemnts the profile and test function.
*******************************************/
#include <stdio.h>
#include "contact.h"
//#include "session_set.h"
//#include "nodes.h"

void TEST_record_list_read (struct Record_List *L);
void TEST_position_map (struct Instance_Map *map);
void TEST_instance_map_connectivity (struct Instance_Map *map);
void TEST_output_a_node_list (struct Node_List * L);

/************************************************************/
// output the trace record loaded from GPS trace file
void TEST_record_list_read (struct Record_List *L)
{
    printf ("%.6f %.6f %ld\n", L->gps.Lon, L->gps.Lat, L->gps.utime);
    struct Record_List *T = L;
    while (T->next) {
	T = T->next;
	printf ("%.6f %.6f %ld\n", T->gps.Lon, T->gps.Lat, T->gps.utime);
    }
}

/**************************************************************/
//output the instance location of each node at a specified moment
void TEST_position_map (struct Instance_Map *map)
{
    struct Instance_Map *m = map;
    while (m) {
	printf("%d %s %f %f %ld\n", m->num, m->node_id, m->gps.Lon, m->gps.Lat, m->gps.utime);
	m = m->next;
    }
    return;
}

/**************************************************************/
// output the one hop neighbours for each node at certain moment
void TEST_instance_map_connectivity (struct Instance_Map *map)
{
    struct Instance_Map *M;
    struct Node_List *L;
    M = map;
    while (M) {
	printf("%4d %s:", M->num, M->node_id);
	if(M->connected_nodes_num > 0) { 
	    L = M-> connected_nodes;
	    while (L) { 
//		printf(" %d %s %f;", L->node.num, L->node.node_id, L->weight); 
		printf(" %d %s;", L->node.num, L->node.node_id);
		L = L->next; 
	    }
	}
        printf("\n");
	M = M->next;
    }
    return;
}

/**************************************************************/
//output a node list
void TEST_output_a_node_list (struct Node_List * L)
{
    struct Node_List *list = L;
    while (list) {
	printf(" %d %s %f;", list->node.num, list->node.node_id, list->weight);
	list = list->next;
    }
    printf("\n");
    return;
}






