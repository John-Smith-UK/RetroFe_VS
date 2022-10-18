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
#pragma once

#include <vector>

#include "../../SDL.h"
#include "../Page.h"
#include "../ViewInfo.h"
#include "../Animate/Tween.h"
#include "../Animate/AnimationEvents.h"
#include "../../Collection/Item.h"
#include "gif_lib.h"
#include <thread>
#include <memory>

#define GIF_MIN_DELAY 0x02   //GIF SUPPORT


class Component
{
public:

    Component(Page &p);
    Component(const Component &copy);
    virtual ~Component();
    virtual void freeGraphicsMemory();
    virtual void allocateGraphicsMemory();
    virtual void deInitializeFonts();
    virtual void initializeFonts();
    void triggerEvent(std::string event, int menuIndex = -1);
    void setPlaylist(std::string name );
    void setNewItemSelected();
    void setNewScrollItemSelected();
    bool isIdle();
    bool isAttractIdle();
    bool isMenuScrolling();
    bool newItemSelected;
    bool newScrollItemSelected;
    void setId( int id );

    virtual void update(float dt);
    virtual void draw();
   
    void setTweens(AnimationEvents *set);
    virtual bool isPlaying();
    virtual bool isJukeboxPlaying();
    virtual void skipForward( ) {};
    virtual void skipBackward( ) {};
    virtual void skipForwardp( ) {};
    virtual void skipBackwardp( ) {};
    virtual void pause( ) {};
    virtual void restart( ) {};
    virtual unsigned long long getCurrent( ) {return 0;};
    virtual unsigned long long getDuration( ) {return 0;};
    virtual bool isPaused( ) {return false;};
    ViewInfo baseViewInfo;
    std::string collectionName;
    void setMenuScrollReload(bool menuScrollReload);
    bool getMenuScrollReload();
    virtual void setText(std::string text, int id = -1) {};
    virtual void setImage(std::string filePath, int id = -1) {};
    int getId( );
    
   // GIF SUPPORT

    


    int w, h;
    uint8_t depth = 0;
    uint16_t frame_index = 0;
    uint16_t delay_val = 0;
    std::vector<SDL_Texture*> frames;
    uint16_t frame_count = 0;
    bool animated = false;   //PROBLABLY WRONG PLACE
    bool ready = false;     //PROBLABLY WRONG PLACE
    

   
  
   // virtual void gifdraw(SDL_Texture* imageTexture);

    void setPalette(ColorMapObject* colorMap, SDL_Surface* surface);
    void setIndex(uint16_t index);
    void prepare(uint16_t index);
   // void animatedImage(std::string file);
    void prepare();
    uint16_t getDelay(uint16_t index);

    uint16_t getDelay();

    ExtensionBlock* getGraphicsBlock(uint16_t index);

    void animating();

    void prerender();
    GifFileType* gif_data = nullptr;
    SDL_Surface* surface = nullptr;
    

#ifdef MAKE_NO_PRERENDER
    bool prerendered = false;
#else
    bool prerendered = true;
#endif

    std::thread animationThread;

    bool play = true;
    bool quit = false;

   // END GIF SUPPORT   
protected:
    Page &page;

    std::string playlistName;
    SDL_Renderer* renderer = SDL::getRenderer(baseViewInfo.Monitor); //gif
private:

    //GIF SUPPORT



    

    // END GIF SUPPORT

    bool animate();
    bool tweenSequencingComplete();

    AnimationEvents *tweens_;
    Animation *currentTweens_;
    SDL_Texture *backgroundTexture_;
    SDL_Rect rect;
    SDL_Rect grect;
        
  


    ViewInfo     storeViewInfo_;
    unsigned int currentTweenIndex_;
    bool         currentTweenComplete_;
    float        elapsedTweenTime_;
    std::string  animationRequestedType_;
    std::string  animationType_;
    bool         animationRequested_;
    bool         menuScrollReload_;
    int          menuIndex_;
    int          id_;
    
    // GIF SUPPORT
    

    // END GIF SUPPORT 
};
