//=============================================================================
// uuidtool 
//
//            Tool to create, print and manipulate OSF/DCE UUIDs
//
//            (C) 1998 Jim Doyle
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//=============================================================================

#include <stdio.h>
#include <getopt.h>
#include <string.h>

#include <stream.h>
#include <iostream.h>

#include "uuid.hpp"

using namespace std;

void mk_dce_idl_hdr();
void mk_c_initializer();
void test_ms_guid();
void test_dce_uuid();
void test_lt();
void test_gt();
void test_eq();

//===========================================================================
// usage():
//
//    Print information on using this tool.
//
//===========================================================================

void usage()
{
  printf(" uuid: Create,Print and Use OSF/DCE Universal Unique Identifiers\n");
  printf("           (C)1998 Jim Doyle, Boston University, <jrd@bu.edu>");
  printf("\n");
  printf("\n\t\t\t General Functions\n\n");
  printf(" -i\t\tProduce a DCE RPC IDL header\n");
  printf(" -o [filename] \tRedirect the output to a given file. \n");
  printf(" -s\t\tGenerates a UUID string as an initialized C structure\n");
  printf(" -n [num]\tGenerate 'num' new UUIDs\n");
  printf(" -h, -?\t\tPrint this help\n");
  printf("\n\t\t\t Comparison Functions\n\n");
  printf(" -e [uuid1],[uuid2]\tExit status of 0 if uuid1 == uuid2\n");
  printf(" -l [uuid1],[uuid2]\tExit status of 0 if uuid1 < uuid2\n");
  printf(" -g [uuid1],[uuid2]\tExit status of 0 if uuid1 > uuid2\n");
  printf(" -t [uuid]\t\tExit status of 0 if UUID is valid and not NIL UUID\n");
  printf(" -d [uuid]\t\tExit status of 0 if given UUID is an OSF/DCE UUID\n");
  printf(" -m [uuid]\t\tExit status of 0 if given UUID is a Microsoft GUID\n");
  printf("\n");
  printf(" default action: create and print one new UUID.\n");
  printf("\n");
  printf(" Note:  uuid1 > uuid2 if uuid1 is later in time than uuid2\n");
  printf("        uuid1 < uuid2 if uuid2 was create after uuid1 \n");
  printf("\n");

  exit(1);

}

//
// Globals and what not
//

static int num_uuids = 1;
char * outfile_name = NULL;
char * uuid_args;
uuid uuid1(nil_uuid);
uuid uuid2(nil_uuid);
bool opt_idl_hdrgen, opt_c_initializer;
bool opt_equals, opt_lessthan, opt_greaterthan, opt_test;
bool opt_testdceuuid, opt_testmsguid;

//===========================================================================
// main()
//===========================================================================

