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
#include "Component.h"
#include "../Animate/Tween.h"
#include "../../Graphics/ViewInfo.h"
#include "../../Utility/Log.h"
#include "../../SDL.h"
#include "../PageBuilder.h"




Component::Component(Page &p)
: page(p)
{
    tweens_                   = NULL;
    backgroundTexture_        = NULL;
    menuScrollReload_         = false;
    freeGraphicsMemory();
    id_                       = -1;
}

Component::Component(const Component &copy)
    : page(copy.page)
{
    tweens_ = NULL;
    backgroundTexture_ = NULL;
    freeGraphicsMemory();

    if ( copy.tweens_ )
    {
        AnimationEvents *tweens = new AnimationEvents(*copy.tweens_);
        setTweens(tweens);
    }


}

Component::~Component()
{
    freeGraphicsMemory();
}

void Component::freeGraphicsMemory()
{
    animationRequestedType_ = "";
    animationType_          = "";
    animationRequested_     = false;
    newItemSelected         = false;
    newScrollItemSelected   = false;
    menuIndex_              = -1;

    currentTweens_        = NULL;
    currentTweenIndex_    = 0;
    currentTweenComplete_ = true;
    elapsedTweenTime_     = 0;

    if ( backgroundTexture_ )
    {
       
        //GIF SUPPORT
        if (this->animated) {
            this->quit = true;
            animationThread.join();
        }
        DGifCloseFile(this->gif_data, nullptr);
        SDL_FreeSurface(this->surface);
        if (prerendered) {
            for (uint32_t i = 0; i < this->frames.size(); i++) {
                SDL_DestroyTexture(this->frames[i]);
            }
        }
        //END GIF SUPPORT
                
       
        SDL_LockMutex(SDL::getMutex());
        SDL_DestroyTexture(backgroundTexture_);
        SDL_UnlockMutex(SDL::getMutex());

        backgroundTexture_ = NULL;
    } 
}

void Component::allocateGraphicsMemory()
{
    if (!backgroundTexture_)
    {

        // make a 4x4 pixel wide surface to be stretched during rendering, make it a white background so we can use
        // color  later
        SDL_Surface* surface = SDL_CreateRGBSurface(0, 4, 4, 32, 0, 0, 0, 0);
        SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 255, 255));

        SDL_LockMutex(SDL::getMutex());
        backgroundTexture_ = SDL_CreateTextureFromSurface(SDL::getRenderer(baseViewInfo.Monitor), surface);
        SDL_UnlockMutex(SDL::getMutex());

        SDL_FreeSurface(surface);
        SDL_SetTextureBlendMode(backgroundTexture_, SDL_BLENDMODE_BLEND);
        
    }

   
}
// GIF SUPPORT
/**
//////* setPalette	- Routine to convert giflib colour palette to SDL colour palette
* colorMap 		> giflib ColorMapObject containing colour count and palette
* surface 		> SDL Surface to apply converted palette to
*/
void Component::setPalette(ColorMapObject* colorMap, SDL_Surface* surface) {
    // start gif colour at the start
    uint8_t* gc = (uint8_t*)colorMap->Colors;

    // this long, stupid conversion required as SDL *REQUIRES* an alpha channel
    for (int i = 0; i < colorMap->ColorCount; i++) {
        //create corresponding SDL_Color
        SDL_Color sc = { gc[0], gc[1], gc[2], 0 };
        //set palette with new colour
        SDL_SetPaletteColors(surface->format->palette, &sc, i, 1);
        // move along to next colour
        gc += sizeof(GifColorType);
    }
}

/**
* setIndex	- Set frame index record to provided index with boundaries checked
* index 	> New frame index
*/
void Component::setIndex(uint16_t index) {
    this->frame_index = index;
    this->frame_index %= this->frame_count;
}

