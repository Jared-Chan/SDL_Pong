#include "sdl_pong.hpp"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include <cassert>
#include <string>

SdlPong::Body::Body(GraphicBox gb, RigidBody rb, SdlPong::Id id)
    : mGraphicBox{gb}, mRigidBody{rb}, mId{id}, mInitGraphicBox{gb},
      mInitRigidBody{rb}, mCollided{false} {}

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
    SDL_RenderFillRect(renderer, &drawingRect);
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

SdlPong::Body::~Body() {
    /*SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Destroying body.");*/
}

SdlPong::TextBody::TextBody(std::string fontPath, GraphicBox gb)
    : Body{gb, {}, {}}, mText{fontPath} {

    constexpr int fontsize{28};
    if (mFont = TTF_OpenFont(fontPath.c_str(), fontsize); mFont == nullptr) {
        SDL_Log("Could not load %s! SDL_ttf Error: %s\n", fontPath.c_str(),
                SDL_GetError());
        assert(false && "Could not load font");
    }
}

void SdlPong::TextBody::Render(SDL_Renderer *renderer) {
    // Set texture position
    SDL_FRect dstRect = {static_cast<float>(mGraphicBox.rect.x),
                         static_cast<float>(mGraphicBox.rect.y),
                         static_cast<float>(mGraphicBox.rect.w),
                         static_cast<float>(mGraphicBox.rect.h)};
    // Render texture
    SDL_RenderTexture(renderer, mFontTexture, NULL, &dstRect);
}

void SdlPong::TextBody::setText(std::string text, SDL_Renderer *renderer) {

    SDL_DestroyTexture(mFontTexture);

    // Load text surface
    if (SDL_Surface *textSurface =
            TTF_RenderText_Blended(mFont, text.c_str(), 0, mGraphicBox.color);
        textSurface == nullptr) {
        SDL_Log("Unable to render text surface! SDL_ttf Error: %s\n",
                SDL_GetError());
    } else {
        // Create texture from surface
        if (mFontTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            mFontTexture == nullptr) {
            SDL_Log(
                "Unable to create texture from rendered text! SDL Error: %s\n",
                SDL_GetError());
        } else {
            mGraphicBox.rect.w = textSurface->w;
            mGraphicBox.rect.h = textSurface->h;
        }

        // Free temp surface
        SDL_DestroySurface(textSurface);
    }
}

SdlPong::TextBody::~TextBody() {
    SDL_DestroyTexture(mFontTexture);
    TTF_CloseFont(mFont);
}

