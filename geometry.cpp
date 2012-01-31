#include "geometry.h"
#include <math.h>
#include <iostream>

#include <cstdlib>
#include <ctime>
#include <sstream>
#include <fstream>

#include <limits>
using namespace std;

PosNumber DxfNgc_PositionList::doesExist(const float x[2])
{
  for(PosNumber i=0;i<(PosNumber)pos.size();i++)
  {
    float d;
    d=fabs(pos[i].x[0]-x[0])+  fabs(pos[i].x[1]-x[1]);
    if(d<epsilon)
    {
      //pos[i].usedby.push_back(owner);
      if(verboseBaseGeo) cout<<"Old:"<<i<<"->("<<pos[i].x[0]<<" "<<pos[i].x[1]<<")"<<endl;
      if(verboseBaseGeo)
      {
       cout<<"used by:";
       for(int j=0;j<pos[i].usedby.size();j++)
         cout<<pos[i].usedby[j]<<" ";
       cout<<endl;
      }
      return i;
    }
  }
  return BadPos;
}

DxfNgc_Construct::DxfNgc_Construct()
{
  //layer.resize(1);
  //curlayer=&layer[0];
  srand(time(0));
}

void DxfNgc_Construct::createLayer(const char* _name,int flags)
{
  size_t s=layer.size();;
  layer.resize(s+1);
  //layer.push_back(Layer());
  curlayer=&layer[s];
  curlayer->setName(_name);
  curlayer->hidden= (flags==1);
  
  cout<<"Layer created:\""<<curlayer->name<<"\""<<" "<<curlayer<<endl;

}

DxfNgc_Layer::DxfNgc_Layer()
{
  curpos[0]=-1e8;
  curpos[1]=-1e8;
  safeheight=2;
  finaldepth=0;
  startdepth=0;
  stepdepth=1;
  curz=1000;
}

void split(const std::string &s, char delim, std::vector<std::string> &elems)
{
 stringstream ss(s);
 std::string item;
 while(getline(ss,item,delim))
 {
  elems.push_back(item); 
 }
}
void DxfNgc_Layer::setName(std::string _name)
{
  name=_name;
  
  vector<string> sl;
  split(name, '_',sl);
  for(int i=0;i<sl.size();i++)
  {
    vector<string> args;
    if(sl[i].length())
    {
        split(sl[i],':',args);
        if(args.size()==2)
        {
          //cout<<"pair:"<<args[0]<<"<>"<<args[1]<<endl;
          stringstream ss(args[1]);
          float x=numeric_limits<float>::max();
          ss>>x;
          if(args[0]=="safe" && x!=numeric_limits<float>::max())
          {
            safeheight=x;
          } else
          if(args[0]=="depth" && x!=numeric_limits<float>::max())
          {
            finaldepth=x;
          } else
          if(args[0]=="step" && x!=numeric_limits<float>::max())
          {
            stepdepth=x;
          } else
          if(args[0]=="start" && x!=numeric_limits<float>::max())
          {
            startdepth=x;
          }else
          if(args[0]=="f" && x!=numeric_limits<float>::max())
          {
            feedside=x;
            feedrate_dive=x;
            feedrate_lift=x;
          }
          else
          {
              cerr<<"unrecognized layer option; ";
              cerr<<"valid is: safe, depth, step, start"<<endl;
          }
        }
    }
    
  }
  if(safeheight<startdepth)
    safeheight=startdepth;
  if(finaldepth>startdepth)
    finaldepth=startdepth;
   
}

BaseGeo::BaseGeo()
{
  p[0]=BadPos;p[1]=BadPos;
  inPath=false;
  backwards=false;
}


void DxfNgc_Layer::addVertex(float x1[2])
{
  Vertex *v= new Vertex;
  vertex.push_back(v);
  v->p[0]=pl.newpos(x1,v);
  v->p[1]=v->p[0];
  if(verboseBaseGeo) cout<<"Vertex created "<<v->p[0]<<endl;
}

