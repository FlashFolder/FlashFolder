
#pragma once

#ifdef DEBUG
	#ifdef WIN64
		#pragma comment( lib, "common64_d.lib" )
	#else
		#pragma comment( lib, "common_d.lib" )
	#endif
#else
	#ifdef WIN64
		#pragma comment( lib, "common64.lib" )
	#else
		#pragma comment( lib, "common.lib" )
	#endif
#endif