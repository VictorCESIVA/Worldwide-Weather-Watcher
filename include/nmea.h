#ifndef NMEA_H
#define NMEA_H
#include "cesi_types.h"
bool nmea_gga(const char *ligne, GPS_Data *gps);
#endif
