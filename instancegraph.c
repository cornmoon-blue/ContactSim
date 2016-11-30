/****************************************************************
 This file implements the connectivity of a instance position map
******************************************************************/
#include <unistd.h>
#include "instancegraph.h"

/*****************************************************************/

/*****************************************************************/
/* This function read the trace of all nodes into a list.        *
 * Input: the directory path where the nodes' traces are stored. *
 * Note that, this directory should no other file except trace.  */
struct Node_Entry * read_nodes_trace (char *dir_name, long int cutdown_time) {
    DIR *dp;  // directory pointer
    if ((dp = opendir(dir_name)) == NULL) {
      fprintf (stderr, "cannt open directory: %s\n", dir_name);
      return NULL;
    }
    char *return_path; return_path = (char *)get_current_dir_name(); // store the current work directory
    chdir (dir_name);
    struct dirent *entry;
    FILE *ftrace;  char fname[256]; fname[0]='\0';
    struct Node_Entry *head, *TMP, *M;  struct Record_List *L;

  head = malloc (sizeof(struct Node_Entry));
  TMP = head; TMP->prev = NULL; 
  while ((entry = readdir (dp)) != NULL) { // for each trace file in this directory
     if (strcmp(".", entry->d_name)==0 || strcmp("..", entry->d_name)==0) continue;
     strncpy (TMP->node_id, entry->d_name, strcspn(entry->d_name, ".")); // get the node ID
     strcat(strcat(strcpy(fname, dir_name), "/"), entry->d_name); // get the trace file path

     if ((ftrace = fopen(fname, "r"))==NULL) {
 	printf("ERROR: Can't Open Trace File %s\n", fname); exit(EXIT_FAILURE);
     }
     L = read_gps_record (ftrace, LONF, LATF, TIMEF, cutdown_time); // read the trace of a node
     if(L==NULL) { fclose (ftrace); continue; }
     TMP->trace_entry = L;
//TEST_record_list_read (L);
//     TMP->trace_entry = cutdown_record_with_time (L, cutdown_time); // in order to save memory
     fclose (ftrace);

     M = malloc (sizeof(struct Node_Entry));
     TMP->next = M; TMP->prev = NULL;
     TMP = TMP->next;
  }
  TMP = head;
  while (TMP->next) { /* construct the "prev" link */
     M = TMP->next; M->prev = TMP; TMP = TMP->next;
  }
  TMP = M->prev; TMP->next = NULL; free(M); /* set the last' "next" as NULL */

  chdir (return_path);  free(return_path); // restore the current work directory
  closedir (dp);
  return head;
}

/*****************************************************************/
/* this function gets the GPS location of all nodes at geiven time instance */
struct Instance_Map * get_instance_position_map (struct Node_Entry *nodes_list, long int time) 
{
  struct Node_Entry *node = nodes_list;
  struct Record_List *trace;
  struct Instance_Map *map, *P, *M;
  map = malloc (sizeof(struct Instance_Map)); M = map; P = map; 
  int k=0;
  while (node) {
     k++;
     if(trace = node->trace_entry) {
         P->num = k; strcpy (P->node_id, node->node_id); // record the node id
         /* set the defalt position as (0,0) if time out range */
         get_instance_position_from_trace (trace, P, time);

         P->connected_nodes = NULL; P->connected_nodes_num = 0;
     } else { //if a null entry,Go on; if tail entry, break;
	 if(node->next) { node = node->next; continue; }else{ P->next = NULL; break; } 
     }
     if (node->next) { 
	 node = node->next;
         M = malloc (sizeof(struct Instance_Map));
         P->next = M; M->prev = P; P = P->next;
     } else { P->next = NULL; break; }
  }
  return map;
}

