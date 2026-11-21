/* Visual fixture only: controlled arrangements, no fake production HUD. */
#include "items/items_internal.h"
#include "raylib.h"
#include "morph_check.h"
#include <string.h>
int main(int argc,char **argv)
{
    MORPH_CHECK(argc==2);SetConfigFlags(FLAG_WINDOW_HIDDEN);InitWindow(960,540,"Itens - validacao visual");
    MORPH_CHECK(IsWindowReady());RenderTexture2D target=LoadRenderTexture(960,540);
    ItemsApi api=ItemsModuleCreate();MORPH_CHECK(api.life.init(api.life.context));ItemsState *s=api.life.context;
    GameState game={.mode=GAME_PLAY};WorldConfig cfg={.duration_steps=1500};api.life.reset_world(s,&game,&cfg);
    s->fragment_count=4;for(int i=0;i<4;i++)s->fragments[i]=(ItemFragmentView){(unsigned)i,{200+i*180.0f,190},23,.5f*i,i?i*5.0f:0};
    s->power_count=6;for(int i=0;i<6;i++)s->powers[i]=(ItemPowerView){(unsigned)i,(BoostType)i,{110+i*145.0f,350},21};
    ItemsState before=*s;BeginTextureMode(target);ClearBackground(GetColor(0x10192eff));
    DrawText("PESSOA 5 - COLETAVEIS VETORIAIS",36,35,24,WHITE);
    DrawText("Fragmento de energia e desaparecimento apos coleta",36,100,18,LIGHTGRAY);
    const char *labels[]={"Escudo","Ima - 7 s","Slow","x2","Vida +1","Intangivel"};
    for(int i=0;i<6;i++)DrawText(labels[i],110+i*145-MeasureText(labels[i],20)/2,420,20,LIGHTGRAY);
    api.draw(s,&game);EndTextureMode();MORPH_CHECK(memcmp(&before,s,sizeof(before))==0);
    Image img=LoadImageFromTexture(target.texture);MORPH_CHECK(IsImageValid(img));ImageFlipVertical(&img);
    char path[2048];snprintf(path,sizeof(path),"%s/itens.png",argv[1]);MORPH_CHECK(ExportImage(img,path));UnloadImage(img);
    api.life.shutdown(s);UnloadRenderTexture(target);CloseWindow();puts("PASS items-render: real draw, pure state, fragment fade and five boost icons and extra-life heart");return 0;
}