/**
* getDelay	- Load extension chunks related to provided image index and search for a delay value
* index 	> Target frame index
*/
uint16_t Component::getDelay(uint16_t index) {
    index %= frame_count;
    //call search for graphics block
    ExtensionBlock* gfx = getGraphicsBlock(index);
    //if one was found, update delay
    if (gfx) {
        // delay is bounded by somewhat standard lower limit of 0.02 seconds per frame
        this->delay_val = std::max((uint16_t)GIF_MIN_DELAY, (uint16_t)(((uint16_t)(gfx->Bytes[2]) << 8) + gfx->Bytes[1])); // see [1]
    }
    return this->delay_val;
}

/**
* getDelay - Shortcut for getDelay() defaulting to current frame index
*/
uint16_t Component::getDelay() {
    return getDelay(frame_index);
}

/**
* getGraphicsBlock 	- Locate and return the Extension Block of type Graphics Control for provided index
* index 			> Target frame index
*/
ExtensionBlock* Component::getGraphicsBlock(uint16_t index) {
    // iterate and look for graphics extension with timing data
    for (int i = 0; i < gif_data->SavedImages[index].ExtensionBlockCount; i++) {
        if (gif_data->SavedImages[index].ExtensionBlocks[i].Function == GRAPHICS_EXT_FUNC_CODE) { // found it
            return &gif_data->SavedImages[index].ExtensionBlocks[i];
        }
    }
    return nullptr;
}

/**
* animate - The function is called as a thread and will advance the index and manage timing for the animation automatically.
*			If the status is set to paused, it will wait before continuing to animate.
*/
void Component::animating() {
    while (!this->quit) {
        while (!this->play) {
            if (this->quit) return; //make it possible to break out for quitting while paused
            SDL_Delay(30);
        }
        setIndex(this->frame_index + 1); 	//advance by a frame
        this->ready = true;					//mark frame as ready to be prepare()'d
        getDelay();
        SDL_Delay(this->delay_val * 10);	//wait (gif resolution is only 1/100 of a sec, mult. by 10 for millis)
    }

   
    return;
}

void Component::prerender()
{
    for (int i = 0; i < this->frame_count; i++) {
        prepare(i);
        this->frames.push_back(this->backgroundTexture_);
        this->backgroundTexture_ = nullptr; //remove reference so next call to prepare doesn't free it
    }
    return;
}

/**
* prepare - Create surface from specified index, apply palette and create texture, keying as required.
*			This should be called as infrequently as possible - static images don't need refreshing.
*/
void Component::prepare(uint16_t index) {
    // shortened for simplicity
    GifImageDesc* im_desc = &this->gif_data->SavedImages[index].ImageDesc;

    // destination for copy - if only a region is being updated, this will not cover the whole image
    //SDL_Rect dest;
    grect.x = im_desc->Left;
    grect.y = im_desc->Top;
    grect.w = im_desc->Width;
   grect.h = im_desc->Height;

   rect = grect;
    
  //  SDL::renderCopy(backgroundTexture_, baseViewInfo.BackgroundAlpha, NULL, &rect, baseViewInfo, page.getLayoutWidth(baseViewInfo.Monitor), page.getLayoutHeight(baseViewInfo.Monitor));
    SDL_Surface* temp = SDL_CreateRGBSurfaceFrom((void*)this->gif_data->SavedImages[index].RasterBits, im_desc->Width, im_desc->Height, this->depth, im_desc->Width * (this->depth >> 3), 0, 0, 0, 0);

    if (gif_data->SavedImages[index].ImageDesc.ColorMap) { // local colour palette
        // convert from local giflib colour to SDL colour and populate palette
        setPalette(gif_data->SavedImages[index].ImageDesc.ColorMap, temp);
    }
    else if (gif_data->SColorMap) { // if global colour palette defined
        // convert from global giflib colour to SDL colour and populate palette
        setPalette(gif_data->SColorMap, temp);
    }

    //get gfx extension block to find transparent palette index
    ExtensionBlock* gfx = getGraphicsBlock(index);

    //if gfx block exists and transparency flag is set, set colour key
    if (gfx && (gfx->Bytes[0] & 0x01)) {
        // set the key with SDL_TRUE to enable it
        SDL_SetColorKey(temp, SDL_TRUE, gfx->Bytes[3]);
    }

    // copy over region that is being updated, leaving anything else
    SDL_BlitSurface(temp, nullptr, this->surface, &grect);
    SDL_FreeSurface(temp);

    SDL_DestroyTexture(this->backgroundTexture_); //delete old texture
    this->backgroundTexture_ = SDL_CreateTextureFromSurface(this->renderer, this->surface);

    this->ready = false; 	// mark current frame as already requested
}

