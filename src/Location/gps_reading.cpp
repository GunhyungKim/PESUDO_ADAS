/*
 *
 *  See the license file included with this source.
 */
 
#include<gps_reading.hpp>

using namespace std;
using namespace nmea;

static int fd1;
static char buff[1000];
static int rd, nbytes, tries;


static pthread_t thread;
static mutex gps_mutex;
namespace location{
GPSData current_loc;
bool print_gps_log = false;

void* gps_thread_func(void*){

	// Fill with your NMEA bytes... make sure it ends with \n
	string line;
	


	// Create a GPS service that will keep track of the fix data.
	NMEAParser parser;
	GPSService gps(parser);
	parser.log = false;
	if(print_gps_log)
	{
	cout << "Fix  Sats  Sig\t\tSpeed    Dir  Lat         , Lon           Accuracy" << endl;
	// Handle any changes to the GPS Fix... This is called whenever it's updated.
	gps.onUpdate += [&gps](){
		cout << (gps.fix.locked() ? "[*] " : "[ ] ") << setw(2) << setfill(' ') << gps.fix.trackingSatellites << "/" << setw(2) << setfill(' ') << gps.fix.visibleSatellites << " ";
		cout << fixed << setprecision(2) << setw(5) << setfill(' ') << gps.fix.almanac.averageSNR() << " dB   ";
		cout << fixed << setprecision(2) << setw(6) << setfill(' ') << gps.fix.speed << " km/h [" << GPSFix::travelAngleToCompassDirection(gps.fix.travelAngle, true) << "]  ";
		cout << fixed << setprecision(6) << gps.fix.latitude << "deg " "N, " << gps.fix.longitude << "deg " "E" << "  ";
		cout << "+/- " << setprecision(1) << gps.fix.horizontalAccuracy() << "m  ";
		cout << endl;
	};
	}


	// -- STREAM THE DATA  ---

	// From a buffer in memory...
	/*
	while (1)
	{
		line = "";
		cin >> line;
		if (line == "")
			continue;
		line += "\n";
		
		parser.readLine(line);
		// Show the final fix information
		cout << gps.fix.toString() << endl;
	}

	cin.ignore();*/


	fd1 = open("/dev/ttyTHS2", O_RDWR);
	

	struct termios options;
	tcgetattr(fd1, &options);          
	cfsetispeed(&options, B38400);              
	tcsetattr(fd1, TCSANOW, &options);     
	fcntl(fd1, F_SETFL,0);
	char chk_sum;
	char *pline;
	size_t len;
	while (1) {
		memset(buff, 0, 1000);
		rd = read(fd1, buff, 999);
		chk_sum = 0;
		if (rd<1)
			continue;
			if(print_gps_log)
			{
				for(int i=0;i<rd;i++){
					if(i!=0&&rd-1-i>=4)
						chk_sum ^= buff[i];
					else
						printf("'s'");
						printf("/%d %c  ",buff[i],buff[i]);
				}
				printf("chksum: 0x%0x ",chk_sum);
				printf("%d ",rd);
				printf("buff: %s", buff);
			}
		line = string(buff);
		line += "\n";
		try{
		parser.readLine(line);
		}
		catch(...){
		perror("gps parsing error\n");
		}
		if(print_gps_log)
		cout << gps.fix.toString() << endl;
		if(!gps.fix.quality)
		{
		    //perror("gps low quality\n");
		    continue;
		}
		//update location
		gps_mutex.lock();
		current_loc.altitude = gps.fix.altitude;
		current_loc.latitude = gps.fix.latitude;
		current_loc.longitude = gps.fix.longitude;
		//cout<<current_loc.altitude<<" "<<current_loc.latitude<<endl;
		current_loc.status = gps.fix.toString();
		gps_mutex.unlock();
	}
	close(fd1);





	return 0;
}


void run_gps_thread()
{
	if(thread)
	{
		perror("already running gps thread\n");
		return;
	}
	pthread_create(&thread,NULL,gps_thread_func,NULL);
}

GPSData getLocationData()
{
	gps_mutex.lock();
	GPSData gd = current_loc;
	gps_mutex.unlock();
	return gd;
}
}
