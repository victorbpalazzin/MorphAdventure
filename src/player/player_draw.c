#include "player_internal.h"
#include "raylib.h"
#include <math.h>

/* Original vector coordinates/palettes from MorphAdventure.html::drawPlayer.
 * CPU transforms keep draw pure and do not leak rlgl matrix/global state. */
typedef struct Art { float a,b,c,d,x,y,opacity,scale; } Art;
static Vector2 Point(Art t, Vector2 p)
{ return (Vector2){t.a*p.x+t.c*p.y+t.x, t.b*p.x+t.d*p.y+t.y}; }
static Art Rotate(Art t, float r)
{
    float co=cosf(r), si=sinf(r), a=t.a, b=t.b;
    t.a=a*co+t.c*si; t.b=b*co+t.d*si;
    t.c=t.c*co-a*si; t.d=t.d*co-b*si;
    return t;
}
static Color Ink(unsigned rgb, float opacity)
{ Color c=GetColor((rgb<<8)|255u); c.a=(unsigned char)(255*fmaxf(0,fminf(1,opacity))); return c; }
static float Cross(Vector2 a, Vector2 b, Vector2 c)
{ return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x); }
static void Triangle(Art t, Vector2 a, Vector2 b, Vector2 c, unsigned rgb)
{
    a=Point(t,a); b=Point(t,b); c=Point(t,c);
    /* raylib triangles use counter-clockwise winding in screen coordinates. */
    if (Cross(a,b,c)>0) { Vector2 swap=b; b=c; c=swap; }
    DrawTriangle(a,b,c,Ink(rgb,t.opacity));
}
static void Line(Art t, const Vector2 *p, int n, unsigned rgb, float width, bool close)
{
    Color color=Ink(rgb,t.opacity);
    float w=width*t.scale;
    if (w<=0) return;
    for (int i=0;i<n;i++) {
        Vector2 a=Point(t,p[i]);
        if(i+1<n || close) DrawLineEx(a,Point(t,p[(i+1)%n]),w,color);
        DrawCircleV(a,w*0.5f,color);
    }
}
static bool InTriangle(Vector2 p, Vector2 a, Vector2 b, Vector2 c, float winding)
{ return Cross(a,b,p)*winding>=0 && Cross(b,c,p)*winding>=0 && Cross(c,a,p)*winding>=0; }
static void Polygon(Art t, const Vector2 *p, int n, unsigned fill, unsigned stroke, float width)
{
    /* Ear clipping: fox ears/tails are concave, so a triangle fan is incorrect. */
    int indices[16];
    if (n<3 || n>16) return;
    float area=0;
    for(int i=0;i<n;i++) { indices[i]=i;area+=p[i].x*p[(i+1)%n].y-p[(i+1)%n].x*p[i].y; }
    float winding=area>=0?1.0f:-1.0f;
    int left=n;
    while(left>2) {
        bool clipped=false;
        for(int k=0;k<left;k++) {
            int prev=(k+left-1)%left, next=(k+1)%left;
            Vector2 a=p[indices[prev]],b=p[indices[k]],c=p[indices[next]];
            if(Cross(a,b,c)*winding<=0) continue;
            bool occupied=false;
            for(int j=0;j<left;j++)
                if(j!=prev && j!=k && j!=next && InTriangle(p[indices[j]],a,b,c,winding)) occupied=true;
            if(occupied) continue;
            Triangle(t,a,b,c,fill);
            for(int j=k;j+1<left;j++) indices[j]=indices[j+1];
            left--;clipped=true;break;
        }
        if(!clipped) break; /* All shipped shapes are simple, nondegenerate. */
    }
    if(width>0) Line(t,p,n,stroke,width,true);
}
#define POLY(t,fill,stroke,width,...) Polygon(t,(Vector2[]){__VA_ARGS__},(int)(sizeof((Vector2[]){__VA_ARGS__})/sizeof(Vector2)),fill,stroke,width)
#define LINE(t,color,width,...) Line(t,(Vector2[]){__VA_ARGS__},(int)(sizeof((Vector2[]){__VA_ARGS__})/sizeof(Vector2)),color,width,false)
static void Ellipse(Art t,float x,float y,float rx,float ry,unsigned color,float rotation)
{
    float co=cosf(rotation),si=sinf(rotation);
    Vector2 center={x,y}, previous={x+rx*co,y+rx*si};
    for(int i=1;i<=48;i++) {
        float a=(float)i*(2*PI/48),px=cosf(a)*rx,py=sinf(a)*ry;
        Vector2 next={x+px*co-py*si,y+px*si+py*co};
        Triangle(t,center,previous,next,color);previous=next;
    }
}
static void Ring(Art t,float radius,unsigned color,float width)
{
    Vector2 points[96];
    for(int i=0;i<96;i++) points[i]=(Vector2){cosf(i*2*PI/96)*radius,sinf(i*2*PI/96)*radius};
    Line(t,points,96,color,width,true);
}

