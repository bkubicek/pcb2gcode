
#ifndef LAYER_H
#define LAYER_H

#include <string>
#include <fstream>
using std::string;
#include <vector>
using std::vector;

#include <boost/noncopyable.hpp>

#include "coord.hpp"
#include "surface.hpp"
#include "mill.hpp"

class Layer : boost::noncopyable
{
public:
	Layer( const string& name, shared_ptr<Surface> surface, shared_ptr<RoutingMill> manufacturer, bool backside, bool mirror_absolute );
	
	vector< shared_ptr<icoords> > get_toolpaths();
	shared_ptr<RoutingMill> get_manufacturer();
	string get_name() { return name; };
	void add_mask( shared_ptr<Layer>);
  void export_layer(std::ofstream* out, bool depthfirst,float safeheight,  float finaldepth,  float startdepth, 
    float stepdepth,float feedrate_dive, float feedrate_lift,  float feedside);

private:
	string name;
	bool mirrored;
	bool mirror_absolute;
	shared_ptr<Surface> surface;
	shared_ptr<RoutingMill>    manufacturer;

	friend class Board;
};

#endif // LAYER_H
