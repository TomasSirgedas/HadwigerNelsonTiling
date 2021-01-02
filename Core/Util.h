#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <sstream>

#include "CoreMacros.h"
#include "DataTypes.h"
#include "trace.h"

CORE_API int mod( int x, int m );


CORE_API XYZ operator*( const Matrix4x4& m, const XYZ& p );
CORE_API Matrix4x4 toMatrix( const XYZ& a, const XYZ& b, const XYZ& c );
CORE_API Matrix4x4 map( const std::vector<XYZ>& a, const std::vector<XYZ>& b );
CORE_API uint64_t matrixHash( const Matrix4x4& m );

// rotation matrix that rotates p to the Z-axis
CORE_API Matrix4x4 matrixRotateToZAxis( XYZ& p );

template<class T>
std::string str( const std::vector<T>& v )
{
   std::stringstream ss;
   ss << "{";
   for ( int i = 0; i < (int) v.size(); i++ )
      ss << (i ? "," : "") << v[i];
   ss << "}";
   return ss.str();
}

template<class T>
std::ostream& operator<<( std::ostream& os, const std::vector<T>& v ) { return os << str( v ); }

class Icosahedron
{
public:
   CORE_API Icosahedron();
   CORE_API Matrix4x4 map( const std::vector<int>& a, const std::vector<int>& b ) const;
   CORE_API XYZ operator[]( int index ) const { return v[index]; }

private:
   std::vector<XYZ> v;
};