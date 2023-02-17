#pragma once

#include "Component.h"
#include <SDL.h>
#include <string>
#include "../../Utility/Utils.h"
#include <SDL_image.h>
#include "Image.h"
#include "../Page.h"



class AnimatedImage : public Component
{
public:
    AnimatedImage(std::string file, std::string altFile, Page& p, int monitor, bool isAlwaysAnimated, std::string frameLoopType);
    AnimatedImage(std::string file, std::string altFile, Page& p, int monitor);
    virtual ~AnimatedImage();
    void freeGraphicsMemory();
    void allocateGraphicsMemory();
    void draw();
    void triggerEvent(std::string event, int menuIndex);
    void setPlaying();
    void setSpeed(int frameCount);
   /* void loopInverted();
    void loopOnceStop();
    void loopInvertedStop();*/


    Uint32 flags;
    int i, j, w, h, g;
    int current_frame;
    bool isAlwaysAnimated, finishedLoopOnce;
    int speed;
    
    

protected:
    IMG_Animation* anim;
    bool isPlaying_;
    SDL_Texture** texture_;
    std::string  file_;
    std::string  altFile_;
    unsigned int startTime;
    int once = 30;
    std::string frameLoopType;

};


