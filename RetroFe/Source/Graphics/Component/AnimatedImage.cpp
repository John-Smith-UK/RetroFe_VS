/* This file is part of RetroFE.
 *
 * RetroFE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RetroFE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RetroFE.  If not, see <http://www.gnu.org/licenses/>.
 */
//#include "Image.h"
#include"AnimatedImage.h"
#include "../ViewInfo.h"
#include "../../SDL.h"
#include "../../Utility/Log.h"
#include <SDL_image.h>


AnimatedImage::AnimatedImage(std::string file, std::string altFile, Page& p, int monitor, bool isAlwaysAnimated, std::string frameLoopType)
    : Component(p)
    , texture_(NULL)
    , file_(file)
    , altFile_(altFile)

{
    baseViewInfo.Monitor = monitor;
    AnimatedImage::isAlwaysAnimated = isAlwaysAnimated;
    AnimatedImage::frameLoopType = frameLoopType;

    AnimatedImage::finishedLoopOnce = false;

    allocateGraphicsMemory();
}

AnimatedImage::AnimatedImage(std::string file, std::string altFile, Page& p, int monitor)
    : Component(p)
    , texture_(NULL)
    , file_(file)
    , altFile_(altFile)

{
    baseViewInfo.Monitor = monitor;
    AnimatedImage::isAlwaysAnimated = false;
    AnimatedImage::frameLoopType = 2; //playLoop
    allocateGraphicsMemory();
}

AnimatedImage::~AnimatedImage()
{
    freeGraphicsMemory();
}

void AnimatedImage::freeGraphicsMemory()
{
    Component::freeGraphicsMemory();
    SDL_LockMutex(SDL::getMutex());
    if (texture_ != NULL)
    {
        for (j = 0; j < anim->count; ++j) {
            SDL_DestroyTexture(texture_[j]);
        }
            IMG_FreeAnimation(anim);
        texture_ = NULL;

    }
    
    SDL_UnlockMutex(SDL::getMutex());
}


void AnimatedImage::allocateGraphicsMemory()
{
        if (!texture_)
        {
            anim = IMG_LoadAnimation(file_.c_str());
            texture_ = (SDL_Texture**)SDL_calloc(anim->count, sizeof(*texture_));
            SDL_LockMutex(SDL::getMutex());
        
        if (!texture_ && altFile_ != "")
        {
            anim = IMG_LoadAnimation(altFile_.c_str());
            texture_ = (SDL_Texture**)SDL_calloc(anim->count, sizeof(*texture_));
        }

        if (texture_ != NULL)
        {
            texture_ = (SDL_Texture**)SDL_calloc(anim->count, sizeof(*texture_));
            for (j = 0; j < anim->count; ++j)
            {
                if (frameLoopType == "playOnce" || frameLoopType == "playLoop") //normal gif
                {
                    texture_[j] = SDL_CreateTextureFromSurface(SDL::getRenderer(baseViewInfo.Monitor), anim->frames[j]);
                }
                else { //inverted gif
                    texture_[j] = SDL_CreateTextureFromSurface(SDL::getRenderer(baseViewInfo.Monitor), anim->frames[anim->count - 1 - j]);
                }
            }
           // current_frame = 0;

            SDL_SetTextureBlendMode(texture_[current_frame], SDL_BLENDMODE_BLEND);
            SDL_QueryTexture(texture_[current_frame], NULL, NULL, &w, &h);
            baseViewInfo.ImageWidth = (float)w;
            baseViewInfo.ImageHeight = (float)h;

                     
        }
        setSpeed(anim->count);
        setPlaying();


SDL_UnlockMutex(SDL::getMutex());


        }

        Component::allocateGraphicsMemory();

}





void AnimatedImage::draw()
{
    Component::draw();

    startTime = 176;
    if (texture_)

    {
        SDL_Rect rect;

        rect.x = static_cast<int>(baseViewInfo.XRelativeToOrigin());
        rect.y = static_cast<int>(baseViewInfo.YRelativeToOrigin());
        rect.h = static_cast<int>(baseViewInfo.ScaledHeight());
        rect.w = static_cast<int>(baseViewInfo.ScaledWidth());

       // SDL_Delay(10);

        if (isPlaying_)
        {
            
            if ((frameLoopType == "playOnce" || frameLoopType == "playInvertedOnce") && current_frame == anim->count - 1 || finishedLoopOnce)
            {
                /*if (current_frame == anim->count - 1 || finishedLoopOnce)*/
               /* {*/
                    current_frame = 0;
                    finishedLoopOnce = true;
            }
            else if (frameLoopType == "noPlay")
            {
                current_frame = 0;
            }
                else {
                    current_frame = ((SDL_GetTicks() - startTime) * once / speed) % anim->count;
                }
         

        }
        else 
        {
            current_frame = 0;
        }

        SDL::renderCopy(texture_[current_frame], baseViewInfo.Alpha, NULL, &rect, baseViewInfo, page.getLayoutWidth(baseViewInfo.Monitor), page.getLayoutHeight(baseViewInfo.Monitor));
    }
}


void AnimatedImage::triggerEvent(std::string event, int menuIndex)
{
    if (anim)
    {
        Component::triggerEvent(event, menuIndex);
        if (event == "highlightEnter")
        {
            setPlaying();
        }

        if (event == "menuEnter") //for animated images that are not aprt of menu
        {
            finishedLoopOnce = false;
            setPlaying();
        }

    }
}

void AnimatedImage::setPlaying()
{
    if (AnimatedImage::isAlwaysAnimated)
    {
        isPlaying_ = true;
        return;
    }

    Item* selectedGif = page.getSelectedItem();
    if (selectedGif)
    {
        std::string filepath = file_.c_str();
        if (filepath.find(selectedGif->title.c_str()) < filepath.length()) //if it is selected item
        {
            isPlaying_ = true;
        }
        else
        {
            isPlaying_ = false;
        }
    }
}
void AnimatedImage::setSpeed(int frameCount)
{

   
    //60 => 1000
    //30 => 3000 30*30
    //10 => 3100 10*3100


    //int defaultAnimValue = 1000; //1sec = 60frames
    //int result = (defaultAnimValue * anim->count) / 60;
    //speed = result;
    speed = 7500;
}