/**
* prepare - If in prerender mode, update index. Otherwise, prepare current index frame.
*/
void Component::prepare() {
    if (prerendered) {
        this->backgroundTexture_ = frames[frame_index];
        this->ready = false;
    }
    else {
        prepare(frame_index);
    }
}


/*
    TODO:
    - Some troublesome gifs will now no longer play at all
    - Scrambled palette bug still exists on some images
*/

/*
[1]: 	Giflib cuts off the first 3 bytes of extension chunk.
        Payload is then [Packed/Flag Byte], [Upper Delay], [Lower Delay], [Transparency Index].
*/

//void Component::animatedImage(SDL_Renderer* renderer, const char*  filename)
//
//{
//
//    gif_data = DGifOpenFileName(filename, nullptr);
//
//    // Will be null if image metadata could not be read
//    if (!gif_data) {
//        std::cout << "DGifOpenFileName() failed - \'" << "\'" << std::endl;
//
//    }
//
//    // Will return GIF_ERROR if gif data structure cannot be populated
//    if (DGifSlurp(gif_data) == GIF_ERROR) {
//        std::cout << "Failed to load image \' - " << "\'" << std::endl;
//    }
//    this->animated = (gif_data->ImageCount > 1);
//    this->renderer = renderer;
//    this->w = gif_data->SWidth;
//    this->h = gif_data->SHeight;
//
//    this->frame_count = gif_data->ImageCount;
//
//    if (gif_data->SColorMap) {
//        // NOTE: IN BITS
//        this->depth = gif_data->SColorMap->BitsPerPixel;
//
//        /* I came across an example gif which was reported as 6 BPP by giflib.		*/
//        /* However, Windows reported it as 8 BPP and when set as 8 BPP, it loaded.	*/
//        /* In conclusion, I'm going to treat everything as a multiple of 8.			*/
//        if (this->depth % 8) {
//            this->depth = 8;
//        }
//    }
//    else {
//        // No global palette, assume 8 BPP depth
//        this->depth = 8;
//    }
//
//
//    // Create surface with existing gif data. 8 bit depth will trigger automatic creation of palette to be filled next
//    this->surface = SDL_CreateRGBSurfaceFrom((void*)gif_data->SavedImages[0].RasterBits, this->w, this->h, this->depth, this->w * (this->depth >> 3), 0, 0, 0, 0);
//
//    if (gif_data->SColorMap) { // if global colour palette defined
//        // convert from global giflib colour to SDL colour and populate palette
//        setPalette(gif_data->SColorMap, this->surface);
//    }
//    else if (gif_data->SavedImages[this->frame_index].ImageDesc.ColorMap) { // local colour palette
//        // convert from local giflib colour to SDL colour and populate palette
//        setPalette(gif_data->SavedImages[this->frame_index].ImageDesc.ColorMap, this->surface);
//    }
//
//    /* Convert to a more friendly format */
//    SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGB888);
//    SDL_Surface* output = SDL_ConvertSurface(surface, format, 0);
//    SDL_FreeFormat(format);
//    SDL_FreeSurface(surface);
//    surface = output;
//
//    this->backgroundTexture_ = SDL_CreateTextureFromSurface(this->renderer, this->surface);
//
//    if (this->animated) {
//        if (this->prerendered) prerender();
//        animationThread = std::thread(&Component::animating, this);
//    }
//}


// END GIF SUPPORT 
void Component::deInitializeFonts()
{
}


