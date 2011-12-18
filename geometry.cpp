#include "geometry.h"
#include <math.h>
#include <iostream>

#include <cstdlib>
#include <ctime>
using namespace std;


Construct::Construct()
{
  //layer.resize(1);
  //curlayer=&layer[0];
  srand(time(0));
}

void Construct::createLayer(const char* name)
{
  size_t s=layer.size();;
  layer.resize(s+1);
  curlayer=&layer[s];
  curlayer->name=name;
   cout<<"Layer created:\""<<name<<"\""<<endl;
}

Layer::Layer()
{

}

BaseGeo::BaseGeo()
{
  p[0]=BadPos;p[1]=BadPos;
  inPath=false;
  backwards=false;
}


void Layer::addVertex(float x1[2])
{
 
}

void Layer::addLine(const float x1[2], const float x2[2])
{
  PosNumber p1,p2;
  p1=pl.doesExist(x1);
  p2=pl.doesExist(x2);
  if(p1!=BadPos && p2!=BadPos)
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

void Layer::addArc(const float angle1, const float angle2, const float radius,const float c[2])
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
  //if(verboseBaseGeo) 
    cout<<"Arc created "<<a->p[0]<<"<>"<<a->p[1]<<"  "<<a<<" "<<angle1<<" "<<angle2<<endl;

}





void Layer::findPaths()
{
  for(int i=0;i<line.size();i++)
  {
   if(!line[i]->inPath)
   {
     
     Path p;
     startPath(line[i],p);
     p.calculatePos(&pl);
     
     path.push_back(p);
     //cout<<"path begin length:"<<path.front().length<<endl;
     //cout<<"path back length:"<<path.back().length<<endl;
     
     
   }
  }
  
  for(int i=0;i<arc.size();i++)
  {
   if(!arc[i]->inPath)
   {
     if(verboseBaseGeo) cout<<"new arc"<<endl;
     Path p;
     startPath(arc[i],p);
     
     p.calculatePos(&pl);
     path.push_back(p);
     cout<<"path begin length:"<<path.front().length<<endl;
     cout<<"path back length:"<<path.back().length<<endl;
     
   }
   else
     if(verboseBaseGeo) cout<<"skipping arc"<<endl;
  }
}

void Layer::startPath(BaseGeo* start, Path& p)
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




PositionList::PositionList()
{
  epsilon=0.00001;
}

PosNumber PositionList::doesExist(const float x[2])
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

PosNumber PositionList::newpos(const float x[2],BaseGeo *owner)
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

Path::Path()
{
  reverse=false;

}

void Path::calculatePos(PositionList* pl)
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

double distancePointLine(const icoordpair &x,const icoordpair &la,const  icoordpair &lb)
{
  icoordpair nab; //normal vector to a-b= {-ab_y,ab_x}
  nab.first=-(la.second-lb.second);
  nab.second=(la.first-lb.first);
  double lnab=sqrt(nab.first*nab.first+nab.second*nab.second);
  double skalar; //product

  skalar=nab.first*(x.first-la.first)+nab.second*(x.second-la.second);
  return fabs(skalar/lnab );
}

void Layer::simplifypaths(double accuracy)
{
//   //take two points of the path
//   // and their interconnecting path.
//   // if the distance between all intermediate points and this line is smaller
//   // than the accuracy, all the points in between can be removed..
//   int i=0;
//   bool change;
//   int lasterased=0;
//   const bool debug=true;
//   std::list<icoordpair> l;
//   for(int i=0;i<path[i].segment->size();i++)
//   {
//     icoordpair &ii=(*outline)[i];
//     l.push_back(ii);
//   }
// 
//   if (debug) cerr<<"outline size:"<<outline->size()<<" accuracy"<<accuracy<<endl;
//   int pos=0;
//   do //cycle until no two points can be combined..
//   {
//     change=false;
// 
//     list<icoordpair>::iterator a=l.begin();
//     do
//     {
//       list<icoordpair>::iterator b,c;
//       b=a;b++;
//       c=b;c++;
//       if((b==l.end()))
//         break;
//       double d=distancePointLine(*b, *a,*c);
//       if((d<accuracy) )
//       {
// 
//         if(debug) cerr<<"erasing at"<<pos<<" of "<<l.size()<<" d="<<d<<endl;
//         a=l.erase(b);
//         change=true;
//       }
//       else
//         a=b;
//       pos++;
//     }while(a!=l.end());
//     //change=false;
//   }
//   while(change);
// 
//   if(debug) cout<<"copying"<<endl;
//   outline->resize(0);
//   for( list<icoordpair>::iterator a=l.begin();a!=l.end();a++)
//     outline->push_back(*a);
//   if(debug) cerr<<"outline size:"<<outline->size()<<endl;

}
void Layer::optiPaths(float _addupdown)
{
  addupdown=_addupdown;
  if(path.size()<3  )
    return;
  cout<<"Nr paths:"<<path.size()<<endl;
  curlength=calcLength(path);
  cout<<"Starting Length:"<<curlength<<endl;
  
  PosNumber max=20*path.size()*path.size();
  
  for(int i=0;i<max;i++)
  {
    tryGreed();
    if(i%1000==0    )
     cout<<i<<":"<<" Current length:"<<curlength<<endl; 
  }
  int cnt=0;
  
//    while(tryGreed() &&(cnt++<path.size()*10)&& !cin.rdbuf()->in_avail() )
//    {
//     cout<<cnt<<":"<<" Current length:"<<curlength<<endl; 
//    }
  cout<<"Final Length:"<<curlength<<endl;
  
}

float Layer::calcLength(std::list<Path> &_path)
{
  float newlength=0;
  float *curpos;
  list<Path>::iterator ps1 = _path.begin();
  if(!ps1->reverse)
  { 
    curpos=(ps1->posExit);
  }
  else
  {
    curpos=(ps1->posEntry);
  }
  //if(verboseOpti) cout<<"Pathlenght="<<ps1->length<<" entry:"<<ps1->posEntry[0]<<" "<<ps1->posEntry[1]<<"  exit:"<<ps1->posExit[0]<<" "<<ps1->posExit[1]<<endl;
 
  for(list<Path>::iterator ps = _path.begin()++; ps!= _path.end(); ps++)
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
void Layer::trySwap()
{
  
  PosNumber p1=rand()%path.size();
  PosNumber p2=rand()%path.size();
  //if(verboseOpti)cout<<"trying swap:"<<p1<<" "<<p2<<endl;
  if(p1==p2) return;
  list<Path>::iterator ip1=path.begin();
  list<Path>::iterator ip2=path.begin();
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
    list<Path>::iterator ip1=path.begin();
    list<Path>::iterator ip2=path.begin();
    advance(ip1,p1);
    advance(ip2,p2);
    path.splice(ip1,path,ip2);
  
  }
  
  
}


void Layer::tryReverse()
{
  
  PosNumber p1=rand()%path.size();
  list<Path>::iterator ip1=path.begin();
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

bool Layer::tryGreed()
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
   
      list<Path> newpath=path;
    PosNumber p1=i;
    PosNumber p2=j;
   // if(verboseOpti)cout<<"trying swap:"<<p1<<" "<<p2<<"-"<<m<<endl;
    //if(p1==p2) return;
    list<Path>::iterator ip1=newpath.begin();
    list<Path>::iterator ip2=newpath.begin();
    list<Path>::iterator ip2to=newpath.begin();
    advance(ip1,p1);
    advance(ip2,p2);
    advance(ip2to,m);
    for(list<Path>::iterator qq=ip2;qq!=ip2to;qq++)
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