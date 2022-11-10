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
#include "Image.h"
#include"AnimatedImage.h"
#include "../ViewInfo.h"
#include "../../SDL.h"
#include "../../Utility/Log.h"
#include <SDL_image.h>





//IMG_Animation* anim;
//SDL_Texture** textures;


static void draw_background(SDL_Renderer* renderer, int w, int h)
{
    SDL_Color col[2] = {
        { 0x66, 0x66, 0x66, 0xff },
        { 0x99, 0x99, 0x99, 0xff },
    };
    int i, x, y;
    SDL_Rect rect;

    rect.w = 8;
    rect.h = 8;
    for (y = 0; y < h; y += rect.h) {
        for (x = 0; x < w; x += rect.w) {
            /* use an 8x8 checkerboard pattern */
            i = (((x ^ y) >> 3) & 1);
            SDL_SetRenderDrawColor(renderer, col[i].r, col[i].g, col[i].b, col[i].a);

            rect.x = x;
            rect.y = y;
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}




AnimatedImage::AnimatedImage(std::string file, std::string altFile, Page& p, int monitor)
    : Component(p)
    , texture_(NULL)
    , file_(file)
    , altFile_(altFile)

{
    baseViewInfo.Monitor = monitor;
    allocateGraphicsMemory();
}

AnimatedImage::~AnimatedImage()
{
    freeGraphicsMemory();
    IMG_FreeAnimation(anim);
}

void AnimatedImage::freeGraphicsMemory()
{
    /*texture_ = (SDL_Texture**)SDL_calloc(anim->count, sizeof(*texture_));*/
    Component::freeGraphicsMemory();
    IMG_FreeAnimation(anim);
    SDL_LockMutex(SDL::getMutex());
    if (texture_ != NULL)
    {
        for (j = 0; j < anim->count; ++j) {
            SDL_DestroyTexture(texture_[j]);
        }

        texture_ = NULL;

    }
    SDL_UnlockMutex(SDL::getMutex());
}


void AnimatedImage::allocateGraphicsMemory()
{
    
     
  
       
       /* w = anim->w;
        h = anim->h;*/
        




        if (!texture_)
        {
            anim = IMG_LoadAnimation(file_.c_str());
            SDL_LockMutex(SDL::getMutex());
            

            texture_ = (SDL_Texture**)SDL_calloc(anim->count, sizeof(*texture_));
            
        
        if (!texture_ && altFile_ != "")
        {
            anim = IMG_LoadAnimation(altFile_.c_str());

        }

        if (texture_ != NULL)
        {
            texture_ = (SDL_Texture**)SDL_calloc(anim->count, sizeof(*texture_));
            for (j = 0; j < anim->count; ++j)
            {
                texture_[j] = SDL_CreateTextureFromSurface(SDL::getRenderer(baseViewInfo.Monitor), anim->frames[j]);
            }
            current_frame = 0;

            //draw_background(SDL::getRenderer(baseViewInfo.Monitor), w, h);
            // SDL_RenderCopy(renderer, texture_[current_frame], NULL, NULL);
            //SDL_RenderPresent(SDL::getRenderer(baseViewInfo.Monitor));

              SDL_SetTextureBlendMode(texture_[current_frame], SDL_BLENDMODE_BLEND);
              SDL_QueryTexture(texture_[current_frame], NULL, NULL, &w, &h);
            baseViewInfo.ImageWidth = (float)w;
            baseViewInfo.ImageHeight = (float)h;

            if (anim->delays[current_frame]) {
                delay = anim->delays[current_frame];
            }
            else
            {
                delay = 100;
            }
            // SDL_Delay(delay);
           
        }

        SDL_UnlockMutex(SDL::getMutex());


        }

    Component::allocateGraphicsMemory();

}





void AnimatedImage::draw()
{

  
    Component::draw();


    if (texture_)

    {


        SDL_Rect rect;

        rect.x = static_cast<int>(baseViewInfo.XRelativeToOrigin());
        rect.y = static_cast<int>(baseViewInfo.YRelativeToOrigin());
        rect.h = static_cast<int>(baseViewInfo.ScaledHeight());
        rect.w = static_cast<int>(baseViewInfo.ScaledWidth());

        //SDL_RenderCopy(renderer, texture_[current_frame], NULL, NULL);
        SDL::renderCopy(texture_[current_frame], baseViewInfo.Alpha, NULL, &rect, baseViewInfo, page.getLayoutWidth(baseViewInfo.Monitor), page.getLayoutHeight(baseViewInfo.Monitor));

        

        current_frame = (current_frame + 1) % anim->count;


    }




}