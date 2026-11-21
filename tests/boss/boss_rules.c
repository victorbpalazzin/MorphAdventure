#include "boss/boss.h"
#include "items/combat_energy.h"
#include "morph_check.h"
#include <math.h>
#include <string.h>
static FrameContext Frame(float scale)
{return (FrameContext){.normal_steps=1,.world_steps=scale,.normal_seconds=1.0f/60,.world_seconds=scale/60,.ui_seconds=1.0f/60,.world_scale=scale};}
static PlayerView Player(void)
{return (PlayerView){.position={180,245},.collision_radius=18,.visual_radius=23,.scale=1};}
static void Energy(void)
{
    CombatEnergy e;CombatEnergyReset(&e,4);FrameContext f=Frame(1);PlayerView p=Player();p.position.x=-1000;
    for(int i=0;i<4000;i++)MORPH_CHECK(!CombatEnergyStep(&e,&f,p,false));
    MORPH_CHECK(e.serial>10 && e.collected==0); /* unlimited missed replacements */
    for(unsigned n=0;n<4;n++){
        while(!e.present)CombatEnergyStep(&e,&f,p,false);
        p.position=e.position;MORPH_CHECK(CombatEnergyStep(&e,&f,p,false));
        MORPH_CHECK(e.collected==n+1);MORPH_CHECK(!CombatEnergyStep(&e,&f,p,false));
        MORPH_CHECK(n==3?CombatEnergyFraction(&e)==1:CombatEnergyFraction(&e)<1);
    }
    for(int i=0;i<500;i++)MORPH_CHECK(!CombatEnergyStep(&e,&f,p,false));
    MORPH_CHECK(e.collected==4);
    CombatEnergyReset(&e,7);MORPH_CHECK(e.required==7 && !e.present && e.collected==0);
}
static void Combat(void)
{
    BossState b;BossBegin(&b,4);FrameContext f=Frame(1);PlayerView p=Player();
    MORPH_CHECK(!BossFire(&b,p));
    for(int i=0;i<180;i++){BossStep(&b,&f,p);MORPH_CHECK(b.projectile_count==0);}
    MORPH_CHECK(b.stage==BOSS_FIGHT);
    for(unsigned phase=1;phase<=3;phase++){
        MORPH_CHECK(b.phase==phase);
        for(int target=42;target<=487;target+=5){
            b.target_y=(float)target;
            for(unsigned j=0;j<phase;j++){
                float y=BossTargetY(&b,j);MORPH_CHECK(y>=185 && y<=420);
                if(j)MORPH_CHECK_NEAR(y-BossTargetY(&b,j-1),110,.001f);
            }
        }
        b.attack_timer=1;BossStep(&b,&f,p);
        MORPH_CHECK(b.warning==65 && !b.projectile_count);
        for(int i=0;i<64;i++){BossStep(&b,&f,p);MORPH_CHECK(!b.projectile_count);}
        BossStep(&b,&f,p);
        if(phase==3){MORPH_CHECK(b.projectile_count==1);for(int j=0;j<36;j++)BossStep(&b,&f,p);}
        MORPH_CHECK(b.projectile_count==phase);
        PlayerView at=p;at.position=b.projectiles[0].position;MORPH_CHECK(BossTouches(&b,at));
        at.position=(GameVec2){180,450};MORPH_CHECK(!BossTouches(&b,at));
        float speed=hypotf(b.projectiles[0].velocity.x,b.projectiles[0].velocity.y);
        MORPH_CHECK_NEAR(speed,phase==1?3.8f:phase==2?4.8f:5.8f,.0001f);
        MORPH_CHECK(!BossFire(&b,p));b.energy.collected=3;MORPH_CHECK(!BossFire(&b,p));
        b.energy.collected=4;MORPH_CHECK(BossFire(&b,p));MORPH_CHECK(!BossFire(&b,p));
        for(int i=0;i<34;i++){BossStep(&b,&f,p);MORPH_CHECK(b.segments==4-phase);}
        BossStep(&b,&f,p);MORPH_CHECK(b.segments==3-phase);
        for(int i=0;i<50;i++){BossStep(&b,&f,p);MORPH_CHECK(b.segments==3-phase);MORPH_CHECK(!BossFire(&b,p));}
        MORPH_CHECK(b.energy.collected==0 && !b.beam_active);
    }
    MORPH_CHECK(b.stage==BOSS_DYING && !b.projectile_count);
    for(int i=0;i<159;i++)BossStep(&b,&f,p);
    MORPH_CHECK(b.stage==BOSS_DYING);BossStep(&b,&f,p);MORPH_CHECK(b.stage==BOSS_DONE);
}
static void Clock(void)
{
    BossState a,b;BossBegin(&a,4);a.stage=BOSS_FIGHT;a.attack_timer=10000;
    a.projectile_count=1;a.projectiles[0]=(BossProjectile){.position={600,90},.previous={600,90},.velocity={-3,0},.radius=12};b=a;
    FrameContext normal=Frame(1),slow=Frame(.68f);PlayerView p=Player();
    for(int i=0;i<68;i++)BossStep(&a,&normal,p);
    for(int i=0;i<100;i++)BossStep(&b,&slow,p);
    MORPH_CHECK_NEAR(a.projectiles[0].position.x,b.projectiles[0].position.x,.005f);
    MORPH_CHECK_NEAR(a.attack_timer,b.attack_timer,.05f);
    MORPH_CHECK(a.clock==68 && b.clock==100);
}
static void Lifetime(void)
{
    BossState b;BossBegin(&b,4);b.stage=BOSS_FIGHT;b.attack_timer=10000;
    b.energy.present=true;b.energy.position=(GameVec2){500,245};
    b.projectile_count=1;b.projectiles[0]=(BossProjectile){.position={741,245},.previous={741,245},.velocity={-8,0},.radius=12};
    FrameContext f=Frame(1);PlayerView p=Player();bool hit=false,passed=false;
    while(b.projectile_count){
        float old=b.projectiles[0].position.x;BossStep(&b,&f,p);
        if(!b.projectile_count){MORPH_CHECK(old+12-8<0);break;}
        MORPH_CHECK(b.projectiles[0].position.x<old);
        if(BossTouches(&b,p)){hit=true;BossRegisterContact(&b,p);MORPH_CHECK(!BossTouches(&b,p));}
        if(b.projectiles[0].position.x<180)passed=true;
    }
    MORPH_CHECK(hit && passed);
    b.projectile_count=1;b.projectiles[0]=(BossProjectile){.position={220,245},.previous={220,245},.velocity={-100,0},.radius=12};
    BossStep(&b,&f,p);MORPH_CHECK(BossTouches(&b,p)); /* endpoint alone misses */
    p.position.y=400;MORPH_CHECK(!BossTouches(&b,p));
    while(b.projectile_count)BossStep(&b,&f,p);
}
int main(void){Energy();Combat();Clock();Lifetime();puts("PASS boss: reoffers, configurable energy, warnings/collisions, 3 phases, exactly 3 hits, delayed defeat and scaled projectile time");return 0;}
