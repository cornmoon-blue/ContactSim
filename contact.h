/* this file defines some data struct and functions used in contact simulator */
#include "nodes.h"

#define SIGN(x) (x>=0? 1 : -1)

struct Contact_Position_List {
//	long int time;
	Gps_Record contact_position_gps;
	struct Contact_Position_List * next;
};

struct Contact_Record_List {
        long int interval;
        long int begin;
        long int duration;
        Gps_Record location;
//      struct Contact_Record_List *prev;
        struct Contact_Record_List *next;
};

/***********************************************************************/
// fp: record data file; lg_f,lt_f,tm_f: the field of logitide, latitude, time; 
// time_bound is the record range to be read into memory
struct Record_List * read_gps_record (FILE *fp, int lg_f, int lt_f, int tm_f, long int time_bound); 

// Used for Cabspotting to reverse the time sequence of records
struct Record_List * reverse_gps_record_list (struct Record_List * H);

/******************************************/
struct Record_List * gps_interpolation (struct Record_List *Rec_Head, int const interval, int const gap);

/******************************************/
// Get the distance of two (long, lat) coordinates
double geo_distance (Gps_Record X, Gps_Record Y);

// Before calculate the Contact opportunisties, the most close time record should be found
int record_time_alignment (struct Record_List ** Rec_Head_S, struct Record_List ** Rec_Head_D);

// Add the censored contact time to a List
struct Contact_Position_List * record_contact (struct Contact_Position_List * C, double lon, double lat, long int t);

struct Contact_Record_List * calculate_contact_duration (struct Contact_Position_List * C);