void DxfNgc_Layer::addLine(const float x1[2], const float x2[2], bool dropduplicates)
{
  PosNumber p1,p2;
  p1=pl.doesExist(x1);
  p2=pl.doesExist(x2);
  if(dropduplicates&&( p1!=BadPos && p2!=BadPos))
  {
    for(int i=0;i<pl.pos[p1].usedby.size();i++)
      for(int j=0;j<pl.pos[p2].usedby.size();j++)
        if(pl.pos[p1].usedby[i]==pl.pos[p2].usedby[j])
        {
         //cout<<"duplicated line found"<<endl;
         return;
          
        }
      
  }
  Line *l= new Line;
  line.push_back(l);
  l->p[0]=pl.newpos(x1,l);
  l->p[1]=pl.newpos(x2,l);
  l->length=sqrt(sqr(x1[0]-x2[0])+sqr(x1[1]-x2[1]));
  if(verboseBaseGeo) cout<<"Line created "<<l->p[0]<<"<>"<<l->p[1]<<"  "<<l<<endl;
}

void DxfNgc_Layer::addArc(const float angle1, const float angle2, const float radius,const float c[2])
{
  Arc *a= new Arc;
  arc.push_back(a);
  
  float x1[2],x2[2];
  x1[0]=c[0]+radius*cos(angle1*PI/180.);
  x1[1]=c[1]+radius*sin(angle1*PI/180.);
  x2[0]=c[0]+radius*cos(angle2*PI/180.);
  x2[1]=c[1]+radius*sin(angle2*PI/180.);
  a->p[0]=pl.newpos(x1,a);
  a->p[1]=pl.newpos(x2,a);
  a->angle1=angle1;
  a->angle2=angle2;
  a->r=radius;
  a->center[0]=c[0];
  a->center[1]=c[1];
  a->length=fabs(2*PI*radius*(angle2-angle1)/360.);
  if(verboseBaseGeo) 
    cout<<"Arc created "<<a->p[0]<<"<>"<<a->p[1]<<"  "<<a<<" "<<angle1<<" "<<angle2<<" x1:"<<x1[0]<<" "<<x1[1]<<" x2:"<<x2[0]<<" "<<x2[1]<<endl;

}





void DxfNgc_Layer::findPaths()
{
  for(int i=0;i<vertex.size();i++)
  {
   if(!vertex[i]->inPath)
   {
     
     DxfNgc_Path p;
     startPath(vertex[i],p);
     p.calculatePos(&pl);
     
     path.push_back(p);
     //cout<<"path begin length:"<<path.front().length<<endl;
     //cout<<"path back length:"<<path.back().length<<endl;
     
     
   }
  }
  for(int i=0;i<line.size();i++)
  {
   if(!line[i]->inPath)
   {
     
     DxfNgc_Path p;
     startPath(line[i],p);
     p.calculatePos(&pl);
     
     path.push_back(p);
//      cout<<"path begin length:"<<path.front().length<<endl;
//      cout<<"path back length:"<<path.back().length<<endl;
//      
     
   }
  }
  
  for(int i=0;i<arc.size();i++)
  {
   if(!arc[i]->inPath)
   {
     if(verboseBaseGeo) cout<<"new arc"<<endl;
     DxfNgc_Path p;
     startPath(arc[i],p);
     
     p.calculatePos(&pl);
     path.push_back(p);
//      cout<<"path begin length:"<<path.front().length<<endl;
//      cout<<"path back length:"<<path.back().length<<endl;
     
   }
   else
     if(verboseBaseGeo) cout<<"skipping arc"<<endl;
  }
}

