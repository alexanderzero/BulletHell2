#pragma once

#include <memory>
#include <string>

class SoundImpl;
class Audio;

class Sound
{
public:
   Sound();
   ~Sound();
   Sound(Sound&& rhs);
   Sound& operator=(Sound&& rhs);

   static Sound createSong(Audio& audio, std::string songPath);
   static Sound createSample(Audio& audio, std::string samplePath);

   //global context
   static Sound createSong(std::string songPath);
   static Sound createSample(std::string samplePath);

   //clone a copy of this sound.... typical use is to have more than one instance of a sound effect.
   Sound clone();

   Sound& play();
   void detach(); //continue playing after this sound dies.

private:
   Sound(Audio* audio);


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


//todo: lift this function somerwhere sane
void playBGM(std::string const& path);
void playSFX(std::string const& path);  //one-shot sfx only.  Loads, plays, and removes...