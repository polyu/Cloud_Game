#include "stdafx.h"
#include "StreamClient.h"
#include "Decoder.h"
#include <SDL.h>
#ifdef main
#undef main
#endif 
#define DEFAULTPORT 1234

int main(int argv,char **argc)
{
	if(!setupStreamClient(DEFAULTPORT))
	{
		printf("Could not initialize NETWORK!.\n");
        exit(-1);
	}
	if(!initDecoder())
	{
		printf("Could not initialize Decoder!.\n");
        exit(-1);
	}
	if((SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)==-1)) 
	{ 
        printf("Could not initialize SDL: %s.\n", SDL_GetError());
        exit(-1);
    }
	SDL_Surface *screen;
	atexit(SDL_Quit);
	screen = SDL_SetVideoMode(RWIDTH, RHEIGHT, 32, SDL_SWSURFACE);
	if ( screen == NULL ) {
        printf("Couldn't set 640x480x8 video mode: %s\n",
                        SDL_GetError());
        exit(1);
    }
	SDL_Overlay *screenOverlay=SDL_CreateYUVOverlay(RWIDTH,RHEIGHT,SDL_YV12_OVERLAY,screen);
	bool quitFlag=false;
	SDL_Event event;
	SDL_Rect        rect;  
	pair<char*,int> data;
	AVFrame* frame;
	while(!quitFlag)
	{
		 if(recvData(data))
		 {
			if(decode_frame(data,&frame))
			{
				 SDL_LockYUVOverlay(screenOverlay);
				 screenOverlay->pixels[0]=frame->data[0];
				 screenOverlay->pixels[1]=frame->data[1];
				 screenOverlay->pixels[2]=frame->data[2];
				 screenOverlay->pitches[0]=frame->linesize[0];
				 screenOverlay->pitches[1]=frame->linesize[1];
				 screenOverlay->pitches[2]=frame->linesize[2];
				 SDL_UnlockYUVOverlay(screenOverlay);
				 rect.w = RWIDTH;  
                 rect.h = RHEIGHT;  
                 SDL_DisplayYUVOverlay(screenOverlay, &rect);
			}
		 }
		 while( SDL_PollEvent( &event ) )
		 {
                switch( event.type )
				{
                   
                    case SDL_KEYDOWN:
                    case SDL_KEYUP:
                       
                        break;
                    case SDL_QUIT:
                        quitFlag = true;
                        break;
                    default:
                        break;
				 }
		 }
	}
	return 0;
}