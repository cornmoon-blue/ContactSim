/* This file includes some typical routing schemes for OpportunisticNetwork and DTN */
//#include "vanetsim.h"
//#include "contactm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//#include "config.h"
//#include "nodes.h"
#include "instancegraph.h"

struct Node_List *epidemic (struct Node_Entry *trace_entry_list, char *source, char *dest, long int time, long int stop_time);
struct Node_List *get_infected_node_in_an_instance_map (struct Node_List *head, struct Instance_Map *map);
void OUTPUT_epidemic_result (struct Node_List *infected_list, char *source, char *dest, long int time, FILE *fp);

struct Node_List * nearest_geo_distance_forwarding_to_fixed_node (struct Node_Entry *trace_entry_list, char *source, Gps_Record *fixed_position, long int time, long int stop_time);
struct Node_List *get_nearest_node_to_fixed_point (struct Node_List *linked, Gps_Record *fixed);
struct Node_List *copy_Node_info_from_Instance_Map (struct Node_List *N, struct Instance_Map *M);
struct Node_List *copy_Node_info (struct Node_List *S, struct Node_List *D);
struct Node_List *ADD_new_Node_to_list (struct Node_List *list, struct Node_List *new_node);
void OUTPUT_forward_path (struct Node_List *path, char *source, char *dest, long int time, char *fname);


/***********************************************************************************/
/* this function input the soruce, destination, and message time; output the infected 
 * node set with the "weight" as infected time */
struct Node_List *epidemic (struct Node_Entry *trace_entry_list, char *source, char *dest, long int time, long int stop_time)
{
    struct Instance_Map *map, *P, *S, *D, *T;
    struct Node_List *sick_head, *L, *M, *N;
    long int t = time;  long int anchor_time;
    M = malloc(sizeof(struct Node_List)); sick_head = M;
    while (t < stop_time) {
        map = get_instance_position_map (trace_entry_list, t); // the initial network state
        P = map;
        while (P){ // find the source and destination node
            if(strcmp(P->node_id, source) == 0) { S = P; T = P; } //T used to tag the source node
	    P = P->next;
        }
        // if S is not exist in this moment graph, read next graph;
        if(S) break; else{ t += INTERVAL; FREE_Instance_Map_list_node (map); }
    }

    if(S) { // initializing the infected node list, add S to the list as original sick node
	M->node.num = S->num; strcpy(M->node.node_id, S->node_id); M->node.position = NULL;
    	M->weight = S->gps.utime; M->next = NULL; // note, weight is used to record the infected time
	anchor_time = S->gps.utime;
    }else{
	return NULL; // can'nt find source node, an invalidit test case
    }
    //---------------------------------------------------------------------------------
    while (t < stop_time) {
	sick_head = get_infected_node_in_an_instance_map(sick_head, map);
	L = sick_head;
	while (L) {
	    if(strcmp(L->node.node_id, dest) == 0) 
                return sick_head; // the destination has been infected
	    L = L->next; //counter++;
	}
	// if destination node infected, go on to spread
	t += INTERVAL;  FREE_Instance_Map_list_node (map);
	if(t > anchor_time + 3*3600) break; 
	map = get_instance_position_map (trace_entry_list, t); 
    }
    //------------------------------------------------------------------------------------
    // if destination is not infected to the time end; return a sick list append destination at tail with weight of 0
    N = sick_head;
    while(N->next){ N = N->next; }
    M = malloc(sizeof(struct Node_List)); N->next = M; M->next = NULL; 
    M->node.num = 0; strcpy(M->node.node_id, dest); M->node.position = NULL; M->weight = 0;
    return sick_head;
}


/*********************************************************************************/
struct Node_List *get_infected_node_in_an_instance_map (struct Node_List *head, struct Instance_Map *map)
{
    struct Instance_Map *P, *source;
    struct Node_List *H, *L, *M, *tail, *index;
    int SICK_FLAG = 0;
    index = head; H = head; tail = head;
    while(index){
	P = map;
        while(P){ // find the current source at first; NOTE, this source is always exist inthe link
    	    if(strcmp(P->node_id, index->node.node_id) == 0) { source = P; break; }
	    else P = P->next;
	}
	//get the infected nodes
        source->connected_nodes_num = get_connected_nodes_for_specified_node (map, source); 
	L = source->connected_nodes;
	while(L){
	    while(H){ // find if the new infected node is alreadly a sick node in the gloable list
		if(strcmp(L->node.node_id, H->node.node_id) == 0) { SICK_FLAG = 1; break; }
		else { if(H->next) tail = H->next;  H = H->next; } // tail used to remember the tail globle list
	    }
	    if(SICK_FLAG == 0){ // L is a new sick node
		M = malloc(sizeof(struct Node_List));
//                M->node.num = L->node.num; strcpy(M->node.node_id, L->node.node_id);
//                M->node.position = NULL; M->weight = source->gps.utime; M->next = NULL;
//                tail->next = M;
tail->next = copy_Node_info (M, L); // add the new sick node to the tail
	    }
	    L = L->next; H = head; SICK_FLAG = 0;  // check the next
	}

	index = index->next; 
    }
    return head;
}

