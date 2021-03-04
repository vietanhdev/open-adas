/*
 * NMEAParser.cpp
 *
 *  Created on: Aug 12, 2014
 *      Author: Cameron Karlsson
 *
 *  See the license file included with this source.
 */

#include <nmeaparse/NMEAParser.h>
#include <nmeaparse/NumberConversion.h>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

using namespace std;
using namespace nmea;



// --------- NMEA PARSE ERROR--------------

NMEAParseError::NMEAParseError(std::string msg)
	: message(msg)
{}
NMEAParseError::NMEAParseError(std::string msg, NMEASentence n)
	: message(msg), nmea(n)
{}

NMEAParseError::~NMEAParseError()
{}

std::string NMEAParseError::what(){
	return message;
}




// --------- NMEA SENTENCE --------------

NMEASentence::NMEASentence() 
: isvalid(false)
, checksumIsCalculated(false)
, calculatedChecksum(0)
, parsedChecksum(0)
{ }

NMEASentence::~NMEASentence()
{ }

bool NMEASentence::valid() const {
	return isvalid;
}

bool NMEASentence::checksumOK() const {
	return (checksumIsCalculated)
		&&
		(parsedChecksum == calculatedChecksum);
}



// true if the text contains a non-alpha numeric value
bool hasNonAlphaNum(string txt){
	for (const char i : txt){
		if ( !isalnum(i) ){
			return true;
		}
	}
	return false;
}

// true if alphanumeric or '-'
bool validParamChars(string txt){
	for (const char i : txt){
		if (!isalnum(i)){
			if (i != '-' && i != '.'){
				return false;
			}
		}
	}
	return true;
}

// remove all whitespace
void squish(string& str){

	char chars[] = {'\t',' '};
	for (const char i : chars)
	{
		// needs include <algorithm>
		str.erase(std::remove(str.begin(), str.end(), i), str.end());
	}
}

// remove side whitespace
void trim(string& str){
	stringstream trimmer;
	trimmer << str;
	str.clear();
	trimmer >> str;
}


// --------- NMEA PARSER --------------



NMEAParser::NMEAParser() 
: log(false)
, maxbuffersize(NMEA_PARSER_MAX_BUFFER_SIZE)
, fillingbuffer(false)
{ }

NMEAParser::~NMEAParser() 
{ }


void NMEAParser::setSentenceHandler(std::string cmdKey, std::function<void(const NMEASentence&)> handler){
	eventTable.erase(cmdKey);
	eventTable.insert({ cmdKey, handler });
}
string NMEAParser::getRegisteredSentenceHandlersCSV()
{
	if(eventTable.empty()){
		return "";
	}

	ostringstream ss;
	for(const auto& table : eventTable){
		ss << table.first;

		if( ! table.second ){
			ss << "(not callable)";
		}
		ss << ",";
	}
	string s = ss.str();
	if( ! s.empty() ){
		s.resize(s.size()-1); // chop off comma
	}
	return s;
}

void NMEAParser::readByte(uint8_t b){
	uint8_t startbyte = '$';

	if (fillingbuffer){
		if (b == '\n'){
			buffer.push_back(b);
			try {
				readSentence(buffer);
				buffer.clear();
				fillingbuffer = false;
			}
			catch (exception&){
				// If anything happens, let it pass through, but reset the buffer first.
				buffer.clear();
				fillingbuffer = false;
				throw;
			}
		}
		else{
			if (buffer.size() < maxbuffersize){
				buffer.push_back(b);
			}
			else {
				buffer.clear();			//clear the host buffer so it won't overflow.
				fillingbuffer = false;
			}
		}
	}
	else {
		if (b == startbyte){			// only start filling when we see the start byte.
			fillingbuffer = true;
			buffer.push_back(b);
		}
	}
}

void NMEAParser::readBuffer(uint8_t* b, uint32_t size){
	for (uint32_t i = 0; i < size; ++i){
		readByte(b[i]);
	}
}

void NMEAParser::readLine(string cmd){
	cmd += "\r\n";
	for (const char i : cmd){
		readByte(i);
	}
}

