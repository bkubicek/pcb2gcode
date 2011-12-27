#ifndef __GEOMETRYH
#define __GEOMETRYH


  


#include "basegeo.h"

class DxfNgc_PositionList
{
public:
  DxfNgc_PositionList();
  PosNumber newpos(const float x[2],BaseGeo *owner);
  PosNumber doesExist(const float x[2]);
  
  std::vector<Position> pos;
  float epsilon;
};



class DxfNgc_Path  //coninous list of segments
{
public:
  DxfNgc_Path();
  std::vector<BaseGeo*> segment;
  void calculatePos(DxfNgc_PositionList *pl);
  
  float posEntry[2];
  float posExit[2];
  float length;
  bool reverse;
  
};



class DxfNgc_Layer
{
public:
  DxfNgc_Layer();
  void setName(std::string _name);

  std::string name;
  
  void addLine(const float x1[2],const float x2[2],bool dropduplicates=false);
  void addArc(const float angle1,const float angle2,const float radius,const float c[2]);
  void addVertex(float x1[2]);
  std::vector<Line*> line;
  std::vector<Arc*> arc;
  std::vector<Vertex*> vertex;
  DxfNgc_PositionList pl;
  
  std::list<DxfNgc_Path> path;
  void findPaths();
  void startPath(BaseGeo *start,DxfNgc_Path &p);
  void applyBackwards();
  bool trySimplifyLine(DxfNgc_Path &p, int seg_from, int seg_to);
  void simplifyPaths();
  void optiPaths(float _addupdown);
  void trySwap();
  void tryReverse();
  void setEpsilon(float epsilon) {pl.epsilon=epsilon;};
  bool tryGreed();
  float calcLength(std::list<DxfNgc_Path> &path);
  float curlength;
  float addupdown;
  bool hidden;
  
  float safe, depthStart,DepthMax,depthsteps;
  
public: //exporting
  float curpos[3],oldpos[3];
  void exportgcode(std::ofstream *_out );
  void exportgcodeDepthfirst(std::ofstream *_out);
  void exportSegment(std::ofstream &out,BaseGeo *bg);
  std::ofstream *out;

  float safeheight;
  float finaldepth;
  float startdepth;
  float stepdepth;
  bool forcefirst;
  float feedrate_dive;
  float feedrate_lift;
  float feedside;
  float curz;
};


class DxfNgc_Construct
{
public:
  DxfNgc_Construct();
  std::vector <DxfNgc_Layer> layer;
  DxfNgc_Layer  *curlayer;
  
  void createLayer(const char * name,int flags);
};



#endif