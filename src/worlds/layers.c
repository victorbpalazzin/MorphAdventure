#include "worlds_internal.h"
#include <math.h>

/* Periodic cell seeds, including copies beyond each edge: the 1920 px cache
 * tiles continuously, without changing random geometry on every frame. */
static float Random(uint32_t *seed)
{ *seed=*seed*1664525u+1013904223u;return (float)((double)*seed/4294967296.0); }
static void Mountain(float x,float h,int depth,bool ice,Color color)
{
    float base=490+depth*18.0f,peak=base-h-(ice?70:0);
    WP(color,{x-110,base},{x+38,peak},{x+210,base});
    WP(GetColor(0x192c6030),{x+38,peak},{x+75,base},{x+210,base});
    if(ice || depth==0)
        WP(ice?WC(0xf2f6fa):WC(0xd8e9df),{x+38,peak},{x-1,peak+70},{x+24,peak+57},{x+43,peak+80},{x+62,peak+62},{x+91,peak+74});
    for(int r=0;r<4;r++) {
        float rx=x+38+r*16;
        WL(GetColor(0xeef9ff12),2,{rx,peak+35+r*18},{rx-14,peak+80+r*18},{rx+10,fminf(base,peak+155+r*10)});
    }
}
static void Coral(float x,float y,float scale,Color color)
{
    for(int j=-2;j<=2;j++) {
        float tip=y-(47-fabsf((float)j)*8)*scale;
        WL(color,5*scale,{x,y},{x+j*8*scale,y-22*scale},{x+j*13*scale,tip});
        WL(color,3*scale,{x+j*9*scale,y-25*scale},{x+j*18*scale,y-32*scale});
    }
    WOval(x,y,21*scale,5*scale,color,0);
}
static void Dune(float x,float top,Color color)
{
    Vector2 points[36];points[0]=(Vector2){x-120,540};
    for(int i=0;i<=32;i++) {
        float u=i/32.0f;
        points[i+1]=(Vector2){x-120+350*u,540-sinf(u*PI)*(540-top)};
    }
    points[34]=(Vector2){x+230,540};WPoly(points,35,color);
}
void WorldBuildLayer(int world,int depth)
{
    const WorldPalette *palette=WorldsPalette(world);Color color=GetColor(palette->layers[depth]);
    for(int cell=-2;cell<14;cell++) {
        int index=(cell%12+12)%12;
        uint32_t seed=907u+(unsigned)world*101u+(unsigned)depth*37u+(unsigned)index*7919u;
        float x=cell*160.0f,h=90+Random(&seed)*130+depth*20;
        if(world==0 || world==3) {
            Mountain(x,h,depth,world==3,color);
            if(world==0 && depth==1) WCloud(x+70,410+Random(&seed)*50,.65f,GetColor(0xcfe5e777));
            if(world==3 && depth==2) {
                WCrystal(x+22,540,45,WC(0xb6ebed));
                if(index%4==1) {
                    WOval(x+80,535,90,63,WC(0x688caf),0);
                    WOval(x+80,535,57,42,WC(0x3a5e88),0);
                    for(int k=0;k<5;k++)
                        WP(WC(0xa9d8e7),{x+35+k*18,496},{x+45+k*18,496},{x+40+k*18,512+k%2*12});
                }
            }
        } else if(world==1) {
            float top=430-depth*18-Random(&seed)*75;
            WP(color,{x-80,560},{x-56,top+43},{x-20,top},{x+35,top+12},{x+68,top+60},{x+100,560});
            for(int j=0;j<4;j++) {
                float px=x+j*15,ph=30+Random(&seed)*90;
                WCurve((Vector2){px,530},(Vector2){px-25,510-ph/2},(Vector2){px+26,530-ph},(Vector2){px,530-ph},
                       depth==2?WC(0x268b87):WC(0x1d7784),5+depth*2);
            }
            if(depth==2) {
                Coral(x+50,522,1,index%2?WC(0xc07992):WC(0xdfaa85));
                for(int j=0;j<5;j++) {
                    WOval(x-30+j*8,518-j%2*12,6,12,WC(0x447f9c),0);
                    WOval(x-30+j*8,506-j%2*12,6,3,WC(0x9dc2cc),0);
                }
                for(int j=0;j<7;j++)WOval(x+20+j*11,531,1.5f,1,GetColor(0xbcd7c544),0);
            }
        } else if(world==2) {
            float trunk=depth==0?16:26,tip=50+Random(&seed)*95;
            WP(color,{x-trunk,540},{x-7,tip},{x+8,tip},{x+trunk+8,540});
            WL(color,14+depth*3,{x,340},{x-60,210},{x-90,185});
            WL(color,12,{x+5,290},{x+69,162});WL(GetColor(0xd9f0b514),3,{x-9,510},{x-3,170});
            for(int j=-2;j<=2;j++) {
                float cy=tip+fabsf((float)j)*12;
                WOval(x+j*44,cy,69,48,depth==2?WC(0x245746):color,0);
                if(depth==2)for(int k=0;k<7;k++)WOval(x+j*44-40+k*13,tip+12+sinf((float)k)*13,9,3,GetColor(0x8aa96720),0);
            }
            if(depth==2) {
                for(int j=0;j<7;j++)WP(WC(0x1a5145),{x+j*15,540},{x+j*15-20,495-Random(&seed)*30},{x+j*15+8,517},{x+j*15+23,495},{x+j*15+12,540});
                WCurve((Vector2){x+35,45},(Vector2){x+12,105},(Vector2){x+70,110},(Vector2){x+37,190},WC(0x759a63),2);
                for(int k=0;k<5;k++)WOval(x+40+(k%2?6:-4),88+k*18,7,3,WC(0x69895a),0);
                for(int k=0;k<3;k++) {
                    float fx=x+45+k*14;WL(WC(0x4b7c58),2,{fx,534},{fx-5,487-k%2*7});
                    for(int j=0;j<4;j++) {
                        WL(WC(0x709460),2,{fx-3,503+j*7},{fx-12,497+j*7});
                        WL(WC(0x709460),2,{fx-3,503+j*7},{fx+6,498+j*7});
                    }
                }
            }
        } else if(world==4) {
            if(depth<2) {
                float top=290+depth*45+Random(&seed)*40;
                WP(color,{x-100,540},{x-70,top+80},{x-40,top},{x+47,top},{x+68,top+55},{x+120,540});
                for(int k=0;k<4;k++)WL(GetColor(0xf9cd9620),4,{x-30,top+15+k*22},{x+45,top+20+k*22});
            }
            if(depth==1 && index%3==0) {
                DrawRectangle((int)x,320,20,140,WC(0x825555));DrawRectangle((int)x+76,320,20,140,WC(0x825555));
                DrawRectangle((int)x-5,310,106,18,WC(0x825555));
                WP(WC(0x9b655a),{x-10,310},{x+46,280},{x+109,310});
                for(int k=0;k<3;k++)WL(GetColor(0xfbd5a033),1,{x+4,338+k*28},{x+15,340+k*28});
            }
            if(depth==2) {
                Dune(x,455+Random(&seed)*25,color);
                WL(GetColor(0xfbe2ad55),2,{x-18,478},{x+32,466},{x+80,475});
                for(int k=0;k<5;k++)WCurve((Vector2){x-15,487+k*7},(Vector2){x+15,472+k*8},
                    (Vector2){x+65,478+k*9},(Vector2){x+106,496+k*6},GetColor(0xf8d69825),1);
            }
        } else {
            float top=230+depth*60+Random(&seed)*35;
            WP(color,{x-130,530},{x+18,top},{x+64,top+4},{x+230,530});
            WP(GetColor(0xfa8c6570),{x+18,top},{x+42,top+20},{x+64,top+4},{x+98,top+67},{x+55,top+46},{x+29,top+52});
            if(depth==0) {
                WL(GetColor(0xff9a6544),5,{x+41,top+18},{x+30,top+68},{x+70,top+133},{x+47,530});
                for(int k=0;k<3;k++)WOval(x+40-k*13,top-20-k*28,22+k*9,21+k*4,GetColor(0x88566944),0);
            }
            if(depth==2) {
                WP(WC(0x352133),{x-60,540},{x-42,463},{x-10,438},{x+25,457},{x+49,540});
                WL(GetColor(0xff8c5c66),2,{x-10,440},{x+2,482},{x-13,510});
                WL(GetColor(0xff8c5c44),1,{x+2,482},{x+25,490},{x+32,516});
            }
        }
    }
}
