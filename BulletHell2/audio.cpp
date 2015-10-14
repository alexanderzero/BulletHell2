#include "audio.hpp"

#include "fmod/fmod.hpp"
#include <vector>
#include <algorithm>




//AUDIO SYSTEM



class AudioImpl
{
public:
   AudioImpl();
   ~AudioImpl();
   void update();
   FMOD::System* system;
   std::vector<std::unique_ptr<SoundImpl> > detachedSounds;
};

class SoundImpl
{
public:
   SoundImpl()
   {
      sound = nullptr;
      system = nullptr;
      playingChannel = nullptr;
   }
   ~SoundImpl()
   {
      if (sound) sound->release();
   }
   void play()
   {
      if (!sound) return;
      stop();
      system->pImpl->system->playSound(sound, nullptr, false, &playingChannel);
   }
   void stop()
   {
      if (!playingChannel) return;
      playingChannel->stop();
   }

   Audio* system;
   FMOD::Sound* sound;
   FMOD::Channel* playingChannel;
};


AudioImpl::AudioImpl()
{
   FMOD::System_Create(&system);
   system->init(512, FMOD_INIT_NORMAL, nullptr);
}
AudioImpl::~AudioImpl()
{
   detachedSounds.clear();
   system->release();
}
void AudioImpl::update()
{
   system->update();

   //kill bad sounds
   detachedSounds.erase(std::remove_if(begin(detachedSounds), end(detachedSounds), [=](std::unique_ptr<SoundImpl>& sound) {
      if (!sound->sound || !sound->playingChannel) return true; 
      bool stillPlaying = false;
      sound->playingChannel->isPlaying(&stillPlaying);
      return !stillPlaying;
   }), detachedSounds.end());
}

Audio::Audio() : pImpl(new AudioImpl) {}
Audio::~Audio() {}

void Audio::update()
{
   pImpl->update();
}


//SOUNDS



Sound::~Sound()
{

}

void Sound::play()
{
   if (!pImpl) return;
   pImpl->play();
}

Sound::Sound()
   : pImpl(new SoundImpl)
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

void Sound::detach()
{
   if (!pImpl) return;
   pImpl->system->pImpl->detachedSounds.push_back(std::move(pImpl));
}

//CONSTRUCTION

Sound Sound::createSong(Audio& audio, std::string songPath)
{
   //"songs" are streamed in for efficiency.
   Sound out;
   auto fSys = audio.pImpl->system;
   out.pImpl->system = &audio;
   fSys->createStream(songPath.c_str(), FMOD_DEFAULT | FMOD_LOOP_NORMAL, nullptr, &out.pImpl->sound);
   return out;
}

Sound Sound::createSample(Audio& audio, std::string samplePath)
{
   Sound out;
   auto fSys = audio.pImpl->system;
   out.pImpl->system = &audio;
   fSys->createSound(samplePath.c_str(), FMOD_CREATESAMPLE | FMOD_DEFAULT, nullptr, &out.pImpl->sound);
   return out;
}