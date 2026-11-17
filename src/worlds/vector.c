#include "worlds_internal.h"
#include <math.h>

static float Cross(Vector2 a,Vector2 b,Vector2 c)
{ return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x); }
static void Tri(Vector2 a,Vector2 b,Vector2 c,Color color)
{ if(Cross(a,b,c)>0) DrawTriangle(a,c,b,color);else DrawTriangle(a,b,c,color); }
void WPoly(const Vector2 *p,int count,Color color)
{
    int ids[128];if(count<3 || count>128) return;
    float area=0;
    for(int i=0;i<count;i++){ids[i]=i;area+=p[i].x*p[(i+1)%count].y-p[(i+1)%count].x*p[i].y;}
    float direction=area>=0?1.0f:-1.0f;
    int n=count;
    while(n>2) {
        bool found=false;
        for(int k=0;k<n;k++) {
            int prev=(k+n-1)%n,next=(k+1)%n;
            Vector2 a=p[ids[prev]],b=p[ids[k]],c=p[ids[next]];
            if(Cross(a,b,c)*direction<=0)continue;
            bool occupied=false;
            for(int j=0;j<n;j++) if(j!=prev && j!=k && j!=next) {
                Vector2 q=p[ids[j]];
                if(Cross(a,b,q)*direction>=0 && Cross(b,c,q)*direction>=0 && Cross(c,a,q)*direction>=0)occupied=true;
            }
            if(occupied)continue;
            Tri(a,b,c,color);
            for(int j=k;j+1<n;j++)ids[j]=ids[j+1];
            n--;found=true;break;
        }
        if(!found)break;
    }
}
void WOval(float x,float y,float rx,float ry,Color color,float angle)
{
    float co=cosf(angle),si=sinf(angle);Vector2 center={x,y},prev={x+rx*co,y+rx*si};
    for(int i=1;i<=40;i++) {
        float a=i*(2*PI/40),px=cosf(a)*rx,py=sinf(a)*ry;
        Vector2 next={x+px*co-py*si,y+px*si+py*co};Tri(center,prev,next,color);prev=next;
    }
}
void WLine(const Vector2 *p,int count,Color color,float width)
{
    for(int i=0;i+1<count;i++)DrawLineEx(p[i],p[i+1],width,color);
    /* Round only the path ends. Alpha circles at every sample make translucent
     * auroras appear dotted and repeatedly darken/lighten the cached strokes. */
    if(count>0){DrawCircleV(p[0],width/2,color);DrawCircleV(p[count-1],width/2,color);}
}
void WCurve(Vector2 a,Vector2 b,Vector2 c,Vector2 d,Color color,float width)
{ DrawSplineSegmentBezierCubic(a,b,c,d,width,color); }
void WCloud(float x,float y,float s,Color c)
{
    WOval(x,y,58*s,19*s,c,0);WOval(x-25*s,y-8*s,26*s,24*s,c,0);
    WOval(x+9*s,y-20*s,30*s,30*s,c,0);WOval(x+38*s,y-4*s,26*s,19*s,c,0);
}
void WCrystal(float x,float y,float s,Color c)
{
    WP(c,{x,y},{x-s*.4f,y-s*.65f},{x-s*.22f,y-s*1.6f},{x+s*.12f,y-s*2},{x+s*.45f,y-s*.6f});
    WP(Fade(WHITE,.21f),{x,y},{x+s*.12f,y-s*2},{x+s*.45f,y-s*.6f});
    WL(Fade(WHITE,.4f),1,{x,y-s*.2f},{x+s*.12f,y-s*1.8f});
}
