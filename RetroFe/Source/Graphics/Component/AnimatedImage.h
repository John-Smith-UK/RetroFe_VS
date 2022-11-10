#pragma once

#include "Component.h"
#include <SDL.h>
#include <string>
#include "../../Utility/Utils.h"
#include <SDL_image.h>
#include "Image.h"





class AnimatedImage : public Component
{
public:
    AnimatedImage(std::string file, std::string altFile, Page& p, int monitor);
    virtual ~AnimatedImage();
    void freeGraphicsMemory();
    void allocateGraphicsMemory();
    void draw();
    

    Uint32 flags;
    int i, j, w, h, done;
    int once = 30;
    int current_frame, delay;

protected:
    IMG_Animation* anim;
   // SDL_Texture* texture_;

    SDL_Texture** texture_;
    std::string  file_;
    std::string  altFile_;
    

};


