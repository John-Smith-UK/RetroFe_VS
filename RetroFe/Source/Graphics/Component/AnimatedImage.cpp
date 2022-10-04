
/*
NICK WILSON
2020
*/



#include "AnimatedImage.h"


/* PUBLIC */



AnimatedImage::AnimatedImage(std::string file, std::string altFile, Page& p, int monitor)
	: Component(p)
	, texture_(NULL)
	, file_(file)
	, altFile_(altFile)

{
    gif_data = DGifOpenFileName(file_.c_str(), nullptr);

    // Will be null if image metadata could not be read
    if (!gif_data) {
        throw Utils::EXCEPT_IMG_OPEN_FAIL;
    }

    // Will return GIF_ERROR if gif data structure cannot be populated
    if (DGifSlurp(gif_data) == GIF_ERROR) {
        throw Utils::EXCEPT_IMG_LOAD_FAIL;
    }

    this->animated = (gif_data->ImageCount > 1);
    // this->Renderer = SDL::getRenderer(baseViewInfo.Monitor);
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

	baseViewInfo.Monitor = monitor;
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
		SDL_DestroyTexture(texture_);
		texture_ = NULL;

	}
	SDL_UnlockMutex(SDL::getMutex());

}

void AnimatedImage::allocateGraphicsMemory()
{


}

void AnimatedImage::gifdraw()
{


}

/**
* set_status 	- Set image state - play or pause (or toggle to swap)
* s 			> New Image::state state
*/
//void AnimatedImage::set_status(state s) {
//	switch (s) {
//		case STATE_PLAY:
//			this->play = true;
//			break;
//		case STATE_PAUSE:
//			this->play = false;
//			break;
//		case STATE_TOGGLE:
//			this->play = !this->play;
//			break;
//		default:
//			break;
//	}
//}

/*
	TODO:
	- Some troublesome gifs will now no longer play at all
	- Scrambled palette bug still exists on some images
*/

/*
[1]: 	Giflib cuts off the first 3 bytes of extension chunk.
		Payload is then [Packed/Flag Byte], [Upper Delay], [Lower Delay], [Transparency Index].
*/
