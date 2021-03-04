/*
 * GPSService.cpp
 *
 *  Created on: Aug 14, 2014
 *      Author: Cameron Karlsson
 *
 *  See the license file included with this source.
 */


#include <nmeaparse/GPSService.h>
#include <nmeaparse/NumberConversion.h>

#include <iostream>
#include <cmath>

using namespace std;
using namespace std::chrono;

using namespace nmea;


// ------ Some helpers ----------
// Takes the NMEA lat/long format (dddmm.mmmm, [N/S,E/W]) and converts to degrees N,E only
double convertLatLongToDeg(string llstr, string dir){

	double pd = parseDouble(llstr);
	double deg = trunc(pd / 100);				//get ddd from dddmm.mmmm
	double mins = pd - deg * 100;

	deg = deg + mins / 60.0;

	char hdg = 'x';
	if (!dir.empty()){
		hdg = dir[0];
	}

	//everything should be N/E, so flip S,W
	if (hdg == 'S' || hdg == 'W'){
		deg *= -1.0;
	}

	return deg;
}
double convertKnotsToKilometersPerHour(double knots){
	return knots * 1.852;
}



// ------------- GPSSERVICE CLASS -------------




GPSService::GPSService(NMEAParser& parser) {
	attachToParser(parser);		// attach to parser in the GPS object
}

GPSService::~GPSService() {
	// TODO Auto-generated destructor stub
}

void GPSService::attachToParser(NMEAParser& _parser){

	// http://www.gpsinformation.org/dale/nmea.htm
	/* used sentences...
	$GPGGA		- time,position,fix data
	$GPGSA		- gps receiver operating mode, satellites used in position and DOP values
	$GPGSV		- number of gps satellites in view, satellite ID, elevation,azimuth, and SNR
	$GPRMC		- time,date, position,course, and speed data
	$GPVTG		- course and speed information relative to the ground
	$GPZDA		- 1pps timing message
	$PSRF150	- gps module "ok to send"
	*/
	_parser.setSentenceHandler("PSRF150", [this](const NMEASentence& nmea){
		this->read_PSRF150(nmea);
	});
	_parser.setSentenceHandler("GPGGA", [this](const NMEASentence& nmea){
		this->read_GPGGA(nmea);
	});
	_parser.setSentenceHandler("GPGSA", [this](const NMEASentence& nmea){
		this->read_GPGSA(nmea);
	});
	_parser.setSentenceHandler("GPGSV", [this](const NMEASentence& nmea){
		this->read_GPGSV(nmea);
	});
	_parser.setSentenceHandler("GPRMC", [this](const NMEASentence& nmea){
		this->read_GPRMC(nmea);
	});
	_parser.setSentenceHandler("GPVTG", [this](const NMEASentence& nmea){
		this->read_GPVTG(nmea);
	});

}




void GPSService::read_PSRF150(const NMEASentence& nmea){
	// nothing right now...
	// Called with checksum 3E (valid) for GPS turning ON
	// Called with checksum 3F (invalid) for GPS turning OFF
}

