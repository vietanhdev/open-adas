/*
 *
 *  See the license file included with this source.
 */

#include <iostream>
#include <fstream>
#include <nmeaparse/nmea.h>





using namespace std;
using namespace nmea;

int main(int argc, char** argv){



	// --------------------------------------------------------
	// ------------  CONFIGURE GPS SERVICE  -------------------
	// --------------------------------------------------------
	
	// Create a GPS service that will keep track of the fix data.
	NMEAParser parser;
	GPSService gps(parser);
	//parser.log = true;		// true: will spit out all sorts of parse info on each sentence.

	// Handle events when the lock state changes
	gps.onLockStateChanged += [](bool newlock){
		if (newlock){
			cout << "\t\t\tGPS aquired LOCK!" << endl;
		}
		else {
			cout << "\t\t\tGPS lost lock. Searching..." << endl;
		}
	};

	// Handle any changes to the GPS Fix... This is called after onSentence
	gps.onUpdate += [&gps](){
		cout << "\t\t\tPosition: " << gps.fix.latitude << "'N, " << gps.fix.longitude << "'E" << endl << endl;
	};

	// (optional) - Handle events when the parser receives each sentence
	parser.onSentence += [&gps](const NMEASentence& n){
		cout << "Received " << (n.checksumOK() ? "good" : "bad") << " GPS Data: " << n.name << endl;
	};


	cout << "-------- Reading GPS NMEA data --------" << endl;

	// --------------------------------------------------------
	// ---------------   STREAM THE DATA  ---------------------
	// --------------------------------------------------------
	try {
		// From a buffer in memory...
		//   cout << ">> [ From Buffer]" << endl;
		//   parser.readBuffer((uint8_t*)bytestream, sizeof(bytestream));
		// ---------------------------------------

		// -- OR --
		// From a device byte stream...
		//   cout << ">> [ From Device Stream]" << endl;
		//   parser.readByte(byte_from_device);
		// ---------------------------------------

		// -- OR --
		// From a text log file...
		cout << ">> [ From File]" << endl;
		string line;
		ifstream file("nmea_log.txt");
		while ( getline(file, line) ){
			try {
				parser.readLine(line);
			}			
			catch (NMEAParseError& e){
				cout << e.message << endl << endl;
				// You can keep feeding data to the gps service...
				// The previous data is ignored and the parser is reset.
			}
		}
	}
	catch (exception& e){
		// Notify the proper authorities. Something is on fire.
		cout << "Something Broke: " << e.what() << endl;
	}
	// ---------------------------------------


	// Show the final fix information
	//cout << gps.fix.toString() << endl;





	// --------------------------------------------------------
	// ---------------   NMEA ALTERNATIVE SENTENCES  ----------
	// --------------------------------------------------------
	// Not using GPS NMEA Sentences? That's A-OK.
	// While there is no data aggregation code written here for 
	// non-GPS use, the parser will still make your job easier.
	// Extract only the sentences you care about.

	// Create our custom parser...
	NMEAParser custom_parser;
	//parser.log = true;
	custom_parser.setSentenceHandler("MYNMEA", [](const NMEASentence& n){
		cout << "Handling $" << n.name << ":" << endl;
		for (size_t i = 0; i < n.parameters.size(); ++i){
			cout << "    [" << i << "] \t- " << n.parameters[i];
			try {
				double num = parseDouble(n.parameters[i]);
				cout << "      (number: " << num << ")";
			} catch (NumberConversionError&){
				cout << " (string)";
			}
			cout << endl;
		}
	});
	custom_parser.onSentence += [](const NMEASentence& n){
		cout << "Received $" << n.name << endl;
	};

	cout << "-------- Reading non-GPS NMEA data --------" << endl;

	// Read the data stream...
	// These don't have correct checksums. They're made up.
	char data[] = "  $MYNMEA,1,3,3,7,Hello*A2\n		\
					$IRRELEVANT,5,5,5*AA\n			\
					$ERRORS,:D,\n					\
					$\n								\
					$$\n							\
					$*\n							\
					$*,\n							\
					$,\n							\
					$,*\n							\
					garbage that will be			\
					!IgN0r3d @)(&%!!!				\
					$MYNMEA,1,3,3,7,World!*A2\r\n	\
					";
	for (int i = 0; i < sizeof(data); i++){
		try {
			custom_parser.readByte(data[i]);
		}
		catch (NMEAParseError& e){
			cout << e.what() << endl;
		}
	}










	// --------------------------------------------------------
	// ---------------   NMEA SENTENCE GENERATION  ------------
	// --------------------------------------------------------
	// Some devices allow control sentences to be sent to them.
	// For some GPS devices this can allow selecting certain data.
	// Only the following 2 Sentences are implemented, however
	// you can create your own from the NMEACommand base class.


	// Test the parser and command generation
	NMEACommand							cmd1;	// A blank generic command
	NMEACommandQueryRate				cmd2;	// The $PSRF command that allows for GPS sentence selection and rate setting.
	NMEACommandQueryRate				cmd3;	// The $PSRF command that allows for GPS sentence selection and rate setting.
	NMEACommandSerialConfiguration		cmd4;	// The $PSRF command that can configure a UART baud rate.
	NMEAParser test_parser;
	test_parser.onSentence += [&cmd1, &cmd2, &cmd3, &cmd4](const NMEASentence& n){
		cout << "Received:  " << n.text;
		
		if (!n.checksumOK()){
			cout << "\t\tChecksum FAIL!" << endl;
		}
		else {
			cout << "\t\tChecksum PASS!" << endl;
		}
	};

	cout << "-------- NMEA Command Generation --------" << endl;

	// Just filling it with something. Could be whatever you need.
	cmd1.name		= "CMD1";
	cmd1.message	= "nothing,special";

	// Config output rate for $GPGGA sentence
	cmd2.messageID	= NMEASentence::MessageID::GGA;
	cmd2.mode		= NMEACommandQueryRate::QueryRateMode::SETRATE;
	cmd2.rate		= 3;		// output every 3 seconds, 0 to disable

	// Query $GPGSV almanac sentence just this once
	cmd3.messageID	= NMEASentence::MessageID::GSV;
	cmd3.mode		= NMEACommandQueryRate::QueryRateMode::QUERY;

	// Set the Baud rate to 9600, because this GPS chip is awesome
	cmd4.baud		= 9600;		// 4800 is NMEA standard

	// Generate the NMEA sentence from the commands and send them back into the test parser.
	test_parser.readSentence(cmd1.toString());
	test_parser.readSentence(cmd2.toString());
	test_parser.readSentence(cmd3.toString());
	test_parser.readSentence(cmd4.toString());



	cout << endl;
	cout << endl;
	cout << "-------- ALL DONE --------" << endl;



	cin.ignore();

	return 0;

}