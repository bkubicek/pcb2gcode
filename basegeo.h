#ifndef __BASEGEO
#define __BASEGEO

#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <math.h>

enum GeoType {GeoVertex=0,GeoLine=1, GeoArc=2 };

typedef  float Coordinates[2];
typedef  unsigned long PosNumber;
const PosNumber BadPos=-1;

const float PI= atan(1.0)*4;
const bool verboseBaseGeo=false;
const bool verboseOpti=true;

class PositionList;
class Position;

inline float sqr(float x) {return x*x;};


class BaseGeo
{
public:
  BaseGeo();
  GeoType type;
  PosNumber p[2];
  float length;
  bool inPath;
  bool backwards;
};

class Vertex: public BaseGeo
{
public:
  Vertex();
};


class Line: public BaseGeo
{
public:
  Line();
  
};

class Arc: public BaseGeo
{
public:
  Arc();
  float angle1, angle2;
  float center[2];
  float r;
};




class Position{
public:
  Position();
  float distanceTo(Position *p);
  
  float x[2];
  std::vector<BaseGeo*> usedby;
};


#endif