void DxfNgc_Layer::startPath(BaseGeo* start, DxfNgc_Path& p)
{
  if(verboseBaseGeo) cout<<"starting path"<<endl;
  PosNumber posold=start->p[0],posnow;
  p.segment.push_back(start);
  start->inPath=true;
  
  BaseGeo *cur=start,*next;
  
  if(verboseBaseGeo) cout<<"Adding new segment "<<cur<<"["<<cur->p[0]<<"->"<<cur->p[1]<<"]"<<endl;
  do
  {

    if (cur->p[0]==posold)
    {
      posnow=cur->p[1];
      cur->backwards=false;
    }
    else
    {
      posnow=cur->p[0];
      cur->backwards=true;
    }
    //if(verbose) cout<<"posnow:"<<posnow << "used by "<<pl.pos[posnow].usedby.size()<<endl;
    next=NULL;
    if(pl.pos[posnow].usedby.size()==2)
    for(int j=0;j<pl.pos[posnow].usedby.size();j++)
      if(pl.pos[posnow].usedby[j]->inPath==false)
    {
      next=pl.pos[posnow].usedby[j];
      
      //if(verbose) cout<<"nextseg:"<<next << ", because node is used by "<<pl.pos[posnow].usedby[0]<<" and "<<pl.pos[posnow].usedby[1]<<endl;
      if(next==cur)
      {
        cerr<<"Bad"<<endl;
      }
      p.segment.push_back(next);
      
      posold=posnow;
      cur=next;
      cur->inPath=true;
      if(verboseBaseGeo) cout<<"Adding new segment "<<cur<<"["<<cur->p[0]<<"->"<<cur->p[1]<<"]"<<endl;
      break; //search loop for finding next
      
    }
  }while(next!=NULL);
}




DxfNgc_PositionList::DxfNgc_PositionList()
{
  epsilon=0.0001;
}

PosNumber DxfNgc_PositionList::newpos(const float x[2],BaseGeo *owner)
{
  for(PosNumber i=0;i<(PosNumber)pos.size();i++)
  {
    float d;
    d=fabs(pos[i].x[0]-x[0])+  fabs(pos[i].x[1]-x[1]);
    if(d<epsilon)
    {
      pos[i].usedby.push_back(owner);
      if(verboseBaseGeo) cout<<"Old:"<<i<<"->("<<pos[i].x[0]<<" "<<pos[i].x[1]<<")"<<endl;
      if(verboseBaseGeo)
      {
       cout<<"used by:";
       for(int j=0;j<pos[i].usedby.size();j++)
         cout<<pos[i].usedby[j]<<" ";
       cout<<endl;
      }
      return i;
    }
  }
  //not found
  size_t s=pos.size();
  pos.resize(s+1);
  Position &p=pos[s];
  p.x[0]=x[0];
  p.x[1]=x[1];
  p.usedby.resize(0);
  p.usedby.push_back(owner);
  if(verboseBaseGeo) cout<<"new: ("<<x[0]<<" "<<x[1]<<")->"<<s<<endl;
  return s;
}

DxfNgc_Path::DxfNgc_Path()
{
  reverse=false;

}

void DxfNgc_Path::calculatePos(DxfNgc_PositionList* pl)
{
  length=0;
  for(int i=0;i<segment.size();i++)
  {
   BaseGeo &s=*segment[i];
   length+=s.length;
    
  }
  float *f;
  PosNumber p;
  if(segment.front()->backwards)
    p=segment.front()->p[1];
  else
    p=segment.front()->p[0];
  posEntry[0]=pl->pos[p].x[0];
  posEntry[1]=pl->pos[p].x[1];

  if(segment.back()->backwards)
    p=segment.back()->p[0];
  else
    p=segment.back()->p[1];
  posExit[0]=pl->pos[p].x[0];
  posExit[1]=pl->pos[p].x[1];
  //if(verboseOpti) cout<<"Path: lenght="<<length<<" entry:"<<posEntry[0]<<" "<<posEntry[1]<<"  exit:"<<posExit[0]<<" "<<posExit[1]<<endl;
}

float distance(float *x, float *y, float *a)
{
  float xy[2];
  float xa[2];
  xy[0]=y[0]-x[0];
  xy[1]=y[1]-x[1];
  xa[0]=a[0]-x[0];
  xa[1]=a[1]-x[1];
  float l=sqrt(sqr(xy[0])+sqr(xy[1]));
  if(l==0)
  {
    return 0;
  }
  return fabs(xy[0]*xa[1]-xy[1]*xa[0])/l;
}
void DxfNgc_Layer::applyBackwards()
{
  for(std::list<DxfNgc_Path>::iterator ps = path.begin(); ps!= path.end(); ps++)
  {  
    DxfNgc_Path &p=*ps;
    
    int start=0;
    int end=p.segment.size();
    
    
    for(int j=0;j!=p.segment.size();j++)
    {
      BaseGeo *bg=p.segment[j];
      if(bg->backwards)
      {
        if(bg->type==GeoLine||bg->type==GeoArc)
        {
          PosNumber p;
          p=bg->p[0];
          bg->p[0]=bg->p[1];
          bg->p[1]=p;
          bg->backwards=false;
        }
      }
      
    }
  }
}

