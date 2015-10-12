#pragma once

#include <memory>
#include <string>

class Audio;

class Sound
{
public:
   ~Sound();
   Sound(Sound&& rhs);
   Sound& operator=(Sound&& rhs);

   void play();
   static Sound createSong(Audio& audio, std::string songPath);

private:
   Sound();

   class Impl;
   std::unique_ptr<Impl> pImpl;
};


class AudioImpl;

class Audio
{
public:
   Audio();
   ~Audio();

   void update();
 
   std::unique_ptr<AudioImpl> pImpl;
};