void GPSService::read_GPGGA(const NMEASentence& nmea){
	/* -- EXAMPLE --
	$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47

	$GPGGA,205630.945,3346.1070,N,08423.6687,W,0,03,,30.8,M,-30.8,M,,0000*73		// ATLANTA!!!!

	Where:
	GGA          Global Positioning System Fix Data
	index:
	[0] 123519       Fix taken at 12:35:19 UTC
	[1-2] 4807.038,N   Latitude 48 deg 07.038' N
	[3-4] 1131.000,E  Longitude 11 deg 31.000' E
	[5] 1            Fix quality: 0 = invalid
	1 = GPS fix (SPS)
	2 = DGPS fix
	3 = PPS fix
	4 = Real Time Kinematic
	5 = Float RTK
	6 = estimated (dead reckoning) (2.3 feature)
	7 = Manual input mode
	8 = Simulation mode
	[6] 08           Number of satellites being tracked
	[7] 0.9          Horizontal dilution of position
	[8-9] 545.4,M      Altitude, Meters, above mean sea level
	[10-11] 46.9,M       Height of geoid (mean sea level) above WGS84
	ellipsoid
	[12] (empty field) time in seconds since last DGPS update
	[13] (empty field) DGPS station ID number
	[13]  *47          the checksum data, always begins with *
	*/
	try
	{
		if (!nmea.checksumOK()){
			throw NMEAParseError("Checksum is invalid!");
		}

		if (nmea.parameters.size() < 14){
			throw NMEAParseError("GPS data is missing parameters.");
		}


		// TIMESTAMP
		this->fix.timestamp.setTime(parseDouble(nmea.parameters[0]));

		string sll;
		string dir;
		// LAT
		sll = nmea.parameters[1];
		dir = nmea.parameters[2];
		if (!sll.empty()){
			this->fix.latitude = convertLatLongToDeg(sll, dir);
		}

		// LONG
		sll = nmea.parameters[3];
		dir = nmea.parameters[4];
		if (!sll.empty()){
			this->fix.longitude = convertLatLongToDeg(sll, dir);
		}


		// FIX QUALITY
		bool lockupdate = false;
		this->fix.quality = (uint8_t)parseInt(nmea.parameters[5]);
		if (this->fix.quality == 0){
			lockupdate = this->fix.setlock(false);
		}
		else if (this->fix.quality == 1){
			lockupdate = this->fix.setlock(true);
		}
		else {}


		// TRACKING SATELLITES
		this->fix.trackingSatellites = (int32_t)parseInt(nmea.parameters[6]);
		if (this->fix.visibleSatellites < this->fix.trackingSatellites){
			this->fix.visibleSatellites = this->fix.trackingSatellites;		// the visible count is in another sentence.
		}

		// ALTITUDE
		if (!nmea.parameters[8].empty()){
			this->fix.altitude = parseDouble(nmea.parameters[8]);
		}
		else {
			// leave old value
		}

		//calling handlers
		if (lockupdate){
			this->onLockStateChanged(this->fix.haslock);
		}
		this->onUpdate();
	}
	catch (NumberConversionError& ex)
	{
		NMEAParseError pe("GPS Number Bad Format [$GPGGA] :: " + ex.message, nmea);
		throw pe;
	}
	catch (NMEAParseError& ex)
	{
		NMEAParseError pe("GPS Data Bad Format [$GPGGA] :: " + ex.message, nmea);
		throw pe;
	}
}

void GPSService::read_GPGSA(const NMEASentence& nmea){
	/*  -- EXAMPLE --
	$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39

	$GPGSA,A,3,18,21,22,14,27,19,,,,,,,4.4,2.7,3.4*32

	Where:
	GSA      Satellite status
	[0] A        Auto selection of 2D or 3D fix (M = manual)
	[1] 3        3D fix - values include: 1 = no fix
	2 = 2D fix
	3 = 3D fix
	[2-13] 04,05... PRNs of satellites used for fix (space for 12)
	[14] 2.5      PDOP (dilution of precision)
	[15] 1.3      Horizontal dilution of precision (HDOP)
	[16] 2.1      Vertical dilution of precision (VDOP)
	[16] *39      the checksum data, always begins with *
	*/


	try
	{
		if (!nmea.checksumOK()){
			throw NMEAParseError("Checksum is invalid!");
		}

		if (nmea.parameters.size() < 17){
			throw NMEAParseError("GPS data is missing parameters.");
		}


		// FIX TYPE
		bool lockupdate = false;
		uint64_t fixtype = parseInt(nmea.parameters[1]);
		this->fix.type = (int8_t)fixtype;
		if (fixtype == 1){
			lockupdate = this->fix.setlock(false);
		}
		else if (fixtype == 3) {
			lockupdate = this->fix.setlock(true);
		}
		else {}


		// DILUTION OF PRECISION  -- PDOP
		double dop = parseDouble(nmea.parameters[14]);
		this->fix.dilution = dop;

		// HORIZONTAL DILUTION OF PRECISION -- HDOP
		double hdop = parseDouble(nmea.parameters[15]);
		this->fix.horizontalDilution = hdop;

		// VERTICAL DILUTION OF PRECISION -- VDOP
		double vdop = parseDouble(nmea.parameters[16]);
		this->fix.verticalDilution = vdop;

		//calling handlers
		if (lockupdate){
			this->onLockStateChanged(this->fix.haslock);
		}
		this->onUpdate();
	}
	catch (NumberConversionError& ex)
	{
		NMEAParseError pe("GPS Number Bad Format [$GPGSA] :: " + ex.message, nmea);
		throw pe;
	}
	catch (NMEAParseError& ex)
	{
		NMEAParseError pe("GPS Data Bad Format [$GPGSA] :: " + ex.message, nmea);
		throw pe;
	}
}

