#include "Util.h"

#include <string>

int mod( int x, int m ) { return x >= 0 ? x % m : (x+1) % m + m-1; }


Matrix4x4 toMatrix( const XYZ& a, const XYZ& b, const XYZ& c )
{
   return Matrix4x4( a.x, b.x, c.x, 0,
                     a.y, b.y, c.y, 0, 
                     a.z, b.z, c.z, 0, 
                     0,   0,   0  , 1 );
}

Matrix4x4 map( const std::vector<XYZ>& a, const std::vector<XYZ>& b )
{
   if ( a.size() == 3 && b.size() == 3 )
      return toMatrix( b[0], b[1], b[2] ) * toMatrix( a[0], a[1], a[2] ).inverted();
   if ( a.size() == 4 && b.size() == 4 )
   {
      Matrix4x4 m = ::map( { a[1]-a[0], a[2]-a[0], a[3]-a[0] }, { b[1]-b[0], b[2]-b[0], b[3]-b[0] } );
      return Matrix4x4::translation( b[0] ) * m * Matrix4x4::translation( -a[0] );
   }
   throw 777;
}


Icosahedron::Icosahedron()
{
   const double PHI = .5 + sqrt(1.25);

   v = { XYZ(    -1,    0, -PHI )  
       , XYZ(     1,    0, -PHI )  
       , XYZ(     0,  PHI,   -1 )  
       , XYZ(  -PHI,    1,    0 )  
       , XYZ(  -PHI,   -1,    0 )  
       , XYZ(     0, -PHI,   -1 )
       , XYZ(     1,    0,  PHI )  
       , XYZ(    -1,    0,  PHI )  
       , XYZ(     0, -PHI,    1 )  
       , XYZ(   PHI,   -1,    0 )  
       , XYZ(   PHI,    1,    0 )  
       , XYZ(     0,  PHI,    1 ) };

   for ( XYZ& p : v ) 
      p = p.normalized();
}

Matrix4x4 Icosahedron::map( const std::vector<int>& a, const std::vector<int>& b ) const
{
   return ::map( std::vector<XYZ> { v[a[0]], v[a[1]], v[a[2]] }, std::vector<XYZ> { v[b[0]], v[b[1]], v[b[2]] } );
}

uint64_t matrixHash( const Matrix4x4& m )
{
   std::wstring str;
   std::vector<double> v = { m[0].x, m[0].y, m[0].z, m[0].w
                           , m[1].x, m[1].y, m[1].z, m[1].w
                           , m[2].x, m[2].y, m[2].z, m[2].w
                           , m[3].x, m[3].y, m[3].z, m[3].w };
      
   static_assert( sizeof(wchar_t) >= 2 );
   for ( double x : v )
      str += wchar_t( lround( x * 1000 ) );
   
   uint64_t h = std::hash<std::wstring>{}( str );
   return h;
}