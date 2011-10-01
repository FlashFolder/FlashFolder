
#pragma once

#ifdef DEBUG
	#ifdef WIN64
		#pragma comment( lib, "commonUtils64_d.lib" )
	#else
		#pragma comment( lib, "commonUtils_d.lib" )
	#endif
#else
	#ifdef WIN64
		#pragma comment( lib, "commonUtils64.lib" )
	#else
		#pragma comment( lib, "commonUtils.lib" )
	#endif
#endif