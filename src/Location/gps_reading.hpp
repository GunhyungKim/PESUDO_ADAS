#include <iostream>
#include <fstream>
#include <iomanip>
#include "nmea.h"
#include <location.hpp>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <mutex>


namespace location{
    extern bool print_gps_log;
    void run_gps_thread();
    GPSData getLocationData();
}