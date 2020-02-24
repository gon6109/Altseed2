﻿#include "Sound.h"

namespace Altseed {

Sound::Sound(
        const char16_t* filePath,
        std::shared_ptr<osm::Sound> sound,
        bool isDecompressed)
    : m_filePath(filePath), m_sound(sound), m_isDecompressed(isDecompressed) {

    auto soundMixer = SoundMixer::GetInstance();

    if(m_sound != nullptr) m_sound->AddRef();

    SetInstanceName(__FILE__);
}

Sound::~Sound() {
    auto soundMixer = SoundMixer::GetInstance();
    if(m_sound != nullptr) m_sound->Release();
}

std::shared_ptr<Sound> Sound::Load(const char16_t* path, bool isDecompressed) {
    auto soundMixer = SoundMixer::GetInstance();
    if (soundMixer->m_manager == nullptr) return nullptr;

    // Create static file & null check
    auto staticFile = StaticFile::Create(path);
    if (staticFile == nullptr) return nullptr;

    // Get data & Create OSM sound & null check
    auto sound = CreateSharedPtr(soundMixer->m_manager->CreateSound(staticFile->GetData(), staticFile->GetSize(), isDecompressed));
    if (sound == nullptr) {
        staticFile->Release();
        return nullptr;
    }

    // Create sound & register to container
    auto soundRet = MakeAsdShared<Sound>(path, sound, isDecompressed);
    auto soundContainer = soundMixer->m_resources->GetResourceContainer(ResourceType::Sound);
    auto soundInfo = std::make_shared<ResourceContainer::ResourceInfomation>(soundRet, path);
    soundContainer->Register(path, soundInfo);

    return soundRet;
}

float Sound::GetLoopStartingPoint() const { return m_sound->GetLoopStartingPoint(); }

void Sound::SetLoopStartingPoint(float startingPoint) const { m_sound->SetLoopStartingPoint(startingPoint); }

float Sound::GetLoopEndPoint() const { return m_sound->GetLoopEndPoint(); }

void Sound::SetLoopEndPoint(float endPoint) const { m_sound->SetLoopEndPoint(endPoint); }

bool Sound::GetIsLoopingMode() const { return m_sound->GetIsLoopingMode(); }

void Sound::SetIsLoopingMode(bool isLoopingMode) { m_sound->SetIsLoopingMode(isLoopingMode); }

float Sound::GetLength() { return m_sound->GetLength(); }

bool Sound::Reload() {
    /*
    auto ls = m_sound->GetLoopStartingPoint();
    auto le = m_sound->GetLoopEndPoint();
    auto lm = m_sound->GetIsLoopingMode();

    m_sound = m_manager->CreateSound()
    */

    return false;
}

}  // namespace Altseed