/*
 * NumberConversion.cpp
 *
 *  Created on: Sep 2, 2014
 *      Author: Cameron Karlsson
 *
 *  See the license file included with this source.
 */

#include <nmeaparse/NumberConversion.h>
#include <cstdlib>

using namespace std;

namespace nmea {
// Note: both parseDouble and parseInt return 0 with "" input.

		double parseDouble(std::string s){

			char* p;
			double d = ::strtod(s.c_str(), &p);
			if (*p != 0){
				std::stringstream ss;
				ss << "NumberConversionError: parseDouble() error in argument \"" << s << "\", '"
					<< *p << "' is not a number.";
				throw NumberConversionError(ss.str());
			}
			return d;
		}
		int64_t parseInt(std::string s, int radix){
			char* p;

			int64_t d = ::strtoll(s.c_str(), &p, radix);

			if (*p != 0) {
				std::stringstream ss;
				ss << "NumberConversionError: parseInt() error in argument \"" << s << "\", '"
					<< *p << "' is not a number.";
				throw NumberConversionError(ss.str());
			}
			return d;
		}

}

/*
#include <iostream>
void NumberConversion_test(){
 	string s;
 	float f;
 	long long k;

	try{
		s = "-1.345";
		f = parseDouble(s);
		cout << s << ": " << f << endl;
	}
	catch(NumberConversionError& ex){
		cout << ex.message << endl;
	}


	try{
		s = "-1.23e-2";
		f = parseDouble(s);
		cout << s << ": " << f << endl;
	}
	catch(NumberConversionError& ex){
		cout << ex.message << endl;
	}


	try{
		s = "";
		f = parseDouble(s);
		cout << s << ": " << f << endl;
	}
	catch(NumberConversionError& ex){
		cout << ex.message << endl;
	}



	try{
		// -- fails, ok
		s = "asd";
		f = parseDouble(s);
		cout << s << ": " << f << endl;
	}
	catch(NumberConversionError& ex){
		cout << ex.message << endl;
	}

	try{
		s = "-1234.123";
		k = parseInt(s);
		cout << s << ": " << k << endl;
	}
	catch(NumberConversionError& ex){
		cout << ex.message << endl;
	}



	try{
		s = "01234";
		k = parseInt(s);
		cout << s << ": " << k << endl;
	}
	catch(NumberConversionError& ex){
		cout << ex.message << endl;
	}


	try{
		// -- converts to 0
		s = "";
		k = parseInt(s);
		cout << s << ": " << k << endl;
	}
	catch(NumberConversionError& ex){
		cout << ex.message << endl;
	}


	try{
		// -- fails, ok
		s = "asd";
		k = parseInt(s);
		cout << s << ": " << k << endl;
	}
	catch(NumberConversionError& ex){
		cout << ex.message << endl;
	}

	try{
		// -- fails, ok
		s = "-16";
		k = parseInt(s);
		cout << s << ": " << k << endl;
	}
	catch(NumberConversionError& ex){
		cout << ex.message << endl;
	}

 }
 */