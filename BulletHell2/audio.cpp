#include "audio.hpp"

#include "fmod/fmod.hpp"
#include <vector>
#include <algorithm>

#include "BulletHell2.hpp"


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
      soundRC = nullptr;
   }
   ~SoundImpl()
   {
      stop();
      if (!sound) return;
      if (soundRC && !--*soundRC) return;
      sound->release();
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
   int* soundRC;
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

Sound& Sound::play()
{
   if (!pImpl) return *this;
   pImpl->play();
   return *this;
}

Sound::Sound()
{


}
Sound::Sound(Audio* audio)
   : pImpl(new SoundImpl)
{
   pImpl->system = audio;
}
Sound Sound::clone()
{
   Sound out(pImpl->system);
   if (!pImpl || !pImpl->sound) return out;
   out.pImpl->sound = pImpl->sound;
   
   if (!pImpl->soundRC)
   {
      pImpl->soundRC = new int(1);
   }

   out.pImpl->soundRC = pImpl->soundRC;
   *pImpl->soundRC++;

   return out;
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
   Sound out(&audio);
   auto fSys = audio.pImpl->system;
   fSys->createStream(songPath.c_str(), FMOD_DEFAULT | FMOD_LOOP_NORMAL, nullptr, &out.pImpl->sound);
   return out;
}

Sound Sound::createSample(Audio& audio, std::string samplePath)
{
   Sound out(&audio);
   auto fSys = audio.pImpl->system;
   fSys->createSound(samplePath.c_str(), FMOD_CREATESAMPLE | FMOD_DEFAULT, nullptr, &out.pImpl->sound);
   return out;
}
Sound Sound::createSong(std::string songPath) { return createSong(*g_context.audio, std::move(songPath)); }
Sound Sound::createSample(std::string samplePath) { return createSample(*g_context.audio, std::move(samplePath)); }

//HELPERS
void playBGM(std::string const& path)
{
   auto ctxt = &g_context;
   if (ctxt->playingBGM) delete ctxt->playingBGM;
   ctxt->playingBGM = new Sound(Sound::createSong(*ctxt->audio, path));
   ctxt->playingBGM->play();
}
void playSFX(std::string const& path)
{
   auto ctxt = &g_context;
   auto sfx = Sound::createSample(*ctxt->audio, path.c_str());
   sfx.play();
   sfx.detach();
}