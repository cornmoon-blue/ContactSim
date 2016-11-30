/*****************************************************************
 This program input the GPS trace record of TWO different entity,
 and extracts the contact time and position between them during the period of records.
 The input data must includes three fields: Logitude, Latitude, Timestamp.
******************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "contact.h"
#include "config.h"

int main (argc, argv)
int argc;
char *argv[];
{
    if (argc != 3) { printf("INPUT: Two data File!\n"); exit(EXIT_FAILURE); }
    // To avoid duplicate process, the 1st(s) parameter should lower than 2nd
    if (strcmp(argv[1], argv[2]) >= 0) exit(EXIT_FAILURE);

    //TODO: read the simulation parameter from simulator.configure file
    /* the following parameters are difference with Dataset */
    int longitude_filed = LONF;
    int latitude_field = LATF;
    int time_field = TIMEF;

    FILE *fp_s, *fp_d;
    struct Record_List *record_s;  // List head of first node
    struct Record_List *record_d;  // List head of second node

    if((fp_s = fopen(argv[1], "r"))==NULL) {
        printf("ERROR: Can't Open SOURCE NODE File %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    if((fp_d = fopen(argv[2], "r"))==NULL) {
        printf("ERROR: Can't Open DESTINATION File %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    char *output_fname, *s1, *s2, *s3;
    char *saveptr1, *saveptr2;

    // Used to deal with absolute path, "strrchr" get the last directory name
    s1 = strrchr (argv[1], (int)('/'));  s2 = strrchr (argv[2], (int)('/'));
    char *sepd="/";
    char *delm = strsep (&s1, sepd);  delm = strsep (&s2, sepd); // delm is no use.

    printf("%s,  %s\n", s1, s2); // TEST: output some informamtion
    // Get the output file name.  Use "." to split string
    output_fname = strcat (strcat (strtok_r(s1, ".", &saveptr1), "-"), strtok_r(s2, ".", &saveptr2));  


    // Read data from files, and store thme to a list
    record_s = read_gps_record (fp_s, longitude_filed, latitude_field, time_field, 0);
    record_d = read_gps_record (fp_d, longitude_filed, latitude_field, time_field, 0);
//    TEST_record_list_read (record_s);
//    TEST_record_list_read (record_d);


    double d;
    struct Record_List *Rec_s, *Rec_d;    struct Contact_Position_List contact_position;
    Rec_s = record_s; Rec_d = record_d;
    struct Contact_Position_List *CTL = &contact_position; // get the entrance

    // Get their Conatct Trace
    for ( ; Rec_s != NULL && Rec_d != NULL; Rec_s = Rec_s->next, Rec_d = Rec_d->next ) {
	// First, Align the record with time
	record_time_alignment (&Rec_s, &Rec_d);

	if (Rec_s->gps.Lon - Rec_d->gps.Lon < 0.01 && Rec_s->gps.Lat - Rec_d->gps.Lat < 0.008) {
		d = geo_distance (Rec_s->gps, Rec_d->gps);
		if (d < RADIO_RANGE)
			CTL = record_contact (CTL, (Rec_s->gps.Lon + Rec_d->gps.Lon)/2, 
						   (Rec_s->gps.Lat + Rec_d->gps.Lat)/2, Rec_s->gps.utime);
//			printf ("%f  %f  %f  %f  %f  %ld  %ld\n", Rec_s->gps.Lon, \
		Rec_s->gps.Lat, Rec_d->gps.Lon, Rec_d->gps.Lat, d, Rec_s->gps.utime, Rec_d->gps.utime);
	}
    }

    // get the Contact trace for a pair of nodes
    struct Contact_Record_List *contact_trace;
    CTL = contact_position.next;
    contact_trace = calculate_contact_duration (CTL);

    struct Contact_Record_List *cthead = contact_trace;
    FILE *output_fp;
    if ((output_fp = fopen (output_fname, "w")) == NULL)
	printf ("Can't Open File %s\n", "xxx");
    else 
	for ( ; cthead != NULL; cthead = cthead->next ) {
//		printf ("%ld %ld %ld\n", cthead->interval, cthead->begin, cthead->duration);
		fprintf (output_fp, "%ld %ld %ld %lf %lf\n", cthead->interval, cthead->begin, cthead->duration,
			cthead->location.Lon, cthead->location.Lat);
	}
    printf ("OUTPUT Contact Trace: %s\n", output_fname);


    // end: free the malloced memory
    cthead = contact_trace;
    for ( ; cthead != NULL; cthead = cthead->next ) free (cthead);
    for ( ; CTL != NULL; CTL = CTL->next ) free(CTL);

    struct Record_List *R; 
    for ( R = record_s; R != NULL; R = R->next )    free (R);
    for ( R = record_d; R != NULL; R = R->next )    free (R);

    fclose (fp_s);  fclose (fp_d); fclose (output_fp);

    return 0;
}


