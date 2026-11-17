#include "player/collision.h"
#include "morph_check.h"
#include <float.h>

int main(void)
{
    GameRect r={10,10,20,20};
    MORPH_CHECK(CollisionCircleRect((GameVec2){20,20},1,r));
    MORPH_CHECK(!CollisionCircleRect((GameVec2){5,20},5,r)); /* tangency */
    MORPH_CHECK(CollisionCircleRect((GameVec2){5.01f,20},5,r));
    MORPH_CHECK(!CollisionCircleRect((GameVec2){6,7},5,r)); /* corner: 3-4-5 */
    MORPH_CHECK(CollisionCircleRect((GameVec2){6,7},5.01f,r));
    MORPH_CHECK(!CollisionCircleRect((GameVec2){6,7},4.99f,r));
    MORPH_CHECK(!CollisionCircleRect((GameVec2){20,20},0,r));
    MORPH_CHECK(!CollisionCircleRect((GameVec2){20,20},-1,r));
    MORPH_CHECK(!CollisionCircleRect((GameVec2){NAN,0},1,r));
    MORPH_CHECK(!CollisionCircleRect((GameVec2){20,20},INFINITY,r));
    MORPH_CHECK(!CollisionCircleRect((GameVec2){20,20},1,(GameRect){0,0,-1,5}));
    const GameVec2 square[]={{10,10},{30,10},{30,30},{10,30}};
    const GameVec2 reverse[]={{10,30},{30,30},{30,10},{10,10}};
    for(int y=-5;y<=45;y++) for(int x=-5;x<=45;x++) {
        GameVec2 p={(float)x,(float)y};
        bool expected=CollisionCircleRect(p,3,r);
        MORPH_CHECK(CollisionCirclePolygon(p,3,square,4)==expected);
        MORPH_CHECK(CollisionCirclePolygon(p,3,reverse,4)==expected);
    }
    /* Concave L: bounding rectangle would wrongly collide in the empty notch. */
    const GameVec2 concave[]={{0,0},{30,0},{30,10},{10,10},{10,30},{0,30}};
    MORPH_CHECK(!CollisionCirclePolygon((GameVec2){20,20},2,concave,6));
    MORPH_CHECK(CollisionCirclePolygon((GameVec2){5,25},1,concave,6));
    MORPH_CHECK(CollisionCirclePolygon((GameVec2){11,11},2,concave,6));
    const GameVec2 repeated[]={{10,10},{30,10},{30,10},{30,30},{10,30},{10,10}};
    MORPH_CHECK(CollisionCirclePolygon((GameVec2){20,20},1,repeated,6));
    MORPH_CHECK(CollisionCirclePolygon((GameVec2){10,20},1,square,4));
    MORPH_CHECK(!CollisionCirclePolygon((GameVec2){5,20},5,square,4));
    MORPH_CHECK(!CollisionCirclePolygon((GameVec2){20,20},1,NULL,4));
    MORPH_CHECK(!CollisionCirclePolygon((GameVec2){20,20},1,square,2));
    const GameVec2 bad[]={{0,0},{10,0},{INFINITY,10}};
    MORPH_CHECK(!CollisionCirclePolygon((GameVec2){0,0},1,bad,3));
    /* Large finite values must not overflow float intermediates into false hits. */
    MORPH_CHECK(!CollisionCircleRect((GameVec2){-FLT_MAX,0},1,(GameRect){FLT_MAX,0,1,1}));
    puts("PASS player-collisions: edges, corners, winding, concavity, repeated vertices, invalid inputs.");
    return 0;
}
