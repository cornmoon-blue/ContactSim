/*************************************************************************
  this program is the simulator of the connectivity of vehicles in VANET.
  input: the gps trace data of vehicles

  NOTE: the message injection time of test case in the testset should be sorted from down to up.
***************************************************************************/
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
//#include "config.h"
//#include "nodes.h"
#include "instancegraph.h"
#include "session_set.h"

extern struct Node_List *epidemic (struct Node_Entry *trace_entry_list, char *source, char *dest, long int time, long int stop_time);

#if SF_CAB
//  char *TRACE_PATH = "/20120520/cabspotting/GpsRecord";
  char *TRACE_PATH = "/20120520/code/ContactSim/test/trace";
#else  //For Shanghai Taxi
char *TRACE_PATH = ""
#endif

//#define EPIDEMIC  // for epidemic compile
#define GEO_NEAREST  // for geo nearest strategy compile


/******************************************************************************/
int main (int argc, char *argv[]) {
    FILE *fp_session;
    if((fp_session = fopen(argv[1], "r"))==NULL) {
        printf("ERROR: Can't Open Test Session File %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

#ifdef GEO_NEAREST
      FILE *fp_hotspot; // the second parameter is expected as hotspots
      if((fp_hotspot = fopen(argv[2], "r"))==NULL) {
          printf("ERROR: Can't Open Hotspots File %s\n", argv[1]);
          exit(EXIT_FAILURE);
      }
#endif

    struct Session *testcases, *TC;  
    // read the forwarding evaluation test cases
    testcases = read_test_session (fp_session);
#ifdef GEO_NEAREST
    struct Hot_Spot *hotsopts, *HP;
    hotsopts = read_hot_spots_for_each_node (fp_hotspot);
//TEST_hot_spot_points_read (hotsopts);
#endif

    // we expect to get the minimal time (the earliest) to load record; 
    // actually, the first is the minimum, if testset is sorted from down to up
    long int record_load_point = testcases->time;
printf("load time %ld\n", record_load_point);
    long int STOP_time = record_load_point + RECORD_TIME_RANGE*3600;

    struct Node_Entry *nodes_trace_list;
    // read all nodes' trace into a link list; The load range is determined by Tmin
    printf("Initializing ... ...\n");
    nodes_trace_list = read_nodes_trace (TRACE_PATH, record_load_point);  
//struct Node_Entry *E=nodes_trace_list; while(E){ if(strcmp(E->node_id, "new_aslagni")==0) break; else E=E->next; }
//TEST_record_list_read (E->trace_entry); return 0;
//struct Instance_Map *nodes_map = get_instance_position_map (nodes_trace_list, 1211648432);
//TEST_position_map (nodes_map);
//get_map_connected_graph (nodes_map);
//TEST_instance_map_connectivity (nodes_map);
//return 0;

    char *source, *dest; long int time;
    FILE *output_fp; 
    printf("EVALUATING ... ...\n");
#ifdef EPIDEMIC
    char *epidemic_output_fname = "epidemic.data";
    if ((output_fp = fopen(epidemic_output_fname, "w")) == NULL)
    {    printf ("Can't Open File %s\n", epidemic_output_fname); exit(EXIT_FAILURE); }
#endif
    clock_t start, end;
    struct Node_List *epidemic_list, *forward_path;    
    struct Position_List *hot_spots_of_anode, *spot;
    TC = testcases; int counter=0; int k_copy = 0;
    while(TC){
        start = clock();
        source = TC->source; dest = TC->dest; time = TC->time;
	// if the testcase start time exeeds certain range (ex.2H), we should reload the trace records
	if(time+3*3600 > STOP_time) {
	    record_load_point = time;
	    FREE_Node_Entry_list (nodes_trace_list);
	    nodes_trace_list = read_nodes_trace (TRACE_PATH, record_load_point);
	    STOP_time = record_load_point + RECORD_TIME_RANGE*3600;
	}

#ifdef EPIDEMIC
        // check if the S and D exist in the loaded trace records; if both exist, then test
        if( CHECK_nodes_in_entry_list(nodes_trace_list, source, dest) )
            epidemic_list = epidemic(nodes_trace_list, source, dest, time, STOP_time);
        else
  	    epidemic_list = NULL;
//TEST_output_a_node_list(epidemic_list);
        OUTPUT_epidemic_result (epidemic_list, source, dest, time, output_fp);
	FREE_Node_List (epidemic_list);
#endif

#ifdef GEO_NEAREST
        char *geo_nearest_output_fname = "geo_nearest.data";

        if( CHECK_nodes_in_entry_list(nodes_trace_list, source, dest) ){
            hot_spots_of_anode = GET_hot_spots_for_specified_node (hotsopts, source);
            spot = hot_spots_of_anode;   k_copy = 0;
            while(k_copy < MULTICPOY_NUM) {
printf("T0-1 %s  %f, %f\n", source, spot->gps.Lon,spot->gps.Lat);
                forward_path = (struct Node_List *) nearest_geo_distance_forwarding_to_fixed_node (nodes_trace_list, 
                                                              source, &(spot->gps), time, STOP_time);
                OUTPUT_forward_path (forward_path, source, dest, time, geo_nearest_output_fname);
TEST_output_a_node_list(forward_path);
                  k_copy++;  spot = spot->next; //if Multi-copy ...
            }
        }
#endif

        TC = TC->next;
	end = clock();
        printf("For Case %4d : %f seconds\n", ++counter, (double)(end - start)/CLOCKS_PER_SEC);
  }

#ifdef EPIDEMIC
  printf("\n  OUTPUT in file : %s\n", epidemic_output_fname);
  fclose(output_fp);
#endif
  return 0;
}



