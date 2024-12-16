#ifndef _JC_SDL_PONG

#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include <SDL3/SDL.h>

namespace SdlPong {

enum Side {
    left,
    right,
};

enum BarDirection {
    up,
    down,
    none
};

enum Id {
    ball,
    leftBar,
    rightBar,
    topWall,
    bottomWall,
    leftWall,
    rightWall,
};

struct GraphicBox {
    SDL_Rect rect;
    SDL_Color color;
    // doubles as collision box
};

struct RigidBody {
    int xvel;
    int yvel;
    // no mass
};

/*using Transform = struct {*/
/*    SDL_Point pos; // x and y*/
/*    // no rotation or scale*/
/*};*/

class Body {
  public:
    Body(GraphicBox gb, RigidBody rb, Id id);
    ~Body();

    void UpdatePos();
    SDL_Rect *GetCollisionBox();
    void SetVel(RigidBody rb);
    RigidBody GetVel();
    void Render(SDL_Renderer *renderer);
    Id getId();
    void RegisterCollision(Body *body);
    void HandleCollision();
    void Reset();

  private:
    /*Transform mTransform;   // pos*/
    GraphicBox mGraphicBox; // rect, color
    RigidBody mRigidBody;   // vel
    Id mId;
    SDL_Rect mPostCollisionPos {};
    int mPostCollisionXvel {};
    int mPostCollisionYvel {};
    bool mCollided {false};
    GraphicBox mInitGraphicBox;
    RigidBody mInitRigidBody;
};

/*struct Collision {*/
/*    Body *b1{nullptr};*/
/*    Body *b2{nullptr};*/
/*};*/

class AppState {

  public:
    AppState(int screenWidth, int screenHeight);

    void incScore(Side side);
    void decScore(Side side);
    void moveBar(Side side, BarDirection dir);

    SDL_Window *getWindow();
    SDL_Renderer *getRenderer();

    void UpdatePositions();
    void CheckCollisions();
    void ProcessCollisions();
    void Render();

    ~AppState();

    SDL_Window *mWindow;
    SDL_Renderer *mRenderer;

  private:
    // Outer padding for collision boxes
    static constexpr int kPadding{1};

    static constexpr int kNumBars{2};
    static constexpr int kNumWalls{4};
    static constexpr int kNumTBWalls{2};
    static constexpr int kNumSideWalls{2};

    int barVel;

    int mLeftScore;
    int mRightScore;


    /*std::queue<Collision> mCollisions{};*/

    Body *mBall;
    Body *mLeftBar;
    Body *mRightBar;

    // invisible bodies
    Body *mTopWall;
    Body *mBottomWall;
    Body *mLeftWall;
    Body *mRightWall;

    Body *mBars[kNumBars] = {mLeftBar, mRightBar};
    Body *mWalls[kNumWalls] = {mLeftWall, mRightWall, mTopWall, mBottomWall};
    Body *mSideWalls[kNumSideWalls] = {mLeftWall, mRightWall};
    Body *mTBWalls[kNumTBWalls] = {mTopWall, mBottomWall};

    // Potential:
    // pause
    // rounds
};

} // namespace SdlPong

#endif /* ifndef  */