bool DxfNgc_Layer::trySimplifyLine(DxfNgc_Path &p, int seg_from, int seg_to)
{
  //cerr<<"try:"<<seg_from<<"<>"<<seg_to<<" of "<<p.segment.size()<<endl;
  float *from, *to;
  from=pl.pos[p.segment[seg_from]->p[0]].x;
  
  to=pl.pos[p.segment[seg_to]->p[1]].x;
  
  const float maxdev=pl.epsilon*sqrt(2);
  for(int i=seg_from;i<seg_to;i++)
  {
//     cerr<<"i"<<i<<endl;
//     cerr<<"from="<<from[0]<<" "<<from[1]<<endl;
//     cerr<<"to="<<to[0]<<" "<<to[1]<<endl;
    if(p.segment[i]->type!=GeoLine)
      return false; //only lines can be optimized
   float d2=distance(from,to,pl.pos[p.segment[i]->p[1]].x ) ;
   if( d2>maxdev)
     return false;
  }
  //simplify=good
  //cerr<<"simplify success"<<endl;
  PosNumber from_p,to_p;
  
  p.segment[seg_from]->p[1]=p.segment[seg_to  ]->p[0];
  
  
  p.segment.erase(p.segment.begin()+seg_from+1,p.segment.begin()+seg_to);
  
  //cerr<<"erase done"<<endl;
  return true;  
}

void DxfNgc_Layer::simplifyPaths()
{
  cerr<<"Simplify paths"<<endl;
  
  applyBackwards();
  for(list<DxfNgc_Path>::iterator ps = path.begin()++; ps!= path.end(); ps++)
  {
    DxfNgc_Path &p=*ps;
    //cerr<<"one path length="<<ps->segment.size()<<endl;
    
    for(int segsimply=10;segsimply>2;segsimply--)
    {
      //cerr<<"simplify ="<<segsimply<<" "<<ps->segment.size()<<endl;
      int laststart=0;
    
      bool found;
      if(p.segment.size()>segsimply)
      do
      {
        found=false;
        for(int j=laststart;j<p.segment.size()-segsimply-2;j++)
        {
          if(j+segsimply>=p.segment.size()-2)
            break;
          laststart=j+1;
          //cerr<<"try:"<<j<<"<>"<<j+segsimply<<" "<<p.segment.size()<<endl;
          if(trySimplifyLine(p,j,j+segsimply)) 
          {
            found=true;
            //break;
          }      
         }
       }while(found &&laststart<p.segment.size()-segsimply-1 );
    }
    
  }
}

void DxfNgc_Layer::optiPaths(float _addupdown)
{
  return;
  addupdown=_addupdown;
  if(path.size()<3)
    return;
  curlength=calcLength(path);
  cout<<"Starting Length:"<<curlength<<" paths:"<<path.size()<<endl;
  
  PosNumber max=20*path.size()*path.size();
  
  for(int i=0;i<max;i++)
  {
    tryGreed();
    if(i%1000==0    )
     cout<<i<<":"<<" Current length:"<<curlength<<endl; 
  
  //int cnt=0;
  
//    while(tryGreed() &&(cnt++<path.size()*10)&& !cin.rdbuf()->in_avail() )
//    {
//     cout<<cnt<<":"<<" Current length:"<<curlength<<endl; 
//    }
  }
  cout<<"Final Length:"<<curlength<<endl;
  
}

