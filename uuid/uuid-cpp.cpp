
#include <uuid.hpp>
#include <string.h>

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
//
// DEFAULT CONSTRUCTOR:  create a NEW UUID value
// 
// 

uuid::uuid()
{
  s_uuid = 0;
  uuid_create(&i_uuid, &e);
  uuid_to_string(&i_uuid, (unsigned char **)&s_uuid, &e);
}

//
// CONSTRUCTOR:           create from a stringified UUID
//  
// Special Case:   If a NULL string, or an EMPTY string is passed
//                 then we generate a NEW, VALID UUID - NOT a nil UUID.
//
//

uuid::uuid(const char * uuid_string)
{
  if (uuid_string && strlen(uuid_string))
    {
      s_uuid = strdup(uuid_string);      
      uuid_from_string((unsigned char *)s_uuid, &i_uuid, &e);
    }
  else
    {
      uuid_create(&i_uuid, &e);
      uuid_to_string(&i_uuid, (unsigned char **)&s_uuid, &e);
    }
  if (e != uuid_s_ok) make_nil();
}

//
// CONSTRUCTOR:           create from a C Language uuid_t type
//

uuid::uuid(const uuid_t c_uuid)
{
  i_uuid = c_uuid;

  uuid_to_string(&i_uuid, (unsigned char **)&s_uuid, &e);

  if (e != uuid_s_ok) make_nil();
}

//
// CONSTRUCTOR:    Copy a UUID
//

uuid::uuid(const uuid& x)
{
  i_uuid = x.i_uuid;
  if (x.s_uuid)
    s_uuid = strdup(x.s_uuid);
}

//
// ASSIGNMENT operator:   Copy a UUID
//

uuid&
uuid::operator = (const uuid& rhs)
{
  i_uuid = rhs.i_uuid;
  if (s_uuid)
    {
      free(s_uuid);
      s_uuid = strdup(rhs.s_uuid);
    }
  else
    s_uuid = NULL;
}


//
// DESTRUCTOR
//

uuid::~uuid()
{
  if (s_uuid) free(s_uuid);
}

//
// isNil():          Returns true is this contains a NIL UUID
//

bool
uuid::IsNil()
{
  return uuid_is_nil(&i_uuid, &e);
}

//
// Hash16bit():      Returns the 16-bit hash value, used by DCE
//

unsigned short
uuid::Hash16bit()
{
  unsigned short p;

  p = uuid_hash(&i_uuid, &e);
  if (e != uuid_s_ok) return 0;
  return p;

}

//
// COMPARISON OPERATORS (overloaded operators C++ style)
//

int
uuid::operator == (const uuid& x)
{
  boolean32 s;

  s = uuid_equal(&i_uuid, (uuid_t *)&x.i_uuid, &e);
  if (e == uuid_s_ok) return (s == TRUE);
  return 0;
}

int
uuid::operator != (const uuid& x)
{
  boolean32 s;

  s = uuid_equal(&i_uuid, (uuid_t *)&x.i_uuid, &e);
  if (e == uuid_s_ok ) return (s == FALSE);
  return 0;
}

int
uuid::operator > (const uuid& x)
{
  unsigned32 c;

  c = uuid_compare(&i_uuid, (uuid_t *)&x.i_uuid, &e);
  if (e == uuid_s_ok) return (c > 0);
  return 0;
}

int
uuid::operator < (const uuid& x)
{
  unsigned32 c;

  c = uuid_compare(&i_uuid, (uuid_t *)&x.i_uuid, &e);
  if (e == uuid_s_ok) return (c < 0);
  return 0;

}


//
// COMPARISON OPERATORS (Java style)
//


void
uuid::Copy(const uuid& rhs)
{
  *this = rhs;
}


int
uuid::Equals(const uuid& rhs)
{
  return ( *this == rhs);
}


int
uuid::GreaterThan(const uuid& rhs)
{
  return ( *this > rhs);
}


int
uuid::LessThan(const uuid& rhs)
{
  return ( *this < rhs);
}

//
// Get string representation
//

char *
uuid::String()
{
  return s_uuid;
}

//
// Get C uuid_t representation 
//

uuid_t 
uuid::C_uuid()
{
  return i_uuid;
}

//
// Stream methods
//


ostream& 
uuid::operator << (ostream &os)
{
  return os << s_uuid;
}

void
uuid::make_nil()
{
  uuid_create_nil(&i_uuid, &e);
  if (s_uuid) strcpy(s_uuid, "");
}


//
// Global NIL uuid
//

const uuid nil_uuid = uuid("nil");