// Loggers
void NMEAParser::onInfo(NMEASentence& nmea, string txt){
	if (log){
		cout << "[Info]    " << txt << endl;
	}
}
void NMEAParser::onWarning(NMEASentence& nmea, string txt){
	if (log){
		cout << "[Warning] " << txt << endl;
	}
}
void NMEAParser::onError(NMEASentence& nmea, string txt){
	throw NMEAParseError("[ERROR] " + txt);
}

// takes a complete NMEA string and gets the data bits from it,
// calls the corresponding handler in eventTable, based on the 5 letter sentence code
void NMEAParser::readSentence(std::string cmd){

	NMEASentence nmea;

	onInfo(nmea, "Processing NEW string...");
	
	if (cmd.empty()){
		onWarning(nmea, "Blank string -- Skipped processing.");
		return;
	}
	
	// If there is a newline at the end (we are coming from the byte reader
	if ( *(cmd.end()-1) == '\n'){
		if (*(cmd.end() - 2) == '\r'){	// if there is a \r before the newline, remove it.
			cmd = cmd.substr(0, cmd.size() - 2);
		}
		else
		{
			onWarning(nmea, "Malformed newline, missing carriage return (\\r) ");
			cmd = cmd.substr(0, cmd.size()-1);
		}
	}

	ios_base::fmtflags oldflags = cout.flags();

	// Remove all whitespace characters.
	size_t beginsize = cmd.size();
	squish(cmd);
	if (cmd.size() != beginsize){
		stringstream ss;
		ss << "New NMEA string was full of " << (beginsize - cmd.size()) << " whitespaces!";
		onWarning(nmea, ss.str());
	}

	
	onInfo(nmea, string("NMEA string: (\"") + cmd + "\")");
	

	// Seperates the data now that everything is formatted
	try{
		parseText(nmea, cmd);
	}
	catch (NMEAParseError&){
		throw;
	}
	catch (std::exception& e){
		string s = " >> NMEA Parser Internal Error: Indexing error?... ";
		throw std::runtime_error(s + e.what());
	}
	cout.flags(oldflags);  //reset

	// Handle/Throw parse errors
	if (!nmea.valid()){

		size_t linewidth = 35;
		stringstream ss;
		if (nmea.text.size() > linewidth){
			ss << "Invalid text. (\"" << nmea.text.substr(0, linewidth) << "...\")";
		}
		else{
			ss << "Invalid text. (\"" << nmea.text << "\")";
		}

		onError(nmea, ss.str());
		return;
	}
	

	// Call the "any sentence" event handler, even if invalid checksum, for possible logging elsewhere.
	onInfo(nmea, "Calling generic onSentence().");
	onSentence(nmea);


	// Call event handlers based on map entries
	function<void(const NMEASentence&)> handler = eventTable[nmea.name];
	if (handler){
		onInfo(nmea, string("Calling specific handler for sentence named \"") + nmea.name + "\"");
		handler(nmea);
	}
	else
	{
		onWarning(nmea, string("Null event handler for type (name: \"") + nmea.name + "\")");
	}





	cout.flags(oldflags);  //reset

}

// takes the string *between* the '$' and '*' in nmea sentence,
// then calculates a rolling XOR on the bytes
uint8_t NMEAParser::calculateChecksum(string s){
	uint8_t checksum = 0;
	for (const char i : s){
		checksum = checksum ^ i;
	}

	// will display the calculated checksum in hex
	//if(log)
	//{
	//	ios_base::fmtflags oldflags = cout.flags();
	//	cout << "NMEA parser Info: calculated CHECKSUM for \""  << s << "\": 0x" << std::hex << (int)checksum << endl;
	//	cout.flags(oldflags);  //reset
	//}
	return checksum;
}


