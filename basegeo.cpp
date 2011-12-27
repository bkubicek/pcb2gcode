#include "basegeo.h"
#include "geometry.h"
#include <math.h>
using namespace std;


Vertex::Vertex(): BaseGeo()
{
  type=GeoVertex;
  length=0;
}




Line::Line(): BaseGeo()
{
  type=GeoLine;
}




Arc::Arc():BaseGeo()
{
  type=GeoArc;
}




Position::Position()
{

}

float Position::distanceTo(Position* p)
{
  return sqrt(sqr(x[0]-p->x[0])+sqr(x[1]-p->x[1]));
}


