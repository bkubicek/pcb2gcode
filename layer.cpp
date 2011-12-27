
#include "layer.hpp"

Layer::Layer( const string& name, shared_ptr<Surface> surface, shared_ptr<RoutingMill> manufacturer, bool backside, bool mirror_absolute )
{
	this->name = name;
	this->mirrored = backside;
	this->mirror_absolute = mirror_absolute;
	this->surface = surface;
	this->manufacturer = manufacturer;
}

#include <iostream>
using namespace std;

vector< shared_ptr<icoords> >
Layer::get_toolpaths()
{
	return surface->get_toolpath( manufacturer, mirrored, mirror_absolute );
}

shared_ptr<RoutingMill>
Layer::get_manufacturer()
{
	return manufacturer;
}

void
Layer::add_mask( shared_ptr<Layer> mask)
{
	surface->add_mask( mask->surface);
}

void Layer::export_layer(ofstream* out, bool depthfirst,float safeheight,  float finaldepth,  float startdepth, float stepdepth,float feedrate_dive, float feedrate_lift,  float feedside)
{
  get_toolpaths();
  surface->l.safeheight=safeheight;
  surface->l.finaldepth=finaldepth;
  surface->l.startdepth=startdepth;
  surface->l.stepdepth=stepdepth;
  surface->l.feedrate_dive=feedrate_dive;
  surface->l.feedrate_lift=feedrate_lift;
  surface->l.feedside=feedside;
  surface->l.name=name;
  if(depthfirst)
    surface->l.exportgcodeDepthfirst(out);
  else
    surface->l.exportgcode(out);
  
}
