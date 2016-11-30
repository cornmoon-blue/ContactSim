/* this file includes some configutr paramenters used in the simulators */

#define DEBUG

/**************************************************************/
#define INTERVAL 5  // used for GPS interpolation
#define GAP 120     // When cab stationary for a long time, it wll not do interpolation
#define RADIO_RANGE 100 // Wireless Ratio communication range

//used in vanetsim.h::cutdown_record_with_time(); in order to save memory, low value, low cost
#define RECORD_TIME_RANGE 48

#define SF_CAB 1
#if SF_CAB
  #define LONF 2
  #define LATF 1
  #define TIMEF 4
#else  //For Shanghai Taxi
  #define LONF 3
  #define LATF 4
  #define TIMEF 7
#endif

/******************************************/
#define EARTH_RADIUS_METER 6378137.0
#define deg2rad(d)  (d * 3.14159265358979 / 180.0)

/******************************************/
//multi-copy num : the message copies number in the forwarding strategies
// NOTE, Now 8 copies at most, limited by the hotspot assisted forwarding
#define MULTICPOY_NUM 1  