static void DrawArt(const PlayerState *player,const GameState *game,const ItemsView *items,int mood)
{
    const PlayerView *v=&player->view;
    if(v->scale<=0) return;
    Art t={v->scale,0,0,v->scale,v->position.x,v->position.y,1,v->scale};
    /* simulation_steps advances only during active modes, never in draw/pause. */
    double steps=(double)game->simulation_steps;
    float wing=(float)sin(steps*0.23)*0.6f;
    bool magnet=false,phase=false;
    for(unsigned i=0;i<items->boost_count && i<MORPH_MAX_BOOSTS;i++){
        if(items->boosts[i].type==BOOST_MAGNET) magnet=true;
        if(items->boosts[i].type==BOOST_PHASE) phase=true;
    }
    if(phase){Ring(t,29+(float)sin(steps*.12)*3,0x91efff,2);t.opacity=.45f;}
    if(items->shield || magnet)
        Ring(t,items->shield?33:40+(float)sin(steps*0.1)*4,items->shield?0xb0f1ff:0xffb2e0,3);
    t=Rotate(t,v->rotation);
    if(v->invulnerable && (int)floorf(player->invulnerability_steps/7)%2==0) t.opacity=0.35f;
    switch(game->world) {
    case 0:
        POLY(t,0x2d558b,0x18385d,2,{-14,8},{-32,1},{-29,17},{-8,13});
        Ellipse(t,0,1,23,18,0xf5c55d,0);Ellipse(t,5,8,16,11,0xffecac,0);
        Ellipse(t,-7,3,15,7,0x377eac,wing-0.3f);
        POLY(t,0xef9252,0x894c3c,1,{18,-2},{32,3},{19,7});
        Ellipse(t,10,-8,11,10,0xf5d271,0);break;
    case 1:
        POLY(t,0xf4b879,0x944e56,2,{-14,0},{-31,-15-wing*6},{-28,15+wing*6});
        Ellipse(t,0,0,24,17,0xf7aa69,0);Ellipse(t,4,6,16,9,0xffe1ab,0);
        POLY(t,0x71d2d0,0x277f96,2,{-8,-13},{1,-25},{9,-14});
        Ellipse(t,-3,3,11,5,0xd96567,wing);LINE(t,0xfff1b7,4,{0,-14},{-5,10});break;
    case 2:
        for(int side=-1;side<=1;side+=2) {
            Art mirror=t;mirror.a*=side;mirror.b*=side;
            Ellipse(mirror,-13,-8,16,21,0x7f91e2,-0.45f-wing*0.2f);
            Ellipse(mirror,-12,13,12,13,0xe8a8d6,0.4f);
            Ellipse(mirror,-15,-8,8,12,0xbedce9,-0.4f);Ellipse(mirror,-15,14,5,5,0xfff4c5,0);
        }
        Ellipse(t,0,2,5,21,0x2b3559,0);Ellipse(t,1,-14,7,7,0xe8ca94,0);
        LINE(t,0x354267,2,{-2,-17},{-9,-25});LINE(t,0x354267,2,{3,-17},{8,-25});break;
    case 3:
        Ellipse(t,0,2,20,24,0x203754,0);Ellipse(t,5,8,14,17,0xeff9f2,0);
        Ellipse(t,-18,3,6,17,0x345777,wing-0.4f);Ellipse(t,7,-12,10,10,0xf2f4eb,0);
        POLY(t,0xefb866,0,0,{17,-9},{29,-4},{17,0});
        Ellipse(t,-7,24,9,4,0xe8aa62,0);Ellipse(t,12,24,8,4,0xe8aa62,0);
        LINE(t,0xcc6e79,6,{-15,-1},{14,3});break;
    case 4: {
        Art tail=Rotate(t,wing*0.25f);
        POLY(tail,0xdf8c56,0x7d4b4c,2,{-12,4},{-37,-9},{-30,18},{-11,18});
        POLY(tail,0xfff0d6,0,0,{-37,-9},{-33,8},{-22,0});
        Ellipse(t,-2,6,21,13,0xd98c54,0);
        POLY(t,0xe8a367,0x754b4b,2,{-8,-5},{-13,-27},{4,-17},{13,-26},{21,-5},{13,14});
        POLY(t,0xfff0d3,0,0,{1,0},{23,2},{10,16},{-1,8});
        Ellipse(t,-5,2,15,5,0xb0d7e4,wing-0.4f);break;
    }
    case 5:
        POLY(t,0x6bb4a2,0x284b54,2,{-16,6},{-35,18},{-27,0},{-17,-1});
        Ellipse(t,0,3,23,16,0x7bc3a8,0);Ellipse(t,4,9,15,8,0xf8d99b,0);
        POLY(t,0xdf997d,0x815267,2,{-12,0},{-26,-24-wing*8},{-8,-17},{2,-3});
        Ellipse(t,13,-8,14,12,0x8cd4b6,0);
        POLY(t,0xffe6b1,0x73716d,1,{6,-17},{5,-29},{13,-18},{18,-27},{21,-16});
        Ellipse(t,25,-3,8,6,0xa4ddbb,0);break;
    default: return;
    }
    float eyeX=game->world==2?3:game->world==5?17:12;
    float eyeY=game->world==2?-15:game->world==5?-10:game->world==4?-5:-7;
    if(mood==1){
        Ellipse(t,eyeX,eyeY,5,6,0xfffaf0,0);Ellipse(t,eyeX+1,eyeY,2.4f,3.5f,0x142b42,0);
        LINE(t,0x23324a,2,{eyeX-5,eyeY-9},{eyeX+4,eyeY-6});
        Ellipse(t,eyeX-3,eyeY+12,2.5f,3.5f,0x23324a,0);
    }else if(mood==2){
        LINE(t,0x23324a,2.5f,{eyeX-4,eyeY+1},{eyeX,eyeY-3},{eyeX+4,eyeY+1});
        LINE(t,0x23324a,2,{eyeX-7,eyeY+10},{eyeX-3,eyeY+13},{eyeX+1,eyeY+10});
        Ellipse(t,eyeX-5,eyeY+6,4,2,0xef9eae,0);
    }else{Ellipse(t,eyeX,eyeY,3,4,0x142b42,0);Ellipse(t,eyeX+1,eyeY-1,1,1.4f,0xfffaf0,0);}
    if(player->debug_hitbox) {
        /* Collision remains unscaled in the contract. Portal is non-damaging. */
        DrawCircleLinesV((Vector2){v->position.x,v->position.y},v->collision_radius,LIME);
        DrawCircleLinesV((Vector2){v->position.x,v->position.y},v->visual_radius,SKYBLUE);
    }
}

void PlayerDrawArt(const PlayerState *player,const GameState *game,const ItemsView *items)
{DrawArt(player,game,items,0);}
void PlayerDrawPortrait(int world,GameVec2 position,float scale,float seconds,bool happy)
{
    PlayerState p={0};p.view.position=position;p.view.scale=scale;
    p.view.rotation=sinf(seconds*2+world)*.045f;
    GameState g={0};g.world=world;g.simulation_steps=(uint64_t)(seconds*60);
    ItemsView items={0};DrawArt(&p,&g,&items,happy?2:1);
}