/**********************************************************************************/
void OUTPUT_epidemic_result (struct Node_List *infected_list, char *source, char *dest, long int time, FILE *fp)
{
    struct Node_List *head, *tail, *T;
    int sick_num = 0; long int latency, delay;
    head = infected_list; tail = infected_list; T = NULL;
    // in fact, the head is the source node; but the tail is not certainly the destination
    while(tail){
        sick_num++;
	if(strcmp(tail->node.node_id, dest)==0) T = tail;
	tail = tail->next;
    }
    if(T) { 
	latency = T->weight - time; delay = T->weight - head->weight;
    }else{ latency = 0; delay = 0; }
    fprintf(fp," S %12s; D %12s; T %ld latency: %ld; delay %ld; Infected: %d\n", 
                  source, dest, time, latency, delay, sick_num);
    return;
}


/***********************************************************************************/
/***********************************************************************************/
/* this function forwards message from a mobile node to a fixed node (or position); *
 * message carrier selects the node nearest to destination as relay                 */
struct Node_List * nearest_geo_distance_forwarding_to_fixed_node (struct Node_Entry *trace_entry_list, 
                                                    char *source, Gps_Record *fixed_position, 
                                                    long int time, long int stop_time) 
{
    struct Instance_Map *map, *P, *S, *T;
    struct Node_List *forward_path, *connected_nodes, *M;
    long int t = time;  long int anchor_time;
    M = malloc(sizeof(struct Node_List)); forward_path = M;
    M = malloc(sizeof(struct Node_List)); connected_nodes = M;
//    int FAST = 0;
    while (t < stop_time) {
        map = get_instance_position_map (trace_entry_list, t); // the initial network state
        S = (struct Instance_Map *) find_node_from_instance_map (map, source);
        if(S) if(S->gps.Lon * S->gps.Lat != 0) break; 
        // if S is not exist in this moment graph, go back to read next instance graph;
//        if(FAST==0) { t += trace_entry_list->trace_entry->gps.utime - t; FAST = 1; } //NOTE: this big jump only can use once
        t += INTERVAL; FREE_Instance_Map_list_node (map); 
    }
    if (S) { // record the original source node as Header
        forward_path = copy_Node_info_from_Instance_Map (forward_path, S);
        connected_nodes = copy_Node_info_from_Instance_Map (connected_nodes, S);
    } else return NULL;

    Gps_Record X, Y;
    X.Lon = S->gps.Lon; X.Lat = S->gps.Lat; X.utime = S->gps.utime;
    Y.Lon = fixed_position->Lon; Y.Lat = fixed_position->Lat; Y.utime = fixed_position->utime;
    forward_path->weight = geo_distance (X, Y); 

    struct Node_List *Hp, *Hc, *N, *L; 
    Hp = forward_path;  Hc = connected_nodes;
    while(Hp->weight > RADIO_RANGE) {
        Hc = get_infected_node_in_an_instance_map(Hc, map); // get the connected nodes
//L=Hc; int lth=0; while(L){ L=L->next; lth++; }
        //find out the nearest node as relay from the connected list
        N = get_nearest_node_to_fixed_point(Hc, fixed_position); 
//printf("  T1-3  %ld %s %f %f; %f\n", t, N->node.node_id, N->node.position->Lon, N->node.position->Lat, N->weight);
//printf("  T1-3  connect %d\n", lth);
        // the minimal distance has stored in N->weight
        forward_path = ADD_new_Node_to_list (forward_path, N);
//TEST_output_a_node_list(forward_path);
        L = forward_path; while(L->next) L = L->next;
	FREE_Instance_Map_list_node (map);
        FREE_Node_List (Hc);

        if (L->weight > RADIO_RANGE) {
            while(1) {
                t += INTERVAL;
                map = get_instance_position_map (trace_entry_list, t);
                S = (struct Instance_Map *) find_node_from_instance_map (map, L->node.node_id);
                if(S) if(S->gps.Lon * S->gps.Lat == 0) continue;  //beacuse S maybe (0,0), if so, must read next graph
                Hc = malloc(sizeof(struct Node_List));
                Hc = copy_Node_info_from_Instance_Map (Hc, S);
                break;
            }
        }else
            break;
    }
//printf("T1-4  %ld\n", t);
    // add the hotspot into forward_path
    M = malloc (sizeof(struct Node_List));
    M->node.num = 0; strcpy(M->node.node_id, "HOTSPOT");
    M->node.position = malloc(sizeof(Gps_Record));
    M->node.position->Lon = fixed_position->Lon; M->node.position->Lat = fixed_position->Lat; 
    M->node.position->utime = t; // the message time
    M->weight = forward_path->weight;
    L = forward_path; while(L->next){ L = L->next; }
    L->next = M; M->next = NULL;
    return forward_path;
}

