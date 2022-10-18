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
#include "../ViewInfo.h"
#include "../../SDL.h"
#include "../../Utility/Log.h"
#include <SDL_image.h>






Image::Image(std::string file, std::string altFile, Page &p, int monitor)
    : Component(p)
    , texture_(NULL)
    , file_(file)
    , altFile_(altFile)

{
    baseViewInfo.Monitor = monitor;
    allocateGraphicsMemory();
}

Image::~Image()
{
    freeGraphicsMemory();

    }

void Image::freeGraphicsMemory()
{
    Component::freeGraphicsMemory();
    
    SDL_LockMutex(SDL::getMutex());
    if (texture_ != NULL)
    {
        SDL_DestroyTexture(texture_);
        texture_ = NULL;
     
    }
    SDL_UnlockMutex(SDL::getMutex());
}

//
//                  /**
//                  * set_status 	- Set image state - play or pause (or toggle to swap)
//                  * s 			> New Image::state state
//                  */
//
//void Image::set_status(state s)
//
//{
//    switch (s) {
//    case STATE_PLAY:
//        this->play = true;
//        break;
//    case STATE_PAUSE:
//        this->play = false;
//        break;
//    case STATE_TOGGLE:
//        this->play = !this->play;
//        break;
//    default:
//        break;
//    }
//}

void Image::allocateGraphicsMemory()
{
    int width;
    int height;


    //SDL_LockMutex(SDL::getMutex()); //lock do acesso
    std::string fileExtension = Utils::toLower(file_.substr(file_.find_last_of(".") + 1));

    if (!texture_)
    {
        if (fileExtension == "gif")

        {

           /* animatedImage(renderer, file_.c_str()); */

                gif_data = DGifOpenFileName(file_.c_str(), nullptr);

                // Will be null if image metadata could not be read
                if (!gif_data) {
                    std::cout << "DGifOpenFileName() failed - \'" << "\'" << std::endl;

                }

                // Will return GIF_ERROR if gif data structure cannot be populated
                if (DGifSlurp(gif_data) == GIF_ERROR) {
                    std::cout << "Failed to load image \' - " << "\'" << std::endl;
                }

                    this->animated = (gif_data->ImageCount > 1);
                    this->renderer = renderer;
                    this->w = gif_data->SWidth;
                    this->h = gif_data->SHeight;

                    this->frame_count = gif_data->ImageCount;

                    if (gif_data->SColorMap) {
                        // NOTE: IN BITS
                        this->depth = gif_data->SColorMap->BitsPerPixel;

                        /* I came across an example gif which was reported as 6 BPP by giflib.		*/
                        /* However, Windows reported it as 8 BPP and when set as 8 BPP, it loaded.	*/
                        /* In conclusion, I'm going to treat everything as a multiple of 8.			*/
                        if (this->depth % 8) {
                            this->depth = 8;
                        }
                    }
                    else {
                        // No global palette, assume 8 BPP depth
                        this->depth = 8;
                    }


                    // Create surface with existing gif data. 8 bit depth will trigger automatic creation of palette to be filled next
                    this->surface = SDL_CreateRGBSurfaceFrom((void*)gif_data->SavedImages[0].RasterBits, this->w, this->h, this->depth, this->w * (this->depth >> 3), 0, 0, 0, 0);

                    if (gif_data->SColorMap) { // if global colour palette defined
                        // convert from global giflib colour to SDL colour and populate palette
                        setPalette(gif_data->SColorMap, this->surface);
                    }
                    else if (gif_data->SavedImages[this->frame_index].ImageDesc.ColorMap) { // local colour palette
                        // convert from local giflib colour to SDL colour and populate palette
                        setPalette(gif_data->SavedImages[this->frame_index].ImageDesc.ColorMap, this->surface);
                    }

                    /* Convert to a more friendly format */
                    SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGB888);
                    SDL_Surface* output = SDL_ConvertSurface(surface, format, 0);
                    SDL_FreeFormat(format);
                    SDL_FreeSurface(surface);
                    surface = output;

                    this->texture_ = SDL_CreateTextureFromSurface(this->renderer, this->surface);

                    if (this->animated) {
                        if (this->prerendered) prerender();
                        animationThread = std::thread(&Component::animating, this);
                    }

        

                  
                 
           
           

            if (!texture_ && altFile_ != "")
            {
              
                gif_data = DGifOpenFileName(altFile_.c_str(), nullptr);
             //   texture_ = SDL_CreateTextureFromSurface(this->renderer, this->surface);
            }

            if (texture_ != NULL)
            {
                prepare();
             //   texture_ = SDL_CreateTextureFromSurface(this->renderer, this->surface);
                SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
                SDL_QueryTexture(texture_, NULL, NULL, &width, &height);
                baseViewInfo.ImageWidth = (float)width;
                baseViewInfo.ImageHeight = (float)height;
                   
            }
            SDL_UnlockMutex(SDL::getMutex());
           
        }


        else if (fileExtension == "apng")
        {
            /* texture_ = IMG_LoadTexture(SDL::getRenderer(baseViewInfo.Monitor), file_.c_str())*/;
        }

        else
        {

            SDL_LockMutex(SDL::getMutex());
            texture_ = IMG_LoadTexture(SDL::getRenderer(baseViewInfo.Monitor), file_.c_str());
            if (!texture_ && altFile_ != "")
            {
                texture_ = IMG_LoadTexture(SDL::getRenderer(baseViewInfo.Monitor), altFile_.c_str());
            }

            if (texture_ != NULL)
            {
                SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
                SDL_QueryTexture(texture_, NULL, NULL, &width, &height);
                baseViewInfo.ImageWidth = (float)width;
                baseViewInfo.ImageHeight = (float)height;
            }
            SDL_UnlockMutex(SDL::getMutex());

        }

        Component::allocateGraphicsMemory();

    }
}
//void Image::gifframedraw()
//{
//    std::string fileExtension = Utils::toLower(file_.substr(file_.find_last_of(".") + 1));
//    
//    if (texture_)
//    {
//        if (fileExtension == "gif")
//
//        {
//           
//
//        }
//    }
//}


void Image::draw()
{
   
     std::string fileExtension = Utils::toLower(file_.substr(file_.find_last_of(".") + 1));
   
    Component::draw();

    
   if (texture_)
    {
   /*      if (fileExtension == "gif")

        {


        }

         if (fileExtension == "apng")

        {



        }*/

        //if (fileExtension == "jpeg" || "png")
        //{

            SDL_Rect rect;

            rect.x = static_cast<int>(baseViewInfo.XRelativeToOrigin());
            rect.y = static_cast<int>(baseViewInfo.YRelativeToOrigin());
            rect.h = static_cast<int>(baseViewInfo.ScaledHeight());
            rect.w = static_cast<int>(baseViewInfo.ScaledWidth());


            SDL::renderCopy(texture_, baseViewInfo.Alpha, NULL, &rect, baseViewInfo, page.getLayoutWidth(baseViewInfo.Monitor), page.getLayoutHeight(baseViewInfo.Monitor));
        }
    /*}*/

        
  
}



    