void GPSService::read_GPGSV(const NMEASentence& nmea){
	/*  -- EXAMPLE --
	$GPGSV,2,1,08,01,40,083,46,02,17,308,41,12,07,344,39,14,22,228,45*75


	$GPGSV,3,1,12,01,00,000,,02,00,000,,03,00,000,,04,00,000,*7C
	$GPGSV,3,2,12,05,00,000,,06,00,000,,07,00,000,,08,00,000,*77
	$GPGSV,3,3,12,09,00,000,,10,00,000,,11,00,000,,12,00,000,*71

	Where:
	GSV          Satellites in view
	[0] 2            Number of sentences for full data
	[1] 1            sentence 1 of 2
	[2] 08           Number of satellites in view

	[3] 01           Satellite PRN number
	[4] 40           Elevation, degrees
	[5] 083          Azimuth, degrees
	[6] 46           SNR - higher is better
	[...]   for up to 4 satellites per sentence
	[17] *75          the checksum data, always begins with *
	*/

	try
	{
		if (!nmea.checksumOK()){
			throw NMEAParseError("Checksum is invalid!");
		}

		// can't do this check because the length varies depending on satallites...
		//if(nmea.parameters.size() < 18){
		//	throw NMEAParseError("GPS data is missing parameters.");
		//}

		// VISIBLE SATELLITES
		this->fix.visibleSatellites = (int32_t)parseInt(nmea.parameters[2]);
		if (this->fix.trackingSatellites == 0){
			this->fix.visibleSatellites = 0;			// if no satellites are tracking, then none are visible!
		}												// Also NMEA defaults to 12 visible when chip powers on. Obviously not right.

		uint32_t totalPages = (uint32_t)parseInt(nmea.parameters[0]);
		uint32_t currentPage = (uint32_t)parseInt(nmea.parameters[1]);


		//if this is the first page, then reset the almanac
		if (currentPage == 1){
			this->fix.almanac.clear();
			//cout << "CLEARING ALMANAC" << endl;
		}

		this->fix.almanac.lastPage = currentPage;
		this->fix.almanac.totalPages = totalPages;
		this->fix.almanac.visibleSize = this->fix.visibleSatellites;

		int entriesInPage = (nmea.parameters.size() - 3) >> 2;	//first 3 are not satellite info
		//- entries come in 4-ples, and truncate, so used shift
		GPSSatellite sat;
		for (int i = 0; i < entriesInPage; i++){
			int prop = 3 + i * 4;

			// PRN, ELEVATION, AZIMUTH, SNR
			sat.prn = (uint32_t)parseInt(nmea.parameters[prop]);
			sat.elevation = (uint32_t)parseInt(nmea.parameters[prop + 1]);
			sat.azimuth = (uint32_t)parseInt(nmea.parameters[prop + 2]);
			sat.snr = (uint32_t)parseInt(nmea.parameters[prop + 3]);

			//cout << "ADDING SATELLITE ::" << sat.toString() << endl;
			this->fix.almanac.updateSatellite(sat);
		}

		this->fix.almanac.processedPages++;

		// 
		if (this->fix.visibleSatellites == 0){
			this->fix.almanac.clear();
		}


		//cout << "ALMANAC FINISHED page " << this->fix.almanac.processedPages << " of " << this->fix.almanac.totalPages << endl;
		this->onUpdate();

	}
	catch (NumberConversionError& ex)
	{
		NMEAParseError pe("GPS Number Bad Format [$GPGSV] :: " + ex.message, nmea);
		throw pe;
	}
	catch (NMEAParseError& ex)
	{
		NMEAParseError pe("GPS Data Bad Format [$GPGSV] :: " + ex.message, nmea);
		throw pe;
	}
}

