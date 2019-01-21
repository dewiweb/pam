#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{

	//Date Version Types
	static const char DATE[] = "21";
	static const char MONTH[] = "01";
	static const char YEAR[] = "2019";
	static const char UBUNTU_VERSION_STYLE[] =  "19.01";

	//Software Status
	static const char STATUS[] =  "Release";
	static const char STATUS_SHORT[] =  "r";

	//Standard Version Type
	static const long MAJOR  = 1;
	static const long MINOR  = 0;
	static const long BUILD  = 893;
	static const long REVISION  = 5002;

	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 2583;
	#define RC_FILEVERSION 1,0,893,5002
	#define RC_FILEVERSION_STRING "1, 0, 893, 5002\0"
	static const char FULLVERSION_STRING [] = "1.0.893.5002";


	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 0;


}
#endif //VERSION_H
