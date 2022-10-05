/*NICK WILSON, ADAPTATION*/

/*NOW NEEDS MERGE WITH COMPONNET.CPP/H */

#include <thread>

#include "Component.h"
#include <SDL.h>
#include <string>
#include "../../Utility/Utils.h"
#include "gif_lib.h"	//gif support
#include "Image.h"	//base class

#ifndef ANIMATEDIMAGE_H
#define ANIMATEDIMAGE_H

#define GIF_MIN_DELAY 0x02

class AnimatedImage : /*public Image */public Component
{
	//COPY FROM ORIGINAL IMAGE 
public:
	AnimatedImage(std::string file, std::string altFile, Page& p, int monitor);
	virtual ~AnimatedImage();
	void freeGraphicsMemory();
	void allocateGraphicsMemory();
	void draw(); 
	// END COPY  

	//PROBLABLY WRONG H FILE
	enum state {
		STATE_PLAY,
		STATE_PAUSE,
		STATE_TOGGLE,
	};
	uint16_t w, h;
	bool animated = false;
	bool ready = false;
	//virtual void set_status([[maybe_unused]] state s) {};
// END PROBLABLY

	uint16_t frame_count = 0;
	bool playable;

	//AnimatedImage() {}   DISABLING IT

	// AnimatedImage(SDL_Renderer* renderer, std::filesystem::path path);  // REPLACE BY LINE 21

	// ~AnimatedImage(); // REPLACE BY LINE 22

	void prepare(uint16_t index);

	void prepare();

	
	// DISABLE IT AS IT SHOULD BE PLAY
	/*void set_status(AnimatedImage::state s);*/

	enum EXCEPT {
		EXCEPT_IMG_LOAD_FAIL,
		EXCEPT_IMG_OPEN_FAIL
	};
private:

	GifFileType* gif_data = nullptr;
	SDL_Surface* surface = nullptr;

	uint8_t depth = 0;
	uint16_t frame_index = 0;
	uint16_t delay_val = 0;

#ifdef MAKE_NO_PRERENDER
	bool prerendered = false;
#else
	bool prerendered = true;
#endif

	std::vector<SDL_Texture*> frames;

	bool play = true;
	bool quit = false;

	std::thread animationThread;

	void setPalette(ColorMapObject* colorMap, SDL_Surface* surface);

	void setIndex(uint16_t index);

	uint16_t getDelay(uint16_t index);

	uint16_t getDelay();

	ExtensionBlock* getGraphicsBlock(uint16_t index);

	void animating();

	void prerender();


	protected:

	SDL_Texture* texture_;
	std::string  file_;
	std::string  altFile_;
	
};





#endif
