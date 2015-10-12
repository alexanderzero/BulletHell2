#include "audio.hpp"

#include "fmod/fmod.hpp"

//AUDIO SYSTEM

class AudioImpl
{
public:
   AudioImpl()
   {
      FMOD::System_Create(&system);
      system->init(512, FMOD_INIT_NORMAL, nullptr);
   }
   ~AudioImpl()
   {
      system->release();
   }
   FMOD::System* system;
};

Audio::Audio() : pImpl(new AudioImpl) {}
Audio::~Audio() {}

void Audio::update()
{
   pImpl->system->update();
}


//SOUNDS

class Sound::Impl
{
public:
   Impl()
   {
      sound = nullptr;
      system = nullptr;
      playingChannel = nullptr;
   }
   ~Impl()
   {
      if (sound) sound->release();
   }
   void play()
   {
      if (!sound) return;
      stop();
      system->playSound(sound, nullptr, false, &playingChannel);
   }
   void stop()
   {
      if (!playingChannel) return;
      playingChannel->stop();
   }

   FMOD::System* system;
   FMOD::Sound* sound;
   FMOD::Channel* playingChannel;
};


Sound::~Sound()
{

}

void Sound::play()
{
   if (!pImpl) return;
   pImpl->play();
}

Sound::Sound()
   : pImpl(new Impl)
{


}
Sound::Sound(Sound&& rhs)
{
   pImpl = std::move(rhs.pImpl);
}
Sound& Sound::operator=(Sound&& rhs)
{
   pImpl = std::move(rhs.pImpl);
   return *this;
}



//CONSTRUCTION

Sound Sound::createSong(Audio& audio, std::string songPath)
{
   //"songs" are streamed in for efficiency.
   Sound out;
   auto fSys = audio.pImpl->system;
   out.pImpl->system = audio.pImpl->system;
   fSys->createStream(songPath.c_str(), FMOD_DEFAULT | FMOD_LOOP_NORMAL, nullptr, &out.pImpl->sound);
   return out;
}