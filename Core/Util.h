#pragma once

#include <vector>

#include "CoreMacros.h"
#include "DataTypes.h"

int mod( int x, int m );

Matrix4x4 toMatrix( const XYZ& a, const XYZ& b, const XYZ& c );
Matrix4x4 map( const std::vector<XYZ>& a, const std::vector<XYZ>& b );
uint64_t matrixHash( const Matrix4x4& m );

class Icosahedron
{
public:
   CORE_API Icosahedron();
   CORE_API Matrix4x4 map( const std::vector<int>& a, const std::vector<int>& b ) const;
   CORE_API XYZ operator[]( int index ) const { return v[index]; }

private:
   std::vector<XYZ> v;
};