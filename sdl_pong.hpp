#ifndef _JC_SDL_PONG

#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

namespace SdlPong {

enum Side {
    left,
    right,
};

enum BarDirection { up, down, none };

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
    SDL_Rect rect; // size and position
    SDL_Color color;
};

struct RigidBody { // velocity
    int xvel;
    int yvel;
};

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

  protected:
    GraphicBox mGraphicBox;
    RigidBody mRigidBody;
    Id mId;

  private:
    SDL_Rect mPostCollisionPos{};
    int mPostCollisionXvel{};
    int mPostCollisionYvel{};
    bool mCollided{false};
    GraphicBox mInitGraphicBox;
    RigidBody mInitRigidBody;
};

class TextBody : Body {
  public:
    TextBody(std::string fontPath, GraphicBox gb);
    ~TextBody();
    void setText(std::string text, SDL_Renderer *renderer);
    void Render(SDL_Renderer *renderer);

  private:
    std::string mText;
    TTF_Font *mFont;
    SDL_Texture *mFontTexture;
};

class AppState {

  public:
    AppState(int screenWidth, int screenHeight);

    void startGame();

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
    static constexpr int kPadding{10};

    static constexpr int kNumBars{2};
    static constexpr int kNumWalls{4};
    static constexpr int kNumTBWalls{2};
    static constexpr int kNumSideWalls{2};

    int barVel;

    int mLeftScore;
    int mRightScore;

    TextBody *mLeftScoreBody;
    TextBody *mRightScoreBody;

    Body *mBall;
    Body *mLeftBar;
    Body *mRightBar;

    // invisible bodies
    Body *mTopWall;
    Body *mBottomWall;
    Body *mLeftWall;
    Body *mRightWall;

    // For interating through
    Body *mBars[kNumBars] = {mLeftBar, mRightBar};
    Body *mWalls[kNumWalls] = {mLeftWall, mRightWall, mTopWall, mBottomWall};
    Body *mSideWalls[kNumSideWalls] = {mLeftWall, mRightWall};
    Body *mTBWalls[kNumTBWalls] = {mTopWall, mBottomWall};
};

} // namespace SdlPong

#endif /* ifndef  */
