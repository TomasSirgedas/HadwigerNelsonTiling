#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>

class Json
{
public:
   enum Type { NONE, OBJECT, ARRAY, STRING, NUMBER, BOOL };
   Json() { _Type = NONE; }
   Json( double number ) { _Type = NUMBER; _Number = number; }
   Json( int number ) { _Type = NUMBER; _Number = number; }
   Json( const std::string& str ) { _Type = STRING; _String = str; }
   Json( const char* str ) { _Type = STRING; _String = str; }
   template<typename T> Json( const std::vector<T>& array ) { _Type = ARRAY; _Array.insert( _Array.end(), array.begin(), array.end() ); }
   Json( bool b ) { _Type = BOOL; _Bool = b; }

   Type type() const { return _Type; }
   const std::vector<Json>& toArray() const { return _Array; }
   const std::map<std::string, Json>& toMap() const { return _Object; }
   const std::string& toString() const { return _String; }
   double toDouble() const { return _Number; }
   int toInt() const { return lround( _Number ); }
   bool toBool() const { return _Bool; }
      
   void push_back( const Json& json ) { setType( ARRAY ); _Array.push_back( json ); }
   Json& operator[]( const std::string& name ) { setType( OBJECT ); return _Object[name]; }
   const Json& operator[]( const std::string& name ) const { return _Object.count(name) ? _Object.at(name) : emptyStatic(); }
   static const Json& emptyStatic() { static Json s_emptyStatic; return s_emptyStatic; }

protected:
   Json( const std::initializer_list<Json>& array ) { _Type = ARRAY; _Array.insert( _Array.end(), array.begin(), array.end() ); }
   Json( const std::vector<Json>& array ) { _Type = ARRAY; _Array = array; }
   Json( const std::map<std::string, Json>& obj ) { _Type = OBJECT; _Object = obj; }

private:
   void setType( Type type ) { if ( _Type != NONE && type != _Type ) throw 777; _Type = type; }

private:
   Type _Type = NONE;

   std::map<std::string, Json> _Object;
   std::vector<Json>           _Array;
   std::string                 _String;
   double                      _Number = 0;
   bool                        _Bool = false;
};

class JsonObj : public Json
{
public:
   JsonObj( const std::map<std::string, Json>& obj ) : Json( obj ) {}
   JsonObj( const std::initializer_list<std::pair<std::string, Json>>& obj ) {
      for ( const auto& e : obj )
         (*this)[e.first] = e.second;
   }
};

class JsonArray : public Json
{
public:
   JsonArray() : Json( std::vector<Json>() ) {}
   JsonArray( const std::initializer_list<Json>& array ) : Json( array ) {}
};