float DxfNgc_Layer::calcLength(std::list<DxfNgc_Path> &_path)
{
  float newlength=0;
  float *curpos;
  list<DxfNgc_Path>::iterator ps1 = _path.begin();
  if(!ps1->reverse)
  { 
    curpos=(ps1->posExit);
  }
  else
  {
    curpos=(ps1->posEntry);
  }
  //if(verboseOpti) cout<<"Pathlenght="<<ps1->length<<" entry:"<<ps1->posEntry[0]<<" "<<ps1->posEntry[1]<<"  exit:"<<ps1->posExit[0]<<" "<<ps1->posExit[1]<<endl;
 
  for(list<DxfNgc_Path>::iterator ps = _path.begin()++; ps!= _path.end(); ps++)
  {
    //if(verboseOpti) cout<<"currentpos: "<<curpos[0]<<" "<<curpos[1]<<endl;
    float add=0;
    if(!ps->reverse)
      add+=sqrt(  sqr(ps->posEntry[0]-curpos[0])+sqr(ps->posEntry[1]-curpos[1]) );
    else
      add+=sqrt(  sqr(ps->posExit[0]-curpos[0])+sqr(ps->posExit[1]-curpos[1]) );
    if(add>0.001) newlength+=addupdown;
    newlength+=add;
    if(!ps->reverse)
    { 
      curpos=ps->posExit;
    }
    else
    {
      curpos=ps->posEntry;
    }
  }
  return newlength;
}
void DxfNgc_Layer::trySwap()
{
  
  PosNumber p1=rand()%path.size();
  PosNumber p2=rand()%path.size();
  //if(verboseOpti)cout<<"trying swap:"<<p1<<" "<<p2<<endl;
  if(p1==p2) return;
  list<DxfNgc_Path>::iterator ip1=path.begin();
  list<DxfNgc_Path>::iterator ip2=path.begin();
  advance(ip1,p1);
  advance(ip2,p2);
  path.splice(ip1,path,ip2);
  //if(verboseOpti) cout<<"swap ok"<<endl;
  float newlength=calcLength(path);
  
      

  if(newlength<curlength)
  {
    //if(verboseOpti) cout<<"++++path decreased:"<<newlength<<" previous: "<<curlength<<endl;
    curlength=newlength;
    
  }
  else
  {
    //if(verboseOpti) cout<<"++++path increased"<<newlength<<" previous: "<<curlength<<endl;
    list<DxfNgc_Path>::iterator ip1=path.begin();
    list<DxfNgc_Path>::iterator ip2=path.begin();
    advance(ip1,p1);
    advance(ip2,p2);
    path.splice(ip1,path,ip2);
  
  }
  
  
}


void DxfNgc_Layer::tryReverse()
{
  
  PosNumber p1=rand()%path.size();
  list<DxfNgc_Path>::iterator ip1=path.begin();
  advance(ip1,p1);
  ip1->reverse=!ip1->reverse;
  float newlength=calcLength(path);
  if(newlength<curlength)
  {
    //if(verboseOpti) cout<<"===reverse good:"<<p1<<" "<<"path decreased "<<newlength<<" previous: "<<curlength<<endl;
    curlength=newlength;
  }
  else
  {
    //if(verboseOpti) cout<<"===reverse bad:"<<p1<<" "<<"path increased "<<newlength<<" previous: "<<curlength<<endl;
    ip1->reverse=!ip1->reverse;
  }
}

