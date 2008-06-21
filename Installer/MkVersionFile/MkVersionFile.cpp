/* This file is part of FlashFolder. 
 * Copyright (C) 2007 zett42 ( zett42 at users.sourceforge.net ) 
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/**
 * \file Tool to extract version numbers from a header file and use them to replace placeholders in another file.
**/

#include <iostream>
#include <fstream>
#include <string>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <sys/types.h> 
#include <sys/stat.h>

using namespace std;
using namespace boost;

//--------------------------------------------------------------------------------------------

void LoadFile( std::string* ps, std::istream& is)
{
   ps->erase();
   ps->reserve( is.rdbuf()->in_avail() );
   char c;
   while( is.get( c ) )
   {
      if( ps->capacity() == ps->size() )
         ps->reserve( ps->capacity() * 3 );
      ps->append( 1, c );
   }
}

//--------------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	if( argc != 4 )
	{
		printf( "[MkVersionFile] not enough arguments\n" );
		printf( "[MkVersionFile] Syntax: mkversionfile <VERSIONFILE> <TEMPLATEFILE> <OUTPUTFILE>\n");
		return 1;
	} 

	char* inpath = argv[1];
	char* templpath = argv[2];
	char* outpath = argv[3];

	ifstream infile( inpath, ios_base::binary );
	if( ! infile )
	{
		printf( "[MkVersionFile] ERROR: could not open input file %s", inpath );
		return 2;		
	}
	ifstream templfile( templpath, ios_base::binary );
	if( ! templfile )
	{
		printf( "[MkVersionFile] ERROR: could not open template file %s", templpath );
		return 3;		
	}

	string instr, outstr;
	LoadFile( &instr, infile );
	LoadFile( &outstr, templfile );
	
	infile.close();
	templfile.close();

	// extract version numbers from a #define, example input: 
    // #define APP_VER_MAJOR 2
	regex expr_major( "#define\\s+APP_VER_MAJOR\\s+(\\d+)" );
	regex expr_minor( "#define\\s+APP_VER_MINOR\\s+(\\d+)" );
	regex expr_micro( "#define\\s+APP_VER_MICRO\\s+(\\d+)" );
	regex expr_build( "#define\\s+APP_VER_BUILD\\s+(\\d+)" );
		
	match_results<const char*> match;
	string v_major, v_minor, v_micro, v_build;

	if( ! regex_search( instr.c_str(), match, expr_major ) )
	{
		printf( "[MkVersionFile] ERROR: input file has unexpected format.\n" );
		return 4;
	}
	v_major = match[1].str();

	if( ! regex_search( instr.c_str(), match, expr_minor ) )
	{
		printf( "[MkVersionFile] ERROR: input file has unexpected format.\n" );
		return 5;
	}
	v_minor = match[1].str();

	if( ! regex_search( instr.c_str(), match, expr_micro ) )
	{
		printf( "[MkVersionFile] ERROR: input file has unexpected format.\n" );
		return 6;
	}
	v_micro = match[1].str();

	if( ! regex_search( instr.c_str(), match, expr_build ) )
	{
		printf( "[MkVersionFile] ERROR: input file has unexpected format.\n" );
		return 7;
	}
	v_build = match[1].str();

	replace_all( outstr, "$APP_VER_MAJOR$", v_major );
	replace_all( outstr, "$APP_VER_MINOR$", v_minor );
	replace_all( outstr, "$APP_VER_MICRO$", v_micro );
	replace_all( outstr, "$APP_VER_BUILD$", v_build );

	ofstream outfile( outpath, ios_base::binary );
	if( ! outfile )
	{
		printf( "[MkVersionFile] could not open output file %s\n", outpath );
		return 8;		
	}
	outfile.write( outstr.c_str(), outstr.size() ); 

	printf( "[MkVersionFile] Version file created successfully: %s\n", outpath );

	return 0;
}