/* SdlPong::AppState::AppState {{{ */
SdlPong::AppState::AppState(int screenWidth, int screenHeight)
    : mLeftScore{-1}, mRightScore{-1},
      barVel{static_cast<int>(screenHeight / 75.0)}, mBall{nullptr},
      mLeftBar{nullptr}, mRightBar{nullptr}, mTopWall{nullptr},
      mBottomWall{nullptr}, mLeftWall{nullptr}, mRightWall{nullptr} {
    // SDL_AppInit will provide window and renderer

    // Dimensions based on screen size
    int ballW{static_cast<int>(screenWidth / 25.0)};
    int ballH{ballW};

    int barH{static_cast<int>(screenHeight / 6.0)};
    int barW{ballW};

    int TBwallH{kPadding};
    int TBwallW{screenWidth};

    int sideWallH{screenHeight};
    int sideWallW{kPadding};

    SDL_Color white{0xFF, 0xFF, 0xFF, 0xFF};
    SdlPong::RigidBody stationery{.xvel = 0, .yvel = 0};
    /*SdlPong::RigidBody rightward{.xvel = barVel, .yvel = 0};*/

    // Ball is in the middle of the screen
    SDL_Rect ballRect{.x = static_cast<int>(screenWidth / 2. - ballW / 2.),
                      .y = static_cast<int>(screenHeight / 2. - ballW / 2.),
                      .w = ballW,
                      .h = ballH};
    SdlPong::GraphicBox ballBox{.rect = ballRect, .color = white};

    SDL_Rect leftBarRect{.x = 0,
                         .y = static_cast<int>(screenHeight / 2. - barH / 2.),
                         .w = barW,
                         .h = barH};
    SdlPong::GraphicBox leftBarBox{.rect = leftBarRect, .color = white};
    SDL_Rect rightBarRect{.x = screenWidth - barW,
                          .y = static_cast<int>(screenHeight / 2. - barH / 2.),
                          .w = barW,
                          .h = barH};
    SdlPong::GraphicBox rightBarBox{.rect = rightBarRect, .color = white};

    SDL_Rect topWallRect{.x = 0, .y = -kPadding, .w = TBwallW, .h = TBwallH};
    SdlPong::GraphicBox topWallBox{.rect = topWallRect, .color = white};
    SDL_Rect bottomWallRect{
        .x = 0, .y = screenHeight, .w = TBwallW, .h = TBwallH};
    SdlPong::GraphicBox bottomWallBox{.rect = bottomWallRect, .color = white};

    SDL_Rect leftWallRect{
        .x = -kPadding, .y = 0, .w = sideWallW, .h = sideWallH};
    SdlPong::GraphicBox leftWallBox{.rect = leftWallRect, .color = white};
    SDL_Rect rightWallRect{
        .x = screenWidth, .y = 0, .w = sideWallW, .h = sideWallH};
    SdlPong::GraphicBox rightWallBox{.rect = rightWallRect, .color = white};

    mBall = new Body(ballBox, stationery, SdlPong::ball);
    mLeftBar = new Body(leftBarBox, stationery, SdlPong::leftBar);
    mRightBar = new Body(rightBarBox, stationery, SdlPong::rightBar);

    mTopWall = new Body(topWallBox, stationery, SdlPong::topWall);
    mBottomWall = new Body(bottomWallBox, stationery, SdlPong::bottomWall);
    mLeftWall = new Body(leftWallBox, stationery, SdlPong::leftWall);
    mRightWall = new Body(rightWallBox, stationery, SdlPong::rightWall);

    mBars[0] = mLeftBar;
    mBars[1] = mRightBar;
    mWalls[0] = mLeftWall;
    mWalls[1] = mRightWall;
    mWalls[2] = mTopWall;
    mWalls[3] = mBottomWall;
    mSideWalls[0] = mLeftWall;
    mSideWalls[1] = mRightWall;
    mTBWalls[0] = mTopWall;
    mTBWalls[1] = mBottomWall;

    // Font
    std::string fontPath = "./slkscr.ttf";
    SDL_Rect leftScoreRect{
        .x = static_cast<int>(screenWidth / 4.), .y = ballH, .w = 0, .h = 0};
    SdlPong::GraphicBox leftScoreBox{.rect = leftScoreRect, .color = white};
    SDL_Rect rightScoreRect{.x = static_cast<int>(screenWidth * 3. / 4.),
                            .y = ballH,
                            .w = 0,
                            .h = 0};
    SdlPong::GraphicBox rightScoreBox{.rect = rightScoreRect, .color = white};

    mLeftScoreBody = new TextBody(fontPath, leftScoreBox);
    mRightScoreBody = new TextBody(fontPath, rightScoreBox);

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "AppState initialized.");
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

    delete mLeftScoreBody;
    delete mRightScoreBody;
}
/* }}} */

void SdlPong::AppState::startGame() {
    mBall->Reset();
    mBall->SetVel({.xvel=barVel, .yvel=0});
    // incScore must be called at least once to render text, and it must be
    // called after the window and renderer are created
    mLeftScore = -1;
    mRightScore = -1;
    incScore(left);
    incScore(right);
}

/* void SdlPong::AppState::incScore(SdlPong::Side side) {{{ */
void SdlPong::AppState::incScore(SdlPong::Side side) {
    if (side == SdlPong::left) {
        std::string scoreString = std::to_string(++mLeftScore);
        mLeftScoreBody->setText(scoreString, mRenderer);
    } else if (side == SdlPong::right) {
        std::string scoreString = std::to_string(++mRightScore);
        mRightScoreBody->setText(scoreString, mRenderer);
    }
} /* }}} */

/* void SdlPong::AppState::decScore(SdlPong::Side side) {{{ */
void SdlPong::AppState::decScore(SdlPong::Side side) {
    if (side == SdlPong::left) {
        std::string scoreString = std::to_string(--mLeftScore);
        mLeftScoreBody->setText(scoreString, mRenderer);
    } else if (side == SdlPong::right) {
        std::string scoreString = std::to_string(--mRightScore);
        mRightScoreBody->setText(scoreString, mRenderer);
    }
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
                if (mSideWalls[j]->getId() == SdlPong::leftWall) {
                    incScore(SdlPong::right);
                    mBall->SetVel({.xvel=-barVel, .yvel=0});
                }
                else if (mSideWalls[j]->getId() == SdlPong::rightWall) {
                    incScore(SdlPong::left);
                    mBall->SetVel({.xvel=barVel, .yvel=0});
                }
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

    // Clear all
    SDL_SetRenderDrawColor(mRenderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(mRenderer);

    mBall->Render(mRenderer);
    mLeftBar->Render(mRenderer);
    mRightBar->Render(mRenderer);

    // Render scores
    mLeftScoreBody->Render(mRenderer);
    mRightScoreBody->Render(mRenderer);

    // Update screen
    SDL_RenderPresent(mRenderer);

} /* }}} */
