#pragma once

#include <memory>
#include <string>

class SoundImpl;
class Audio;

class Sound
{
public:
   ~Sound();
   Sound(Sound&& rhs);
   Sound& operator=(Sound&& rhs);

   static Sound createSong(Audio& audio, std::string songPath);
   static Sound createSample(Audio& audio, std::string samplePath);

   void play();
   void detach(); //continue playing after this sound dies.

private:
   Sound();

   std::unique_ptr<SoundImpl> pImpl;
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
