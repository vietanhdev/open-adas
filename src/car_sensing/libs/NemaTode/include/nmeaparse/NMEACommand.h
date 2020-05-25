/*
 * NMEACommand.h
 *
 *  Created on: Sep 8, 2014
 *      Author: Cameron Karlsson
 *
 *  See the license file included with this source.
 */

#ifndef NMEACOMMAND_H_
#define NMEACOMMAND_H_

#include <string>
#include <nmeaparse/NMEAParser.h>

namespace nmea {

		class NMEACommand {
		public:
			std::string message;
			std::string name;
			char checksum;
			NMEACommand();
			virtual ~NMEACommand();
			virtual std::string toString();
			std::string addChecksum(std::string s);
		};



		class NMEACommandSerialConfiguration : public NMEACommand {
		public:
			/*
			// $PSRF100,0,9600,8,1,0*0C

			Table 2-4 Set Serial Port Data Format
			Name		Example		Unit Description
			Message ID	$PSRF100	PSRF100 protocol header
			Protocol	0 			0=SiRF binary, 1=NMEA
			Baud 		9600 		1200, 2400, 4800, 9600, 19200, 38400, 57600, and 115200
			DataBits	8 			8,71

			StopBits	1 			0,1		1. SiRF protocol is only valid for 8 data bits, 1stop bit, and no parity.
			Parity		0 			0=None, 1=Odd, 2=Even
			Checksum	*0C
			<CR> <LF> End of message termination
			*/
			int32_t baud;       //4800, 9600, 19200, 38400
			int32_t databits;	//7, 8 Databits
			int32_t stopbits;	//0, 1 Stopbits
			int32_t parity;		//0=none, 1=odd, 2=even Parity

			NMEACommandSerialConfiguration(){
				name = "PSRF100";
				baud = 4800;
				databits = 8;
				stopbits = 1;
				parity = 0;
			};
			virtual std::string toString();
		};

		class NMEACommandQueryRate : public NMEACommand {
		public:
			// data fields that will be stringed.


			//  $PSRF103,00,01,00,01*25
			/*
			* Table 2-9 Query/Rate Control Data Format
			Name		Example		Unit Description
			Message ID	$PSRF103	PSRF103 protocol header
			Msg			00 			See Table 2-10
			Mode		01 			0=SetRate, 1=Query
			Rate		00 			sec Output—off=0, max=255
			CksumEnable 01 			0=Disable Checksum, 1=Enable Checksum
			Checksum	*25
			<CR> <LF> End of message termination
			*/
			/*
			* Table 2-10 Messages
			Value Description
			0 GGA
			1 GLL
			2 GSA
			3 GSV
			4 RMC
			5 VTG
			6 MSS (If internal beacon is supported)
			7 Not defined
			8 ZDA (if 1PPS output is supported)
			9 Not defined
			*/

			enum QueryRateMode {
				SETRATE = 0,
				QUERY = 1
			};

			NMEASentence::MessageID messageID;
			QueryRateMode mode;
			int rate;
			int checksumEnable;
			NMEACommandQueryRate(){
				name = "PSRF103";
				messageID = NMEASentence::Unknown;
				mode = QueryRateMode::SETRATE;
				rate = 0;
				checksumEnable = 1;
			};
			virtual std::string toString();
		};


}

#endif /* NMEACOMMAND_H_ */
