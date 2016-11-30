/*****************************************************************
 This program get the waiting time of a mobile node to visit a (or a set of) 
   fixed hot spot.
 Input: a GPS trace record file of mobile node; 
	a <Longitude, Latitude> geo position file
	a time instance in UNIX time format

******************************************************************/
#include "revisit.h"

int main (argc, argv)
int argc;
char *argv[];
{
    if (argc != 4) { 
	printf("INPUT: Two data File! One Trace; Another Hotspot\n And a TIME Value\n"); 
	exit(EXIT_FAILURE); 
    }

    //TODO: read the simulation parameter from contact.configure file
    /* the following parameters are difference with Dataset, According to the first PARA trace file */
    int longitude_filed = LONF;
    int latitude_field = LATF;
    int time_field = TIMEF;

    FILE *fp_trace, *fp_hotspot;
    if((fp_trace = fopen(argv[1], "r"))==NULL) {
        printf("ERROR: Can't Open TRACE RECORD File %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    if((fp_hotspot = fopen(argv[2], "r"))==NULL) {
        printf("ERROR: Can't Open Hotspot File %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    long int time0 = atol (argv[3]); // get the begining time
//printf("time0 %ld\n", time0);

    char *s1, *s2; 
    // Used to deal with directory path, and finally get the file name (is also node ID)
    s1 = strrchr (argv[1], (int)('/'));  s2 = strrchr (argv[2], (int)('/')); // Please "man strrchr"
    char *sepd1="."; s1=strsep (&s1, sepd1); s2=strsep (&s2, sepd1);
    char *sepd2="/"; strsep (&s1, sepd2); strsep (&s2, sepd2); // Please "man strsep"
//printf("%s,  %s,  %ld\n", s1, s2, time0);


    struct Record_List *trace_record;  // List head of gps trace record
    struct Record_List *hotspot;  // List head of HotSpot
    /* Read data from files, and store thme to a list. NOTE, the read data is interploted */
    trace_record = read_gps_record (fp_trace, longitude_filed, latitude_field, time_field, 0);
    hotspot = read_hotspot (fp_hotspot); // the 1st and 2nd fields in hotspot are Longitude and Latitude

//    cutdown_record_with_grid (trace_record); // delete the seccessive records in the same grid
//    TEST_record_list_read (trace_record);
//    TEST_record_list_read (hotspot);

    /* Get the Hotspot revisit Data, and output to file */
    long int latency = wait_time_to_a_hotspot (trace_record, hotspot, time0);
    printf ("OUTPUT wait time: %ld\n", latency);

    // end: free the malloced memory
    struct Record_List *R; 
    for ( R = trace_record; R != NULL; R = R->next )    free (R);
    for ( R = hotspot; R != NULL; R = R->next )    free (R);

    fclose (fp_trace);  fclose (fp_hotspot); 

    return 0;
}


