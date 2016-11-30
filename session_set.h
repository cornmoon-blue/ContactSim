/********************************************************
* This file implement some function related to testcase */

#define SOURCE_FIELD 1
#define DEST_FIELD 2
#define TIME_FIELD 3

struct Session {
    char source[32];
    char dest[32];
    long int time;
    struct Session *next;
};

struct Position_List {
    Gps_Record gps;
    struct Position_List *next;
};

struct Hot_Spot {
    char node_id[32];
    struct Position_List *spot_list;
    struct Hot_Spot *next;
};

struct Session *read_test_session (FILE *fp);
struct Hot_Spot *read_hot_spots_for_each_node (FILE *fp);
struct Position_List * add_a_location_to_position_list (struct Hot_Spot *spots, double lon, double lat, long int time);
void TEST_hot_spot_points_read (struct Hot_Spot *spots);


/*****************************************************/
// read test session node pair from a testset file
struct Session *read_test_session (FILE *fp)
{
    char s[256];
    char *S, *D; long int Tstart;
    char *str, *token; char *delim="\t "; char *saveptr;
    struct Session *testcase, *P, *N, *T;
    testcase = malloc (sizeof(struct Session)); P = testcase;
    int i;
    while (fgets(s, 256, fp)) {
	for (i=1, str=s; ; i++, str=NULL) {
	    token = strtok_r (str, delim, &saveptr);
	    if (token == NULL) break;
	    if (i == SOURCE_FIELD) S = token;
	    else if(i == DEST_FIELD) D = token;
	    else if(i == TIME_FIELD) Tstart = atol (token);
	}
	strcpy(P->source, S); strcpy(P->dest, D); P->time = Tstart;
	N = malloc (sizeof(struct Session));
	T = P; P->next = N; P = P->next;
    }
    T->next = NULL; free(P); //the last malloc is not used
    return testcase;
}

/*****************************************************/
#define HP_ID_FIELD 1
#define HP_LON_FIELD 2
#define HP_LAT_FIELD 3

struct Hot_Spot *read_hot_spots_for_each_node (FILE *fp)
{
    char s[256];  char *id; double lon, lat;
    char *str, *token; char *delim="\t "; char *saveptr;
    struct Hot_Spot *hotsopt, *P, *N, *T;
    Gps_Record *L;
    hotsopt = malloc (sizeof(struct Hot_Spot)); P = hotsopt; P->next = NULL; 
    int i;
    while (fgets(s, 256, fp)) {
        // extract the location record
        for (i=1, str=s; ; i++, str=NULL) {
            token = strtok_r (str, delim, &saveptr);
            if (token == NULL) break;
            if (i == HP_ID_FIELD) id = token;
            else if(i == HP_LON_FIELD) lon = atof (token);
            else if(i == HP_LAT_FIELD) lat = atof (token); 
        }
        if(strcmp(P->node_id, id) == 0){
            add_a_location_to_position_list (P, lon, lat, 0);
        }else{
            P = hotsopt; 
            while(P){
                if(strcmp(P->node_id, id) == 0) {
                    add_a_location_to_position_list (P, lon, lat, 0);
                    break;
                } else { 
                    T = P; P = P->next; // P is used to locate the tail of a list
                }
            }
            if(!P){ // if P=NULL, then its a new node's hot spot
                N = malloc (sizeof(struct Hot_Spot));
                T->next = N; N->next = NULL; N->spot_list = NULL; P = N;
                strcpy(P->node_id, id);
                add_a_location_to_position_list (P, lon, lat, 0);
            } else printf("T2-11\n");
        }
    }
    T = hotsopt; hotsopt = hotsopt->next; free(T); //actually, the first malloced header is not used
    return hotsopt;
}

/****************************************************************************/
struct Position_List * add_a_location_to_position_list (struct Hot_Spot *spots, double lon, double lat, long int time)
{
    struct Position_List *M, *L;
    M = malloc (sizeof(struct Position_List));   //create a new position
    M->gps.Lon = lon; M->gps.Lat = lat; M->gps.utime = 0; 
    M->next = NULL;

    L = spots->spot_list; 
    if(!L) spots->spot_list = M; // if for a new node, this is it 1st hotspot
    else {
        while(L->next) 
            L = L->next; // find the tail
        L->next = M;
    }
    return spots->spot_list;
}

/****************************************************************************/
struct Position_List *GET_hot_spots_for_specified_node (struct Hot_Spot *spots, char *node)
{
    struct Hot_Spot *P;
    P = spots; 
    while (P) {
        if(strcmp(P->node_id, node) == 0) return P->spot_list;
        P = P->next;
    }
    return NULL;  // not found
}




/****************************************************************************/
void TEST_hot_spot_points_read (struct Hot_Spot *spots)
{
    struct Hot_Spot *P = spots; struct Position_List *L;
    while(P){
        L = P->spot_list;
        while(L){
            printf("%12s %f %f\n", P->node_id, L->gps.Lon, L->gps.Lat);
            L = L->next;
        }
        P = P->next;
    }
    return;
}

/****************************************************************************/