void GPSService::read_GPRMC(const NMEASentence& nmea){
	/*  -- EXAMPLE ---
	$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
	$GPRMC,235957.025,V,,,,,,,070810,,,N*4B
	$GPRMC,061425.000,A,3346.2243,N,08423.4706,W,0.45,18.77,060914,,,A*47

	Where:
	RMC          Recommended Minimum sentence C
	[0] 123519       Fix taken at 12:35:19 UTC
	[1] A            Status A=active or V=Void.
	[2-3] 4807.038,N   Latitude 48 deg 07.038' N
	[4-5] 01131.000,E  Longitude 11 deg 31.000' E
	[6] 022.4        Speed over the ground in knots
	[7] 084.4        Track angle in degrees True
	[8] 230394       Date - 23rd of March 1994
	[9-10] 003.1,W      Magnetic Variation
	[10] *6A          The checksum data, always begins with *
	// NMEA 2.3 includes another field after
	*/

	try
	{
		if (!nmea.checksumOK()){
			throw NMEAParseError("Checksum is invalid!");
		}

		if (nmea.parameters.size() < 11){
			throw NMEAParseError("GPS data is missing parameters.");
		}

		// TIMESTAMP
		this->fix.timestamp.setTime(parseDouble(nmea.parameters[0]));

		string sll;
		string dir;
		// LAT
		sll = nmea.parameters[2];
		dir = nmea.parameters[3];
		if (!sll.empty()){
			this->fix.latitude = convertLatLongToDeg(sll, dir);
		}

		// LONG
		sll = nmea.parameters[4];
		dir = nmea.parameters[5];
		if (!sll.empty()){
			this->fix.longitude = convertLatLongToDeg(sll, dir);
		}


		// ACTIVE
		bool lockupdate = false;
		char status = 'V';
		if (!nmea.parameters[1].empty()){
			status = nmea.parameters[1][0];
		}
		this->fix.status = status;
		if (status == 'V'){
			lockupdate = this->fix.setlock(false);
		}
		else if (status == 'A') {
			lockupdate = this->fix.setlock(true);
		}
		else {
			lockupdate = this->fix.setlock(false);		//not A or V, so must be wrong... no lock
		}


		this->fix.speed = convertKnotsToKilometersPerHour(parseDouble(nmea.parameters[6]));		// received as knots, convert to km/h
		this->fix.travelAngle = parseDouble(nmea.parameters[7]);
		this->fix.timestamp.setDate((int32_t)parseInt(nmea.parameters[8]));


		//calling handlers
		if (lockupdate){
			this->onLockStateChanged(this->fix.haslock);
		}
		this->onUpdate();
	}
	catch (NumberConversionError& ex)
	{
		NMEAParseError pe("GPS Number Bad Format [$GPRMC] :: " + ex.message, nmea);
		throw pe;
	}
	catch (NMEAParseError& ex)
	{
		NMEAParseError pe("GPS Data Bad Format [$GPRMC] :: " + ex.message, nmea);
		throw pe;
	}
}

void GPSService::read_GPVTG(const NMEASentence& nmea){
	/*
	$GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48

	where:
	VTG          Track made good and ground speed
	[0-1]	054.7,T      True track made good (degrees)
	[2-3]	034.4,M      Magnetic track made good
	[4-5]	005.5,N      Ground speed, knots
	[6-7]	010.2,K      Ground speed, Kilometers per hour
	[7]	*48          Checksum
	*/

	try
	{
		if (!nmea.checksumOK()){
			throw NMEAParseError("Checksum is invalid!");
		}

		if (nmea.parameters.size() < 8){
			throw NMEAParseError("GPS data is missing parameters.");
		}

		// SPEED
		// if empty, is converted to 0
		this->fix.speed = parseDouble(nmea.parameters[6]);		//km/h


		this->onUpdate();
	}
	catch (NumberConversionError& ex)
	{
		NMEAParseError pe("GPS Number Bad Format [$GPVTG] :: " + ex.message, nmea);
		throw pe;
	}
	catch (NMEAParseError& ex)
	{
		NMEAParseError pe("GPS Data Bad Format [$GPVTG] :: " + ex.message, nmea);
		throw pe;
	}
}

