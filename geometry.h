#ifndef __GEOMETRYH
#define __GEOMETRYH




#include "basegeo.h"

class PositionList
{
public:
  PositionList();
  PosNumber newpos(const float x[2],BaseGeo *owner);
  PosNumber doesExist(const float x[2]);
  std::vector<Position> pos;
  float epsilon;
};



class Path  //coninous list of segments
{
public:
  Path();
  std::vector<BaseGeo*> segment;
  void calculatePos(PositionList *pl);
  
  float posEntry[2];
  float posExit[2];
  float length;
  bool reverse;
  
};
#include "coord.hpp"


class Layer
{
public:
  Layer();
  std::string name;
  
  void addLine(const float x1[2],const float x2[2]);
  void addArc(const float angle1,const float angle2,const float radius,const float c[2]);
  void addVertex(float x1[2]);
  std::vector<Line*> line;
  std::vector<Arc*> arc;
  std::vector<Vertex*> vertex;
  PositionList pl;
  
  std::list<Path> path;
  void findPaths();
  void startPath(BaseGeo *start,Path &p);
  
  void optiPaths(float _addupdown);
  void trySwap();
  void tryReverse();
  bool tryGreed();
  float calcLength(std::list<Path> &path);
  float curlength;
  float addupdown;
  
  float safe, depthStart,DepthMax,depthsteps;
  void simplifypaths( double accuracy);
};

class Construct
{
public:
  Construct();
  std::vector <Layer> layer;
  Layer  *curlayer;
  
  void createLayer(const char * name);
};



#endif