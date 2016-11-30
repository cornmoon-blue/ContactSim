/*****************************************************************
 This program get the revisit time interval of a mobile node to 
   a set of hot spot.
 Input: a GPS trace record file; a <Longitude, Latitude> geo position file
******************************************************************/
#include "revisit.h"

int main (argc, argv)
int argc;
char *argv[];
{
    if (argc != 3) { printf("INPUT: Two data File! One Trace; Another Hotspot\n"); exit(EXIT_FAILURE); }

    //TODO: read the simulation parameter from contact.configure file
    /* the following parameters are difference with Dataset */
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

    char *s1, *s2; 
    // Used to deal with directory path, and finally get the file name (is also node ID)
    s1 = strrchr (argv[1], (int)('/'));  s2 = strrchr (argv[2], (int)('/')); // Please "man strrchr"
    char *sepd1="."; s1=strsep (&s1, sepd1); s2=strsep (&s2, sepd1);
    char *sepd2="/"; strsep (&s1, sepd2); strsep (&s2, sepd2); // Please "man strsep"
    printf("%s,  %s\n", s1, s2);


    struct Record_List *trace_record;  // List head of gps trace record
    struct Record_List *hotspot;  // List head of HotSpot
    /* Read data from files, and store thme to a list. NOTE, the read data is interploted */
    trace_record = read_gps_record (fp_trace, longitude_filed, latitude_field, time_field, 0);
    hotspot = read_hotspot (fp_hotspot); // the 1st and 2nd fields in hotspot are Longitude and Latitude

    cutdown_record_with_grid (trace_record); // delete the seccessive records in the same grid
//    TEST_record_list_read (trace_record);
//    TEST_record_list_read (hotspot);

    FILE *output_fp;
    char *output_fname = strcat (s1, "_revisit.data");
    if ((output_fp = fopen (output_fname, "w")) == NULL)
	printf ("Can't Open File %s\n", "xxx");
    else {
	/* Get the Hotspot revisit Data, and output to file */
	revisit_hotspot (trace_record, hotspot, output_fp);
	printf ("OUTPUT Hotspot Revisit Data: %s\n", output_fname);
    }


    // end: free the malloced memory
    struct Record_List *R; 
    for ( R = trace_record; R != NULL; R = R->next )    free (R);
    for ( R = hotspot; R != NULL; R = R->next )    free (R);

    fclose (fp_trace);  fclose (fp_hotspot);  fclose (output_fp);

    return 0;
}