/*************************************************************/
struct Node_List *copy_Node_info_from_Instance_Map (struct Node_List *N, struct Instance_Map *M)
{
    N->node.num = M->num; strcpy(N->node.node_id, M->node_id); 
    N->node.position = malloc(sizeof(Gps_Record));
    N->node.position->Lon = M->gps.Lon; N->node.position->Lat = M->gps.Lat; N->node.position->utime = M->gps.utime;
    N->weight = 0; N->next = NULL;
    return N;
}

struct Node_List *copy_Node_info (struct Node_List *S, struct Node_List *D)
{
    S->node.num = D->node.num; strcpy(S->node.node_id, D->node.node_id);
    if(D->node.position) {
        S->node.position = malloc(sizeof(Gps_Record));
        S->node.position->Lon = D->node.position->Lon;  S->node.position->Lat = D->node.position->Lat;
        S->node.position->utime = D->node.position->utime;
    }else S->node.position = NULL;
    S->weight = D->weight; S->next = NULL;
    return S;
}

/*************************************************************/
// input a node list and the position of the fixed node
struct Node_List *get_nearest_node_to_fixed_point (struct Node_List *linked, Gps_Record *fixed)
{
    double d, min; double dlon, dlat;
    struct Node_List *relay, *N;
    relay = linked; N = linked; min = 100000000;
    while (N) {
        d = geo_distance (*(N->node.position), *fixed);
        if(d<min) {  min = d; relay = N; }
//        dlon = N->node.position->Lon - fixed->Lon;  dlat = N->node.position->Lat - fixed->Lat;
//        if (dlon > -0.001 && dlon < 0.001 && dlat > -0.001 && dlat < 0.001){ relay = N; }
        N = N->next;
    }
    relay->weight = min;
    return relay;
}

/*************************************************************/
// add a node into list, if the node is already existing, update its weight, else add to the tail
struct Node_List *ADD_new_Node_to_list (struct Node_List *list, struct Node_List *new_node)
{
    struct Node_List *L, *T, *M;
    L = list;
    while (L) { 
        if(strcmp(L->node.node_id, new_node->node.node_id) == 0){//find it already in the list 
            if(L->next == NULL) {// and it is the tail
                if(L->weight > new_node->weight) {
                    L->weight = new_node->weight; //update the weight
                    return list;
                }else return list; // although it is tail, but not nearer, do nothing and return
            }
        }
        T = L;  L = L->next;
    }
    // T is the tail now
    if(T->weight > new_node->weight){
        M = malloc(sizeof(struct Node_List));
        T->next = copy_Node_info (M, new_node); // add new node the tail
    }
    return list;
}

/*************************************************************/
void OUTPUT_forward_path (struct Node_List *path, char *source, char *dest, long int time, char *fname)
{
    FILE *output_fp;
    if ((output_fp = fopen(fname, "a")) == NULL)
    {    printf ("Can't Open File %s\n", fname); exit(EXIT_FAILURE); }

    struct Node_List *head, *tail, *T;
    int hops = 0; long int latency, delay; double distance;
    head = path; tail = path; T = path;
    while(T->next){
        hops++; T = T->next;
    }
    latency = T->node.position->utime - head->node.position->utime;
    delay = T->node.position->utime - time;
    distance = head->weight;

//printf(" S %12s; D %12s; T %ld latency: %ld; delay: %ld; distance %f; hops_number: %d; HOTSPOT: [%f, %f]\n", 
//        source, dest, time, latency, delay, distance, hops, T->node.position->Lon, T->node.position->Lat);

fprintf(output_fp, " S %12s; D %12s; T %ld latency: %ld; delay: %ld; distance %f; hops_number: %d; HOTSPOT: [%f, %f]\n",
        source, dest, time, latency, delay, distance, hops, T->node.position->Lon, T->node.position->Lat);
    fclose(output_fp);
    return;
}