void Component::initializeFonts()
{
}


void Component::triggerEvent(std::string event, int menuIndex)  
{
    animationRequestedType_ = event;
    animationRequested_     = true;
    menuIndex_              = (menuIndex > 0 ? menuIndex : 0);
}

void Component::setPlaylist(std::string name)
{
    this->playlistName = name;
}

void Component::setNewItemSelected()
{
    newItemSelected = true;
}

void Component::setNewScrollItemSelected()
{
    newScrollItemSelected = true;
}

void Component::setId( int id )
{
    id_ = id;
}

bool Component::isIdle()
{
    return (currentTweenComplete_ || animationType_ == "idle" || animationType_ == "menuIdle" || animationType_ == "attract");
}

bool Component::isAttractIdle()
{
    return (currentTweenComplete_ || animationType_ == "idle" || animationType_ == "menuIdle");
}

bool Component::isMenuScrolling()
{
    return (!currentTweenComplete_ && animationType_ == "menuScroll");
}

void Component::setTweens(AnimationEvents *set)
{
    tweens_ = set;
}

void Component::update(float dt)
{
    elapsedTweenTime_ += dt;

    if ( animationRequested_ && animationRequestedType_ != "" )
    {
      Animation *newTweens;
      // Check if this component is part of an active scrolling list
      if ( menuIndex_ >= MENU_INDEX_HIGH )
      {
          // Check for animation at index i
          newTweens = tweens_->getAnimation( animationRequestedType_, MENU_INDEX_HIGH );
          if ( !(newTweens && newTweens->size() > 0) )
          {
              // Check for animation at the current menuIndex
              newTweens = tweens_->getAnimation( animationRequestedType_, menuIndex_ - MENU_INDEX_HIGH);
          }
      }
      else
      {
          // Check for animation at the current menuIndex
          newTweens = tweens_->getAnimation( animationRequestedType_, menuIndex_ );
      }
      if (newTweens && newTweens->size() > 0)
      {
        animationType_        = animationRequestedType_;
        currentTweens_        = newTweens;
        currentTweenIndex_    = 0;
        elapsedTweenTime_     = 0;
        storeViewInfo_        = baseViewInfo;
        currentTweenComplete_ = false;
      }
      animationRequested_   = false;
    }

    if (tweens_ && currentTweenComplete_)
    {
        animationType_        = "idle";
        currentTweens_        = tweens_->getAnimation( "idle", menuIndex_ );
        if ( currentTweens_ && currentTweens_->size( ) == 0 && !page.isMenuScrolling( ) )
        {
            currentTweens_    = tweens_->getAnimation( "menuIdle", menuIndex_ );
            if ( currentTweens_ && currentTweens_->size( ) > 0 )
            {
                currentTweens_ = currentTweens_;
            }
        }
        currentTweenIndex_    = 0;
        elapsedTweenTime_     = 0;
        storeViewInfo_        = baseViewInfo;
        currentTweenComplete_ = false;
        animationRequested_   = false;
    }

    currentTweenComplete_ = animate();
    if ( currentTweenComplete_ )
    {
      currentTweens_     = NULL;
      currentTweenIndex_ = 0;
    }
}

void Component::draw()
{

    if (backgroundTexture_)
    {
        //SDL_Rect rect;
        rect.h = static_cast<int>(baseViewInfo.ScaledHeight());
        rect.w = static_cast<int>(baseViewInfo.ScaledWidth());
        rect.x = static_cast<int>(baseViewInfo.XRelativeToOrigin());
        rect.y = static_cast<int>(baseViewInfo.YRelativeToOrigin());


        SDL_SetTextureColorMod(backgroundTexture_,
            static_cast<char>(baseViewInfo.BackgroundRed * 255),
            static_cast<char>(baseViewInfo.BackgroundGreen * 255),
            static_cast<char>(baseViewInfo.BackgroundBlue * 255));

        SDL::renderCopy(backgroundTexture_, baseViewInfo.BackgroundAlpha, NULL, &rect, baseViewInfo, page.getLayoutWidth(baseViewInfo.Monitor), page.getLayoutHeight(baseViewInfo.Monitor));
         
        
    }

   
}