main(int argc, char * argv[])
{

  extern char * optarg;
  extern int optind, opterr, optopt;
  int j;

  //
  // Pick apart the command line args
  //

  while ((j = getopt(argc, argv, "vish?o:n:e:l:g:t:d:m:")) != EOF)
    {
      uuid_args = 0;
      switch(j)
	{
	case 'i':
	  opt_idl_hdrgen = true;
	  break;
	case 's':
	  opt_c_initializer = true;
	  break;
	case 'h':
	case '?':
	case 'v':
	  usage();
	  break;
	case 'o':
	  outfile_name = optarg;
	  break;
	case 'n':
	  num_uuids = atoi(optarg);
	  break;
	case 'e':
	  opt_equals = true;
	  uuid_args = optarg;
	  break;
	case 'l':
	  opt_lessthan = true;
	  uuid_args = optarg;
	  break;
	case 'g':
	  opt_greaterthan = true;
	  uuid_args = optarg;
	  break;
	case 't':
	  opt_test = true;
	  uuid_args = optarg;
	  break;
	case 'd':
	  opt_testdceuuid = true;
	  uuid_args = optarg;
	  break;
	case 'm':
	  opt_testmsguid = true;
	  uuid_args = optarg;
	  break;
	default:
	  usage();
	}
    }



  //
  // For options that take a single UUID, go get it
  //

  if (opt_test || opt_testmsguid || opt_testdceuuid)
    {
      uuid1 = uuid(uuid_args);
      if (uuid1.IsNil())
	{
	  cout << "Error: argument must be a valid UUID" << endl;
	  exit(1);
	}
    }

  //
  // For options that take a pair of UUID's, split them up now, commas, slashes
  // and colons are all legitimate separators
  //

  if (opt_equals || opt_lessthan || opt_greaterthan)
    {
      if (uuid_args)
	{
	  char * token;
	  
	  token = strtok(uuid_args,",:/");
	  if (token) uuid1 = uuid(token);
	  token = strtok(NULL, ",:/");
	  if (token) uuid2 = uuid(token);

	  if (uuid1.IsNil() || uuid2.IsNil())
	    {
	      cout << "Error: one or both of the UUID arguments is invalid" << endl;
	      exit(1);
	    }
	}
      else
	{
	  cout << "Error: need UUID arguments " << endl;
	  exit(1);
	}
    }

    

  //
  // Dispatch the really easy functions
  //

  if (opt_idl_hdrgen)    mk_dce_idl_hdr();
  if (opt_c_initializer) mk_c_initializer();
  if (opt_testmsguid) test_ms_guid();
  if (opt_testdceuuid) test_dce_uuid();
  if (opt_lessthan) test_lt();
  if (opt_greaterthan) test_gt();
  if (opt_equals) test_eq();
  if (opt_test) test_dce_uuid();

  //
  // If no options, then generate N uuids
  //

  int i;
  for (i=0; i<num_uuids; i++) { cout << uuid().String() << endl; }
  exit(0);

}


//
// mk_dce_idl_hdr():
//      Generate a header for a DCE RPC IDL interface file.
//
void
mk_dce_idl_hdr()
{
  uuid new_uuid;

  cout << "[ uuid(" << new_uuid.String() << "), version(1.0) ]" << endl;
  cout << "interface INTERFACENAME" << endl;
  cout << "{ " << endl << " /* My interface definition goes here */ " << endl << "}" << endl;
  exit(0);
}

//
// mk_c_initializer()
//      Generate an C-language initializer for a UUID.
//

void
mk_c_initializer()
{
  uuid new_uuid;
  uuid_t intl;
  int j; int v;

  intl = new_uuid.C_uuid();
  
  cout << "uuid_t foo = { /* uuid (" << new_uuid.String() << ") */" << endl;
  cout << "  " << "0x" << hex << intl.time_low << "," << "0x" << hex << intl.time_mid << "," << 
    "0x" << hex << intl.time_hi_and_version << ",";
  cout << "0x" << hex << (int)intl.clock_seq_hi_and_reserved << "," 
       << "0x" << hex << (int)intl.clock_seq_low << ",";
  
  //
  // print the 6 byte node id 
  //

  cout << " { ";
  for (j=0; j<6; j++)
    {
      v = 0;
      v = intl.node[j];
      cout << "0x" << hex << v;
      if (j < 5) cout << ", ";
    }
  cout << " }";
  cout << "};" << endl;

  exit(0);
}

//
// Is a Microsoft GUID?
//

void
test_ms_guid()
{
  cout << "Error: MS GUID test presently not implemented. " << endl;
  exit(1);
}

//
// Is a DCE UUID?
//

void
test_dce_uuid()
{
  if (uuid1.IsNil())
    {
      cout << "is not a valid uuid." << endl;
      exit(1);
    }
  else
    {
      exit(0);
    }
}

//
// Less Than comparator
//

void
test_lt()
{
  if (uuid1 < uuid2)
    {
      cout << "is less than." << endl;
      exit(0);
    }
  else
    {
      exit(1);
    }
}

//
// Greater than comparator
//

void
test_gt()
{

  if (uuid1 > uuid2)
    {
      cout << "is greater than." << endl;
      exit(0);
    }
  else
    {
      exit(1);
    }

}

//
// Equality comparator
//

void
test_eq()
{

  if (uuid1 == uuid2)
    {
      cout << "is equal to." << endl;
      exit(0);
    }
  else
    {
      exit(1);
    }

}

