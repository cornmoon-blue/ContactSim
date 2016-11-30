# ContactSim

## Introduction
An experimental simulator to extract short range contact opportunities from GPS vehicles trajectories.
In Opportunistic Networks, the contact means the mobile nodes can communicate with each other by the wireless technology.
Beacuse nodes are mobile, the contact happens and continues uncertainly. This explains the meanning 'Opportunistic'.

## step
* Read a vehicle's GPS data from file
* Make interpolation to the GPS to get a refined trajectory
* Align the GPS records time of two nodes
* Calculate the distance of the node pairs, and judge if they are in communication range
* Record the contact opportunities
* Examine different opportunistic routing methods 


This job is mainly worked in 2012.
