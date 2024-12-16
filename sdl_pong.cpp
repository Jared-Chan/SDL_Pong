#include "sdl_pong.hpp"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include <cassert>

SdlPong::Body::Body(GraphicBox gb, RigidBody rb, SdlPong::Id id)
    : mGraphicBox{gb}, mRigidBody{rb}, mId{id}, mInitGraphicBox{gb},
      mInitRigidBody{rb} {}

void SdlPong::Body::UpdatePos() {
    mGraphicBox.rect.x += mRigidBody.xvel;
    mGraphicBox.rect.y += mRigidBody.yvel;
}

void SdlPong::Body::Reset() {
    mGraphicBox = mInitGraphicBox;
    mRigidBody = mInitRigidBody;
}

SDL_Rect *SdlPong::Body::GetCollisionBox() { return &mGraphicBox.rect; }

void SdlPong::Body::SetVel(RigidBody rb) {
    mRigidBody.xvel = rb.xvel;
    mRigidBody.yvel = rb.yvel;
}

SdlPong::RigidBody SdlPong::Body::GetVel() { return mRigidBody; }

void SdlPong::Body::Render(SDL_Renderer *renderer) {
    SDL_FRect drawingRect{static_cast<float>(mGraphicBox.rect.x),
                          static_cast<float>(mGraphicBox.rect.y),
                          static_cast<float>(mGraphicBox.rect.w),
                          static_cast<float>(mGraphicBox.rect.h)};
    SDL_SetRenderDrawColor(renderer, mGraphicBox.color.r, mGraphicBox.color.g,
                           mGraphicBox.color.b, mGraphicBox.color.a);
    SDL_RenderRect(renderer, &drawingRect);
}

SdlPong::Id SdlPong::Body::getId() { return mId; }
void SdlPong::Body::RegisterCollision(SdlPong::Body *body) {

    if (!(mId == SdlPong::ball || mId == SdlPong::leftBar ||
          mId == SdlPong::rightBar)) {
        assert(false && "Don't register collisions for walls");
    }

    mCollided = true;
    mPostCollisionPos = mGraphicBox.rect;
    mPostCollisionXvel = mRigidBody.xvel;
    mPostCollisionYvel = mRigidBody.yvel;

    if (mId == SdlPong::ball) {
        if (body->getId() == SdlPong::leftBar ||
            body->getId() == SdlPong::rightBar) {
            mPostCollisionXvel = -mPostCollisionXvel;
            mPostCollisionYvel = body->GetVel().yvel;
        } else if (body->getId() == SdlPong::leftWall ||
                   body->getId() == SdlPong::rightWall) {
            // AppState handles this case
            mCollided = false;
        } else if (body->getId() == SdlPong::topWall ||
                   body->getId() == SdlPong::bottomWall) {
            mPostCollisionYvel = -mPostCollisionYvel;
        } else {
            assert(false && "Unknown collision body.");
        }
    } else if (mId == SdlPong::leftBar || mId == SdlPong::rightBar) {
        if (body->getId() == SdlPong::topWall) {
            mPostCollisionYvel = 0;
            mPostCollisionPos.y =
                body->GetCollisionBox()->y + body->GetCollisionBox()->h;
        } else if (body->getId() == SdlPong::bottomWall) {
            mPostCollisionYvel = 0;
            mPostCollisionPos.y =
                body->GetCollisionBox()->y - mGraphicBox.rect.h;
        } else {
            assert(false && "Unknown collision body.");
        }
    }
}
void SdlPong::Body::HandleCollision() {
    if (mCollided) {
        mGraphicBox.rect = mPostCollisionPos;
        mRigidBody.xvel = mPostCollisionXvel;
        mRigidBody.yvel = mPostCollisionYvel;
        mCollided = false;
    }
}

/* SdlPong::AppState::AppState {{{ */
SdlPong::AppState::AppState(int screenWidth, int screenHeight)
    : mLeftScore{0}, mRightScore{0},
      barVel{static_cast<int>(screenHeight / 4.0)}, mBall{nullptr},
      mLeftBar{nullptr}, mRightBar{nullptr}, mTopWall{nullptr},
      mBottomWall{nullptr}, mLeftWall{nullptr}, mRightWall{nullptr} {
    // SDL_AppInit will provide window and renderer

    // Dimensions based on screen size
    int ballW{static_cast<int>(screenWidth / 10.0)};
    int ballH{ballW};

    int barH{static_cast<int>(screenHeight / 5.0)};
    int barW{ballW};

    int TBwallH{kPadding};
    int TBwallW{screenWidth};

    int sideWallH{screenHeight};
    int sideWallW{kPadding};

    SDL_Color white{0xFF, 0xFF, 0xFF, 0xFF};
    SdlPong::RigidBody stationery{.xvel = 0, .yvel = 0};
    SdlPong::RigidBody rightward{.xvel = 0, .yvel = 0};

    // Ball is in the middle of the screen
    SDL_Rect ballRect{
        .x = static_cast<int>(screenWidth / 2. + kPadding - ballW / 2.),
        .y = static_cast<int>(screenWidth / 2. + kPadding - ballW / 2.),
        .w = ballW,
        .h = ballH};
    SdlPong::GraphicBox ballBox{.rect = ballRect, .color = white};

    SDL_Rect leftBarRect{
        .x = kPadding,
        .y = static_cast<int>(screenWidth / 2. + kPadding - barH / 2.),
        .w = barW,
        .h = barH};
    SdlPong::GraphicBox leftBarBox{.rect = leftBarRect, .color = white};
    SDL_Rect rightBarRect{
        .x = screenWidth + kPadding - barW,
        .y = static_cast<int>(screenWidth / 2. + kPadding - barH / 2.),
        .w = barW,
        .h = barH};
    SdlPong::GraphicBox rightBarBox{.rect = rightBarRect, .color = white};

    SDL_Rect topWallRect{.x = kPadding, .y = 0, .w = TBwallW, .h = TBwallH};
    SdlPong::GraphicBox topWallBox{.rect = topWallRect, .color = white};
    SDL_Rect bottomWallRect{.x = kPadding,
                            .y = screenHeight + kPadding,
                            .w = TBwallW,
                            .h = TBwallH};
    SdlPong::GraphicBox bottomWallBox{.rect = bottomWallRect, .color = white};

    SDL_Rect leftWallRect{
        .x = 0, .y = kPadding, .w = sideWallW, .h = sideWallH};
    SdlPong::GraphicBox leftWallBox{.rect = leftWallRect, .color = white};
    SDL_Rect rightWallRect{.x = screenWidth + kPadding,
                           .y = kPadding,
                           .w = sideWallW,
                           .h = sideWallH};
    SdlPong::GraphicBox rightWallBox{.rect = rightWallRect, .color = white};

    mBall = new Body(ballBox, rightward, SdlPong::ball);
    mLeftBar = new Body(leftBarBox, stationery, SdlPong::leftBar);
    mRightBar = new Body(rightBarBox, stationery, SdlPong::rightBar);

    mTopWall = new Body(topWallBox, stationery, SdlPong::topWall);
    mBottomWall = new Body(bottomWallBox, stationery, SdlPong::bottomWall);
    mLeftWall = new Body(leftWallBox, stationery, SdlPong::leftWall);
    mRightWall = new Body(rightWallBox, stationery, SdlPong::rightWall);
}
/* }}} */

