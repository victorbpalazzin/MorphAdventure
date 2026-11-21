#include "items_internal.h"
#include "core/game.h"
#include <math.h>
#include <string.h>
static ItemsState state;
static uint32_t reset_seed=505;
static const ItemBoostDefinition definitions[]={
    {BOOST_SHIELD,"Escudo",-1,0x8ddfffff}, {BOOST_MAGNET,"Ima",7,0xffb2e0ff},
    {BOOST_SLOW,"Slow motion",4,0xbcb5ffff}, {BOOST_DOUBLE,"x2",7,0xffe58fff},
    {BOOST_LIFE,"Vida extra",0,0xff8faaff},
    {BOOST_PHASE,"Intangivel",7,0x91efffff}
};
const ItemBoostDefinition *ItemsBoostDefinition(BoostType type)
{
    for(size_t i=0;i<sizeof(definitions)/sizeof(*definitions);i++)if(definitions[i].type==type)return &definitions[i];
    return NULL;
}
static int Active(const ItemsState *s,BoostType type)
{for(unsigned i=0;i<s->boost_count;i++)if(s->boosts[i].type==type)return (int)i;return -1;}
static void RemoveBoost(ItemsState *s,unsigned index)
{for(unsigned i=index;i+1<s->boost_count;i++)s->boosts[i]=s->boosts[i+1];s->boost_count--;}
static void Activate(ItemsState *s,BoostType type)
{
    const ItemBoostDefinition *def=ItemsBoostDefinition(type);if(!def)return;
    int found=Active(s,type);
    if(found>=0){if(type!=BOOST_SHIELD)s->boosts[found].seconds=def->duration_seconds;return;}
    if(s->boost_count==MORPH_MAX_BOOSTS)RemoveBoost(s,0);
    s->boosts[s->boost_count++]=(ActiveItemBoost){type,def->duration_seconds};
}
static uint32_t MixSeed(uint32_t x)
{x^=x>>16;x*=0x7feb352du;x^=x>>15;x*=0x846ca68bu;return x^(x>>16);}
static float BoostRandom(ItemsState *s)
{s->boost_seed=s->boost_seed*1664525u+1013904223u;return (float)((s->boost_seed>>8)/16777216.0);}
static float Random(ItemsState *s)
{s->seed=s->seed*1664525u+1013904223u;return (float)((s->seed>>8)/16777216.0);}
static bool Init(void *ctx){memset(ctx,0,sizeof(ItemsState));return true;}
static void Shutdown(void *ctx){memset(ctx,0,sizeof(ItemsState));}
static void Reset(void *ctx,const GameState *g,const WorldConfig *cfg)
{
    ItemsState *s=ctx;memset(s,0,sizeof(*s));s->world=g->world;s->required=GameEnergyRequired(g->world);
    s->valid=s->required>0 && cfg && isfinite(cfg->duration_steps) && cfg->duration_steps>0;
    s->seed=MixSeed(reset_seed ^ (0x9e3779b9u*(unsigned)(g->world+1)) ^ MixSeed(g->run_id));
    s->boost_seed=MixSeed(s->seed^0xa511e9b3u);
    s->fragment_timer=180;s->boost_timer=360+BoostRandom(s)*320;
    if(s->valid)s->fragment_interval=fminf(180,fmaxf(125,cfg->duration_steps/(s->required+1))); /* fewer goals must not lengthen the wait for a replacement */
}
static void Timers(void *ctx,const FrameContext *f)
{
    ItemsState *s=ctx;if(!isfinite(f->normal_seconds)||f->normal_seconds<=0)return;
    for(unsigned i=0;i<s->boost_count;){
        if(s->boosts[i].type==BOOST_SHIELD){i++;continue;}
        s->boosts[i].seconds-=f->normal_seconds;
        if(s->boosts[i].seconds<=0.000001)RemoveBoost(s,i);else i++;
    }
}
static ItemsView View(void *ctx)
{
    const ItemsState *s=ctx;ItemsView v={0};v.collected=s->collected;v.boost_count=s->boost_count;
    v.shield=Active(s,BOOST_SHIELD)>=0;v.score_multiplier=Active(s,BOOST_DOUBLE)>=0?2:1;
    v.world_scale=Active(s,BOOST_SLOW)>=0?.68f:1;
    for(unsigned i=0;i<s->boost_count;i++)v.boosts[i]=(BoostView){s->boosts[i].type,(float)s->boosts[i].seconds};
    return v;
}
static bool ConsumeShield(void *ctx)
{ItemsState *s=ctx;int index=Active(s,BOOST_SHIELD);if(index<0)return false;RemoveBoost(s,(unsigned)index);return true;}
static bool SafePosition(PassageQuery q,float x,float fallback,float radius,GameVec2 *position)
{
    float target=q.safe_y(q.context,x,fallback);if(!isfinite(target))return false;
    *position=(GameVec2){x,target};
    return q.is_safe(q.context,*position,radius);
}
static void SpawnFragment(ItemsState *s,PassageQuery q)
{
    if(s->fragment_count==ITEMS_FRAGMENT_CAPACITY)return;
    for(unsigned id=0;id<s->required;id++){
        if(s->collected_ids[id])continue;
        bool exists=false;for(unsigned i=0;i<s->fragment_count;i++)if(s->fragments[i].id==id)exists=true;
        if(exists)continue;
        GameVec2 p;if(!SafePosition(q,995,140+Random(s)*230,23,&p))return;
        s->fragments[s->fragment_count++]=(ItemFragmentView){id,p,23,0,0};return;
    }
}
static void SpawnPower(ItemsState *s,PassageQuery q)
{
    if(s->power_count==ITEMS_POWER_CAPACITY)return;
    BoostType choices[sizeof(definitions)/sizeof(*definitions)];unsigned count=0;
    for(size_t i=0;i<sizeof(definitions)/sizeof(*definitions);i++)if(definitions[i].type!=BOOST_LIFE && Active(s,definitions[i].type)<0)choices[count++]=definitions[i].type;
    if(!count)return;
    BoostType type=choices[(unsigned)(BoostRandom(s)*count)];GameVec2 p;
    if(!SafePosition(q,1040,140+BoostRandom(s)*230,21,&p))return;
    GameVec2 varied={p.x,p.y+(BoostRandom(s)-.5f)*70};
    if(q.is_safe(q.context,varied,21))p=varied;
    s->powers[s->power_count++]=(ItemPowerView){++s->next_power_id,type,p,21};
}
static void SpawnLife(ItemsState *s,PassageQuery q)
{
    if(s->world%2!=1 || s->life_collected || s->collected<s->required/2 || s->power_count==ITEMS_POWER_CAPACITY)return;
    for(unsigned i=0;i<s->power_count;i++)if(s->powers[i].type==BOOST_LIFE)return;
    GameVec2 p;if(!SafePosition(q,995,245,21,&p))return;
    s->powers[s->power_count++]=(ItemPowerView){++s->next_power_id,BOOST_LIFE,p,21};
}
static bool Touch(GameVec2 p,float radius,PlayerView player)
{float dx=p.x-player.position.x,dy=p.y-player.position.y;float r=radius+player.visual_radius;return dx*dx+dy*dy<r*r;}
static void Magnet(ItemsState *s,ItemFragmentView *item,PlayerView player,float dt,PassageQuery q)
{
    if(Active(s,BOOST_MAGNET)<0)return;
    float dx=player.position.x-item->position.x,dy=player.position.y-item->position.y,d=hypotf(dx,dy);
    if(d<=0 || d>=210)return;
    float move=fminf(d,8*dt);GameVec2 next={item->position.x+dx/d*move,item->position.y+dy/d*move};
    if(q.is_safe(q.context,next,item->radius))item->position=next;
}
static void Collect(void *ctx,const GameState *g,const FrameContext *f,PlayerView player,PassageQuery q,GameEventBuffer *events)
{
    ItemsState *s=ctx;if(g->mode!=GAME_PLAY)return;
    if(!events)return;
    if(!s->valid || !q.safe_y || !q.is_safe || !isfinite(f->world_steps) || f->world_steps<0 || !isfinite(f->scroll_delta) || f->scroll_delta<0){events->overflow=true;return;}
    s->fragment_timer-=f->world_steps;s->boost_timer-=f->world_steps;
    if(s->fragment_timer<=0 && s->collected<s->required){SpawnFragment(s,q);s->fragment_timer=s->fragment_interval;}
    if(s->boost_timer<=0){SpawnPower(s,q);s->boost_timer=500+BoostRandom(s)*440;}
    s->life_timer-=f->world_steps;
    if(s->life_timer<=0){SpawnLife(s,q);s->life_timer=90;}
    unsigned kept=0;
    for(unsigned i=0;i<s->fragment_count;i++){
        ItemFragmentView item=s->fragments[i];item.spin+=.06f*f->normal_steps;
        if(item.fade>0){item.fade+=f->normal_steps;if(item.fade<22)s->fragments[kept++]=item;continue;}
        item.position.x-=f->scroll_delta;
        if(item.position.x>player.position.x+100){
            float y=q.safe_y(q.context,item.position.x,item.position.y);
            GameVec2 next={item.position.x,item.position.y+(y-item.position.y)*fminf(1,.08f*f->normal_steps)};
            if(isfinite(y) && q.is_safe(q.context,next,item.radius))item.position=next;
        }
        Magnet(s,&item,player,f->world_steps,q);
        /* Unsafe moving passages release the ID for a later opportunity instead
         * of carrying a required fragment through a solid obstacle. */
        if(item.position.x<-50 || !q.is_safe(q.context,item.position,item.radius))continue;
        if(!s->collected_ids[item.id] && Touch(item.position,item.radius,player)){
            GameEvent e={0};e.type=EVENT_FRAGMENT_COLLECTED;e.position=item.position;e.entity_id=item.id;e.multiplier=View(s).score_multiplier;
            if(GameEnqueueEvent(events,e)){s->collected_ids[item.id]=true;s->collected++;item.fade=1;}
        }
        s->fragments[kept++]=item;
    }
    s->fragment_count=kept;kept=0;
    /* Reference order: fragments precede power pickups in the same step. */
    for(unsigned i=0;i<s->power_count;i++){
        ItemPowerView item=s->powers[i];item.position.x-=f->scroll_delta;
        if(item.position.x<-50 || !q.is_safe(q.context,item.position,item.radius))continue;
        if(Touch(item.position,item.radius,player)){
            GameEvent e={0};e.type=EVENT_BOOST_COLLECTED;e.position=item.position;e.entity_id=item.id;e.boost=item.type;e.multiplier=View(s).score_multiplier;
            if(GameEnqueueEvent(events,e)){
                if(item.type==BOOST_LIFE)s->life_collected=true;else Activate(s,item.type);
                continue;
            }
        }
        s->powers[kept++]=item;
    }
    s->power_count=kept;
}
static void Draw(void *ctx,const GameState *g){(void)g;ItemsDraw(ctx);}
ItemsApi ItemsModuleCreate(void)
{return (ItemsApi){{&state,Init,Reset,Shutdown},Timers,View,ConsumeShield,Collect,Draw};}
void ItemsSetSeed(uint32_t seed){reset_seed=seed;}
ItemEnergyView ItemsEnergy(void)
{return (ItemEnergyView){state.collected,state.required,GameEnergyFraction(state.collected,state.world),state.required>0&&state.collected==state.required};}
ItemsSnapshot ItemsInspect(void)
{
    ItemsSnapshot v={0};v.world=state.world;v.fragment_count=state.fragment_count;v.power_count=state.power_count;
    v.fragment_timer=state.fragment_timer;v.boost_timer=state.boost_timer;v.configuration_valid=state.valid;
    memcpy(v.fragments,state.fragments,sizeof(v.fragments));memcpy(v.powers,state.powers,sizeof(v.powers));return v;
}