/*****************************************************************************/
// scan the trace to find the position at given time instance; store it into the map
int get_instance_position_from_trace (struct Record_List *trace, struct Instance_Map *map_node, long int time)
{
    struct Record_List *P, *N;
    while(trace->next){
        P = trace; N = trace->next;
        if ((P->gps.utime - time > 0 && N->gps.utime - time < 0) || 
            (P->gps.utime - time < 0 && N->gps.utime - time > 0)) {
            map_node->gps.Lon = (1 - (P->gps.utime - time)/(P->gps.utime - N->gps.utime))*P->gps.Lon;
            map_node->gps.Lat = (1 - (P->gps.utime - time)/(P->gps.utime - N->gps.utime))*P->gps.Lat;
            map_node->gps.utime = time;
            return 1;
        } else
            trace = trace->next;
    }

    map_node->gps.Lon = 0; map_node->gps.Lat = 0; map_node->gps.utime = time;
    return 0;
}

/*****************************************************************************/
struct Instance_Map *find_node_from_instance_map (struct Instance_Map *map, char *id)
{
    struct Instance_Map *P, *T;
    P = map;
    while (P){
        if(strcmp(P->node_id, id) == 0){ T = P; return T; }
        P = P->next;
    }
printf("-----------\n");
    return NULL;
}

/*****************************************************************/
/* this function gets the connected node set for a specified node
 * Input: the nodes position Map; and the reference node  
 * Output: the list of connected nodes for the reference */
int get_connected_nodes_for_specified_node(struct Instance_Map *map, struct Instance_Map *spec_node)
{
  struct Instance_Map *M;
  struct Node_List *net_node, *Nprev, *Nnext, *T;
  net_node = malloc (sizeof(struct Node_List)); /* prepare for the first connected node */
  double d; int k;
  M = map; Nprev = net_node; k = 0;
  if(spec_node->gps.Lon == 0) return 0;
  while (M) {
     /* skip if it is the specified node */
//     if(M == spec_node) { M = M->next; continue; } // NOTE: always error, i donot know why?
     // Remember: we reserve itself in the connected set
     d = geo_distance (M->gps, spec_node->gps); // get the distance 
     if(d<RADIO_RANGE) { // if is a connected node, then 
	Nprev->node.num = M->num;
        strcpy(Nprev->node.node_id, M->node_id);
        Nprev->node.position = &(M->gps);
	Nprev->weight = 1-d/RADIO_RANGE;
        k++; //counting
        Nnext = malloc (sizeof(struct Node_List));
        T = Nprev; Nprev->next = Nnext; Nprev = Nprev->next;
     }
     M = M->next; // check next 
  }
  free (Nnext); T->next = NULL; /*there always an empty member at tail, delete it*/
  if (k>0) spec_node->connected_nodes = net_node; /*return the header of connected node list*/
  return k;
}

/*****************************************************************/
/* this function gets the connected node set for all nodes in the system */
int get_map_connected_graph (struct Instance_Map *nodes_map)
{
  struct Instance_Map *map, *M;
  map = nodes_map; M = nodes_map;
  while (M) {
     M->connected_nodes_num = get_connected_nodes_for_specified_node (map, M);
     M = M->next;
  }
  return 0;
}

/*******************************************************************/
int CHECK_nodes_in_entry_list (struct Node_Entry *list, char *source, char *dest)
{
    struct Node_Entry *T;  int FS; int FD;
    T = list; FS = 0; FD = 0;
    while(T){
	if(strcmp(T->node_id, source)==0) FS = 1;
	if(strcmp(T->node_id, dest)==0) FD = 1;
	T = T->next;
    }
    if(FS*FD) return 1;
    return 0;
}

/*****************************************************************/

void FREE_Node_Entry_list (struct Node_Entry * list)
{
    struct Node_Entry *T, *M;
    T = list;
    while (T){
	FREE_Record_List (T->trace_entry);
	M = T; T = T->next;
	free(M);
    }
    return;
}

void FREE_Record_List (struct Record_List *list)
{
    struct Record_List *T, *M;
    T = list;
    while (T){
        M = T; T = T->next;
        free(M);
    }
    return;
}

void FREE_Instance_Map_list_node (struct Instance_Map *list)
{
    struct Instance_Map *T, *M;
    T = list;
    while (T){
        M = T; T = T->next;
        if(M->connected_nodes) FREE_Node_List (M->connected_nodes);
        free(M);
    }
    return;
}

void FREE_Node_List (struct Node_List *list)
{
    struct Node_List *T, *M;
    T = list;
    while (T){
        M = T; T = T->next; free(M);
    }
    return;
}







