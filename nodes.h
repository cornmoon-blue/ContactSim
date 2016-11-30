

// Gps_Record defines a GPS record
typedef struct {
        double Lon;  // logitude
        double Lat;  // latitude
        long int utime;  // Unix time
} Gps_Record;

// GPS record list, which defines the trace of a node
struct Record_List {
    struct Record_List *prev;
    Gps_Record gps;
    struct Record_List *next;
};

// Node_Entry used to orgnize all the mobile nodes in the system
struct Node_Entry {
    char node_id [32];
    struct Record_List *trace_entry;
    struct Node_Entry *prev;
    struct Node_Entry *next;
};

// As a list to store the position of each mobile node at an instance time
struct Instance_Map {
    int num;
    char node_id [32];
    Gps_Record gps;
    struct Instance_Map *prev;
    struct Instance_Map *next;
    struct Node_List *connected_nodes; // store the neighbour nodes
    int connected_nodes_num;
};

/*******************************/
//#define EPIDEMIC
#define GEO_NEAREST
/*******************************/
typedef struct {
    int num;
    char node_id [32];
    Gps_Record *position;
} Anode;

typedef struct {
    Anode point1;
    Anode point2;
#ifdef GEO_NEAREST
    double weight;
#endif
#ifdef EPIDEMIC
    long int weight;
#endif
} Edge;


struct Node_List {
    Anode node;
#ifdef GEO_NEAREST
    double weight;
#endif
#ifdef EPIDEMIC
    long int weight;
#endif
    struct Node_List *next;
};