void NMEAParser::parseText(NMEASentence& nmea, string txt){

	if (txt.empty()){
		nmea.isvalid = false;
		return;
	}

	nmea.isvalid = false;	// assume it's invalid first
	nmea.text = txt;		// save the received text of the sentence

	// Looking for index of last '$'
	size_t startbyte = 0;
	size_t dollar = txt.find_last_of('$');
	if (dollar == string::npos){
		// No dollar sign... INVALID!
		return;
	}
	else
	{
		startbyte = dollar;
	}


	// Get rid of data up to last'$'
	txt = txt.substr(startbyte + 1);


	// Look for checksum
	size_t checkstri = txt.find_last_of('*');
	bool haschecksum = checkstri != string::npos;
	if (haschecksum){
		// A checksum was passed in the message, so calculate what we expect to see
		nmea.calculatedChecksum = calculateChecksum(txt.substr(0, checkstri));
	}
	else
	{
		// No checksum is only a warning because some devices allow sending data with no checksum.
		onWarning(nmea, "No checksum information provided. Could not find '*'.");
	}

	// Handle comma edge cases
	size_t comma = txt.find(',');
	if (comma == string::npos){		//comma not found, but there is a name...
		if (!txt.empty())
		{	// the received data must just be the name
			if ( hasNonAlphaNum(txt) ){
				nmea.isvalid = false;
				return;
			}
			nmea.name = txt;
			nmea.isvalid = true;
			return;
		}
		else
		{	//it is a '$' with no information
			nmea.isvalid = false;
			return;	
		}
	}

	//"$," case - no name
	if (comma == 0){
		nmea.isvalid = false;
		return;
	}


	//name should not include first comma
	nmea.name = txt.substr(0, comma);	
	if ( hasNonAlphaNum(nmea.name) ){
		nmea.isvalid = false;
		return;
	}


	//comma is the last character/only comma
	if (comma + 1 == txt.size()){		
		nmea.parameters.push_back("");
		nmea.isvalid = true;
		return;	
	}


	//move to data after first comma
	txt = txt.substr(comma + 1, txt.size() - (comma + 1));

	//parse parameters according to csv
	istringstream f(txt);
	string s;
	while (getline(f, s, ',')) {
		//cout << s << endl;
		nmea.parameters.push_back(s);
	}


	//above line parsing does not add a blank parameter if there is a comma at the end...
	// so do it here.
	if (*(txt.end() - 1) == ','){

		// supposed to have checksum but there is a comma at the end... invalid
		if (haschecksum){
			nmea.isvalid = false;
			return;
		}

		//cout << "NMEA parser Warning: extra comma at end of sentence, but no information...?" << endl;		// it's actually standard, if checksum is disabled
		nmea.parameters.push_back("");

		stringstream sz;
		sz << "Found " << nmea.parameters.size() << " parameters.";
		onInfo(nmea, sz.str());

	}
	else
	{
		stringstream sz;
		sz << "Found " << nmea.parameters.size() << " parameters.";
		onInfo(nmea, sz.str());

		//possible checksum at end...
		size_t endi = nmea.parameters.size() - 1;
		size_t checki = nmea.parameters[endi].find_last_of('*');
		if (checki != string::npos){
			string last = nmea.parameters[endi];
			nmea.parameters[endi] = last.substr(0, checki);
			if (checki == last.size() - 1){
				onError(nmea, "Checksum '*' character at end, but no data.");
			}
			else{
				nmea.checksum = last.substr(checki + 1, last.size() - checki);		//extract checksum without '*'

				onInfo(nmea, string("Found checksum. (\"*") + nmea.checksum + "\")");

				try
				{
					nmea.parsedChecksum = (uint8_t)parseInt(nmea.checksum, 16);
					nmea.checksumIsCalculated = true;
				}
				catch( NumberConversionError& )
				{
					onError(nmea, string("parseInt() error. Parsed checksum string was not readable as hex. (\"") +  nmea.checksum + "\")");
				}
				
				onInfo(nmea, string("Checksum ok? ") + (nmea.checksumOK() ? "YES" : "NO") + "!");
				

			}
		}
	}


	for (size_t i = 0; i < nmea.parameters.size(); i++){
		if (!validParamChars(nmea.parameters[i])){
			nmea.isvalid = false;
			stringstream ss;
			ss << "Invalid character (non-alpha-num) in parameter " << i << " (from 0): \"" << nmea.parameters[i] << "\"";
			onError(nmea, ss.str() );
			break;
		}
	}


	nmea.isvalid = true;

	return;

}