bool DxfNgc_Layer::tryGreed()
{
  PosNumber s=path.size();
  
  int rev=rand()%2;
   int i=rand()%(s-3);
    int j=i+1+rand()%(s-i-2);
    int m=j+rand()%(s-j-1);
  //for(int i=0;i<s-2;i++)
  //for(int j=i+1;j<s-1;j++)
  //for(int m=s-1;m>j;m--)
    
  //int rev=rand()%2;
  {
   
      list<DxfNgc_Path> newpath=path;
    PosNumber p1=i;
    PosNumber p2=j;
   // if(verboseOpti)cout<<"trying swap:"<<p1<<" "<<p2<<"-"<<m<<endl;
    //if(p1==p2) return;
    list<DxfNgc_Path>::iterator ip1=newpath.begin();
    list<DxfNgc_Path>::iterator ip2=newpath.begin();
    list<DxfNgc_Path>::iterator ip2to=newpath.begin();
    advance(ip1,p1);
    advance(ip2,p2);
    advance(ip2to,m);
    for(list<DxfNgc_Path>::iterator qq=ip2;qq!=ip2to;qq++)
      if(rev) qq->reverse=!qq->reverse;
    newpath.splice(ip1,path,ip2,ip2to);
    //if(verboseOpti) cout<<"swap ok"<<endl;
    float newlength=calcLength(newpath);
    
        

    if(newlength<curlength)
    {
      //if(verboseOpti) cout<<"++++path decreased:"<<newlength<<" previous: "<<curlength<<endl;
      curlength=newlength;
      path=newpath;
      return true;
      
    }
    else
    {
      //if(verboseOpti) cout<<"++++path increased"<<newlength<<" previous: "<<curlength<<endl;
      /*list<Path>::iterator ip1=path.begin();
      list<Path>::iterator ip2=path.begin();
      list<Path>::iterator ip1to=path.begin();
      advance(ip1,p1);
      advance(ip1to,m-p2);
      advance(ip2,p2);
      if(rev) ip1->reverse=!ip1->reverse;
      path.splice(ip2,path,ip1, ip1to);
        float newlength=calcLength();
      cout<<"Reverted length:"<<newlength<<" "<<curlength<<endl;
    */
    }
  }
  return false;
}






void DxfNgc_Layer::exportSegment(std::ofstream &out,BaseGeo *bg)
{
  out.setf ( ios::fixed);

  int fromindex=0;
  int newindex=1;
  
  if(1 && bg->backwards)
  {
    fromindex=1;
    newindex=0;
  }
  
  float *xfrom=pl.pos[ bg->p[fromindex]].x;
  //out<<"( from "<<xfrom[0]<<" "<<xfrom[1]<<" z:"<<curz<<")"<<endl;
  //out<<"( cur pos:"<<curpos[0]<<" "<<curpos[1]<<" "<<curpos[2]<<")"<<endl;
  if((xfrom[0]!=curpos[0])||(xfrom[1]!=curpos[1])) //different position
  {
    if(curpos[2]<safeheight)
    {
      //out<<"( heihgt smaller safe: "<<curpos[2]<<" )"<<endl;
      out<<"G4 P0"<<endl;
      out<<"G1 Z"<<startdepth<<" F"<<feedrate_lift<<endl; // so it is forced straight up
      out<<"G0 Z"<<safeheight<<endl;
      curpos[2]=safeheight;
    }
    //else
    //  out<<"G0 Z"<<safeheight<<endl;
     //out<<"( move other pos: )"<<endl;
    out<<"G0 X"<<xfrom[0]<<" Y"<<xfrom[1]<<endl;
    out<<"G0 Z"<<safeheight<<endl;
    out<<"G1 Z"<<curz<<" F"<<feedrate_dive<<endl;
    out<<"G4 P0"<<endl;
    out<<"F"<<feedside<<endl;
    curpos[0]=xfrom[0];curpos[1]=xfrom[1];curpos[2]=curz;
  }
  else //same position 
  if(curpos[2]!=curz) //but different height
  {
    out<<"G1 Z"<<curz<<endl;
    out<<"G4 P0"<<endl;
    curpos[2]=curz;
  }
  
  
  float *x=pl.pos[ bg->p[newindex]].x;
  switch(bg->type)
  {
    case GeoLine:
    {   
      out<<"G1 X"<<x[0]<<" Y"<<x[1]<<endl; 
    }break;
    case GeoArc:
    {
      bool ccw=false;
      Arc *l=(Arc*)bg;
      if(l->angle2>l->angle1) ccw=false;
      if(l->backwards) ccw=!ccw;
      if(ccw)
        out<<"G2";
      else
        out<<"G3";
      
      out<<" X"<<x[0]<<" Y"<<x[1]<<" I"<<l->center[0]-xfrom[0]<<" J"<<l->center[1]-xfrom[1]<<endl;
        //out<<" X"<<x[0]<<" Y"<<x[1]<<" I"<<l->center[0]-x[0]<<" J"<<l->center[1]-x[1]<<endl;
      //cout<<"ARC export: "<<x[0]<<" "<<x[1]<<" from:"<<xfrom[0]<<" "<<xfrom[1]<<" center: "<<l->center[0]<<" "<<l->center[1]<<" "<<curpos[0]<<" "<<curpos[1]<<endl;
    }break;
    case GeoVertex:
    {
       out<<"G1 X"<<x[0]<<" Y"<<x[1]<<endl;
    }break;
    default:
      cerr<<"bad basetype"<<bg->type<<endl;
      ;
  }
  curpos[0]=x[0];curpos[1]=x[1];
}