/* SdlPong::AppState::~AppState {{{ */
SdlPong::AppState::~AppState() {
    delete mBall;
    delete mLeftBar;
    delete mRightBar;

    delete mTopWall;
    delete mBottomWall;
    delete mLeftWall;
    delete mRightWall;
}
/* }}} */

/* void SdlPong::AppState::incScore(SdlPong::Side side) {{{ */
void SdlPong::AppState::incScore(SdlPong::Side side) {
    if (side == SdlPong::left)
        ++mLeftScore;
    else if (side == SdlPong::right)
        ++mRightScore;
} /* }}} */

/* void SdlPong::AppState::decScore(SdlPong::Side side) {{{ */
void SdlPong::AppState::decScore(SdlPong::Side side) {
    if (side == SdlPong::left)
        --mLeftScore;
    else if (side == SdlPong::right)
        --mRightScore;
} /* }}} */

void SdlPong::AppState::moveBar(SdlPong::Side side, SdlPong::BarDirection dir) {
    SdlPong::RigidBody newRb{0, 0};
    switch (dir) {
    case SdlPong::up:
        newRb.yvel = -barVel;
        break;
    case SdlPong::down:
        newRb.yvel = barVel;
        break;
    case SdlPong::none:
        newRb.yvel = 0;
        break;
    default:
        assert(false && "Invaild direction");
    }
    if (side == SdlPong::left)
        mLeftBar->SetVel(newRb);
    else if (side == SdlPong::right)
        mRightBar->SetVel(newRb);
}

/* SDL_Window SdlPong::AppState::getWindow() {{{ */
SDL_Window *SdlPong::AppState::getWindow() { return mWindow; } /* }}} */

/* SDL_Renderer SdlPong::AppState::getRenderer() {{{ */
SDL_Renderer *SdlPong::AppState::getRenderer() { return mRenderer; } /* }}} */

/* void SdlPong::AppState::UpdatePositions() {{{ */
void SdlPong::AppState::UpdatePositions() {
    mBall->UpdatePos();
    mLeftBar->UpdatePos();
    mRightBar->UpdatePos();
} /* }}} */

/* void SdlPong::AppState::CheckCollisions() {{{ */
void SdlPong::AppState::CheckCollisions() {

    for (int i{0}; i < kNumBars; ++i) {

        if (SDL_HasRectIntersection(mBars[i]->GetCollisionBox(),
                                    mBall->GetCollisionBox())) {
            mBall->RegisterCollision(mBars[i]);
        }

        for (int j{0}; j < kNumSideWalls; ++j) {
            if (SDL_HasRectIntersection(mBall->GetCollisionBox(),
                                        mSideWalls[j]->GetCollisionBox())) {
                /*mBall->RegisterCollision(mSideWalls[j]);*/
                // AppState handles this case
                mBall->Reset();
                if (mSideWalls[j]->getId() == SdlPong::leftWall)
                    incScore(SdlPong::right);
                else if (mSideWalls[j]->getId() == SdlPong::rightWall)
                    incScore(SdlPong::left);
            }
        }
        for (int j{0}; j < kNumTBWalls; ++j) {
            if (SDL_HasRectIntersection(mBall->GetCollisionBox(),
                                        mTBWalls[j]->GetCollisionBox()))
                mBall->RegisterCollision(mTBWalls[j]);
            if (SDL_HasRectIntersection(mBars[i]->GetCollisionBox(),
                                        mTBWalls[j]->GetCollisionBox()))
                mBars[i]->RegisterCollision(mTBWalls[j]);
        }
    }

} /* }}} */
/* void SdlPong::AppState::ProcessCollisions() {{{ */
void SdlPong::AppState::ProcessCollisions() {

    for (int i{0}; i < kNumBars; ++i) {
        mBars[i]->HandleCollision();
    }
    mBall->HandleCollision();

} /* }}} */
/* void SdlPong::AppState::Render() {{{ */
void SdlPong::AppState::Render() {

    mBall->Render(mRenderer);
    mLeftBar->Render(mRenderer);
    mRightBar->Render(mRenderer);

    // Render scores

} /* }}} */
