
#ifndef _UUID_CXX_X_
#define _UUID_CXX_X_

#if defined(UUID_BUILD_STANDALONE)
#include "uuid.h"
#else
#include <dce/uuid.h>
#endif

#include <string>
#include <iostream>

//=============================================================================
//
// UUID C++ Class
//
// Jim Doyle, Boston University, September 1998, <jrd@bu.edu>
//
// Generalized object interface for DCE Unique Universal Identifiers. 
// This interface offers C++ operator-overloaded methods, as well as
// the alternative Java style interface to operators. This will allow
// the C++ class to be easily proxied to other languages such
// as Java (JNI), Perl, Tcl and Python.
//
//=============================================================================
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


class uuid {
  
private:

  uuid_t i_uuid;
  char * s_uuid;
  unsigned32 e;

  void make_nil();

public:

  //
  // Constructors can take from a C uuid_t or a uuid string
  //

  uuid();                                        // implicitly get new UUID
  uuid(const char * uuid_string);                // import from a string
  uuid(const uuid_t c_uuid);                     // import from a C uuid
  uuid(const uuid& x);                           // copy
  ~uuid();                                       // destroy


  bool           IsNil();                        // is the UUID null ?
  unsigned short Hash16bit();                    // get the DCE 16-bit hash
  
  //
  // C++ style operator Methods
  //

  uuid& operator =  (const uuid& rhs);
  int operator   == (const uuid& x);
  int operator   != (const uuid& x);
  int operator   <  (const uuid& x);
  int operator   >  (const uuid& x);

  //
  // Java style operator methods 
  // (alternative interface also useful for Perl, Tcl, Python, etc)
  //
  
  void     Copy(const uuid& rhs);
  int      Equals(const uuid& rhs);
  int      GreaterThan(const uuid& rhs);
  int      LessThan(const uuid& rhs);
  char *   String();
  uuid_t   C_uuid();                          // Hook for legacy stuff

  //
  // Helper operators for STD/iostream
  //

  std::ostream& operator << (std::ostream &os);

};

//
// uuid_nil is ALWAYS the trivial UUID
//

extern const uuid nil_uuid;


#endif /* _UUID_CXX_X_ */