bool Component::animate()
{
    bool completeDone = false;
    if ( !currentTweens_ || currentTweenIndex_ >= currentTweens_->size() )
    {
        completeDone = true;
    }
    else if ( currentTweens_ )
    {
        bool currentDone = true;
        TweenSet *tweens = currentTweens_->tweenSet(currentTweenIndex_);

        for(unsigned int i = 0; i < tweens->size(); i++)
        {
            Tween *tween = tweens->tweens()->at(i);
            double elapsedTime = elapsedTweenTime_;

            //todo: too many levels of nesting
            if ( elapsedTime < tween->duration )
                currentDone = false;
            else
                elapsedTime = static_cast<float>(tween->duration);

            switch(tween->property)
            {
            case TWEEN_PROPERTY_X:
                if (tween->startDefined)
                    baseViewInfo.X = tween->animate(elapsedTime);
                else
                    baseViewInfo.X = tween->animate(elapsedTime, storeViewInfo_.X);
                break;

            case TWEEN_PROPERTY_Y:
                if (tween->startDefined)
                    baseViewInfo.Y = tween->animate(elapsedTime);
                else
                    baseViewInfo.Y = tween->animate(elapsedTime, storeViewInfo_.Y);
                break;

            case TWEEN_PROPERTY_HEIGHT:
                if (tween->startDefined)
                    baseViewInfo.Height = tween->animate(elapsedTime);
                else
                    baseViewInfo.Height = tween->animate(elapsedTime, storeViewInfo_.Height);
                break;

            case TWEEN_PROPERTY_WIDTH:
                if (tween->startDefined)
                    baseViewInfo.Width = tween->animate(elapsedTime);
                else
                    baseViewInfo.Width = tween->animate(elapsedTime, storeViewInfo_.Width);
                break;

            case TWEEN_PROPERTY_ANGLE:
                if (tween->startDefined)
                    baseViewInfo.Angle = tween->animate(elapsedTime);
                else
                    baseViewInfo.Angle = tween->animate(elapsedTime, storeViewInfo_.Angle);
                break;

            case TWEEN_PROPERTY_ALPHA:
                if (tween->startDefined)
                    baseViewInfo.Alpha = tween->animate(elapsedTime);
                else
                    baseViewInfo.Alpha = tween->animate(elapsedTime, storeViewInfo_.Alpha);
                break;

            case TWEEN_PROPERTY_X_ORIGIN:
                if (tween->startDefined)
                    baseViewInfo.XOrigin = tween->animate(elapsedTime);
                else
                    baseViewInfo.XOrigin = tween->animate(elapsedTime, storeViewInfo_.XOrigin);
                break;

            case TWEEN_PROPERTY_Y_ORIGIN:
                if (tween->startDefined)
                    baseViewInfo.YOrigin = tween->animate(elapsedTime);
                else
                    baseViewInfo.YOrigin = tween->animate(elapsedTime, storeViewInfo_.YOrigin);
                break;

            case TWEEN_PROPERTY_X_OFFSET:
                if (tween->startDefined)
                    baseViewInfo.XOffset = tween->animate(elapsedTime);
                else
                    baseViewInfo.XOffset = tween->animate(elapsedTime, storeViewInfo_.XOffset);
                break;

            case TWEEN_PROPERTY_Y_OFFSET:
                if (tween->startDefined)
                    baseViewInfo.YOffset = tween->animate(elapsedTime);
                else
                    baseViewInfo.YOffset = tween->animate(elapsedTime, storeViewInfo_.YOffset);
                break;

            case TWEEN_PROPERTY_FONT_SIZE:
                if (tween->startDefined)
                    baseViewInfo.FontSize = tween->animate(elapsedTime);
                else
                    baseViewInfo.FontSize = tween->animate(elapsedTime, storeViewInfo_.FontSize);
                break;

            case TWEEN_PROPERTY_BACKGROUND_ALPHA:
                if (tween->startDefined)
                    baseViewInfo.BackgroundAlpha = tween->animate(elapsedTime);
                else
                    baseViewInfo.BackgroundAlpha = tween->animate(elapsedTime, storeViewInfo_.BackgroundAlpha);
                break;

            case TWEEN_PROPERTY_MAX_WIDTH:
                if (tween->startDefined)
                    baseViewInfo.MaxWidth = tween->animate(elapsedTime);
                else
                    baseViewInfo.MaxWidth = tween->animate(elapsedTime, storeViewInfo_.MaxWidth);
                break;

            case TWEEN_PROPERTY_MAX_HEIGHT:
                if (tween->startDefined)
                    baseViewInfo.MaxHeight = tween->animate(elapsedTime);
                else
                    baseViewInfo.MaxHeight = tween->animate(elapsedTime, storeViewInfo_.MaxHeight);
                break;

            case TWEEN_PROPERTY_LAYER:
                if (tween->startDefined)
                    baseViewInfo.Layer = static_cast<unsigned int>(tween->animate(elapsedTime));
                else
                    baseViewInfo.Layer = static_cast<unsigned int>(tween->animate(elapsedTime, storeViewInfo_.Layer));
                break;

            case TWEEN_PROPERTY_CONTAINER_X:
                if (tween->startDefined)
                    baseViewInfo.ContainerX = tween->animate(elapsedTime);
                else
                    baseViewInfo.ContainerX = tween->animate(elapsedTime, storeViewInfo_.ContainerX);
                break;

            case TWEEN_PROPERTY_CONTAINER_Y:
                if (tween->startDefined)
                    baseViewInfo.ContainerY = tween->animate(elapsedTime);
                else
                    baseViewInfo.ContainerY = tween->animate(elapsedTime, storeViewInfo_.ContainerY);
                break;

            case TWEEN_PROPERTY_CONTAINER_WIDTH:
                if (tween->startDefined)
                    baseViewInfo.ContainerWidth = tween->animate(elapsedTime);
                else
                    baseViewInfo.ContainerWidth = tween->animate(elapsedTime, storeViewInfo_.ContainerWidth);
                break;

            case TWEEN_PROPERTY_CONTAINER_HEIGHT:
                if (tween->startDefined)
                    baseViewInfo.ContainerHeight = tween->animate(elapsedTime);
                else
                    baseViewInfo.ContainerHeight = tween->animate(elapsedTime, storeViewInfo_.ContainerHeight);
                break;

            case TWEEN_PROPERTY_VOLUME:
                if (tween->startDefined)
                    baseViewInfo.Volume = tween->animate(elapsedTime);
                else
                    baseViewInfo.Volume = tween->animate(elapsedTime, storeViewInfo_.Volume);
                break;

            case TWEEN_PROPERTY_MONITOR:
                if (tween->startDefined)
                    baseViewInfo.Monitor = static_cast<unsigned int>(tween->animate(elapsedTime));
                else
                    baseViewInfo.Monitor = static_cast<unsigned int>(tween->animate(elapsedTime, storeViewInfo_.Monitor));
                break;

            case TWEEN_PROPERTY_NOP:
                break;
            }
        }

        if ( currentDone )
        {
            currentTweenIndex_++;
            elapsedTweenTime_ = 0;
            storeViewInfo_    = baseViewInfo;
        }
    }

    if ( !currentTweens_ || currentTweenIndex_ >= currentTweens_->tweenSets()->size() )
    {
        completeDone = true;
    }

    return completeDone;
}


bool Component::isPlaying()
{
    return false;
}




bool Component::isJukeboxPlaying()
{
    return false;
}


void Component::setMenuScrollReload(bool menuScrollReload)
{
    menuScrollReload_ = menuScrollReload;
}


bool Component::getMenuScrollReload()
{
    return menuScrollReload_;
}

int Component::getId( )
{
    return id_;
}