void DxfNgc_Layer::exportgcode(std::ofstream *_out)
{
  out=_out;
  
  cout<<"Exporting layer \""<<name<<"\" with "<<path.size()<<" paths"<<endl;
  *out<<endl<<"; layer "<<name<<endl;
  cout<<"Required steps:";
  int n=1+floor(fabs(finaldepth-startdepth)/stepdepth);
  for(int ii=n-1;ii>=0;ii--)
  {
   cout<<ii*stepdepth+finaldepth<<" ";
  }
  cout<<endl;
  for(int ii=n-1;ii>=0;ii--)
  {
    curz=123;
    curpos[2]=1000;
    
    float add=ii*stepdepth;
    curz=add+finaldepth;
   // cout<<"curz="<<curz<<" final="<<finaldepth<<" "<<" add="<<add<<endl;
    *out<<"; depth="<<curz<<" i="<<ii<<" stepdepth"<<stepdepth<<" finaldepth="<<finaldepth<<" "<<add<<endl;
    
    
    for(std::list<DxfNgc_Path>::iterator ps = path.begin(); ps!= path.end(); ps++)
    {  
      DxfNgc_Path &p=*ps;
      *out<<"G4 P0"<<endl;
      *out<<"(new path)"<<endl;
      //cout<<"exporting path"<<endl;
      int start=0;
      int end=p.segment.size();
      if(p.reverse)
      {
        end=-1;
        start=p.segment.size()-1;
      }
      
      for(int j=start;j!=end;)
      {
        BaseGeo *bg=p.segment[j];
        if(p.reverse)
        {
          bg->backwards=!bg->backwards;
        }
        exportSegment(*out,bg);
        
        if(p.reverse)
          j--;
        else
          j++;
        
      }
    }
    
    
  }
  *out<<"G4 P0"<<endl;
  *out<<"G1 Z"<<startdepth<<endl; // so it is forced straight up
  *out<<"G0 Z"<<safeheight<<endl;
}

void DxfNgc_Layer::exportgcodeDepthfirst(std::ofstream *_out)
{
  out=_out;
  cerr<<"Exporting layer \""<<name<<"\""<<endl;
  *out<<endl<<"; layer "<<name<<endl;
  cerr<<"Required steps:";
  int n=1+floor(fabs(finaldepth-startdepth)/stepdepth);
  for(int ii=n-1;ii>=0;ii--)
  {
   cout<<ii*stepdepth+finaldepth<<" ";
  }
  cout<<endl;
   
  curpos[2]=1000;
  for(std::list<DxfNgc_Path>::iterator ps = path.begin(); ps!= path.end(); ps++)
  for(int ii=n-1;ii>=0;ii--)
  {
        curz=123;
    float add=ii*stepdepth;
    curz=add+finaldepth;
    {  
      
      DxfNgc_Path &p=*ps;
      //cout<<"exporting path"<<endl;
      int start=0;
      int end=p.segment.size();
      if(p.reverse)
      {
        end=-1;
        start=p.segment.size()-1;
      }
      
      for(int j=start;j!=end;)
      {
        BaseGeo *bg=p.segment[j];
        if(p.reverse)
        {
          bg->backwards=!bg->backwards;
        }
        exportSegment(*out,bg);
       
        if(p.reverse)
          j--;
        else
          j++;
        
      }
    }
    
    
  }
  *out<<"G4 P0"<<endl;
  *out<<"G1 Z"<<startdepth<<endl; // so it is forced straight up
  *out<<"G0 Z"<<safeheight<<endl;
}
