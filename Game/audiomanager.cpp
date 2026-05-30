#include "audiomanager.h"

AudioManager* AudioManager::m_audioManager = nullptr;

AudioManager* AudioManager::instance(){
    if(!m_audioManager){
        m_audioManager = new AudioManager;
    }
    return m_audioManager;
}

AudioManager::AudioManager(){

    //循环播放的音效
    players[BOSS_ATMOSPHERE] = new QMediaPlayer;
    players[MOON_ATMOSPHERE] = new QMediaPlayer;

    outputs[BOSS_ATMOSPHERE] = new QAudioOutput;
    outputs[MOON_ATMOSPHERE] = new QAudioOutput;

    players[BOSS_ATMOSPHERE]->setAudioOutput(outputs[BOSS_ATMOSPHERE]);
    players[MOON_ATMOSPHERE]->setAudioOutput(outputs[MOON_ATMOSPHERE]);

    players[BOSS_ATMOSPHERE]->setLoops(QMediaPlayer::Infinite);
    players[MOON_ATMOSPHERE]->setLoops(QMediaPlayer::Infinite);

    players[BOSS_ATMOSPHERE]->setSource(QUrl::fromLocalFile("resource/Sound/boss/boss_bgm.mp3"));
    players[MOON_ATMOSPHERE]->setSource(QUrl::fromLocalFile("resource/Sound/atmosphere/moon_bgm.mp3"));

    //非循环播放音效
    path[SWORD_DRAW] = "resource/Sound/sword/sword_take.mp3";
    path[BOW] = "resource/Sound/bow/arrow_shot.mp3";
    path[CHARACTER_HURT] = "resource/Sound/hurt/character_hurt.mp3";
    path[CHARACTER_DEATH] = "resource/Sound/death/death.mp3";
    path[FIRE] = "resource/Sound/fire/fire.mp3";
    path[BOOM] = "resource/Sound/fire/boom.mp3";
    path[TIME_STOP] = "resource/Sound/skill/time_stop.mp3";
    path[SWORD_ATTACK] = "resource/Sound/sword/sword_attack.mp3";
    path[ENEMY_HURT] = "resource/Sound/hurt/enemy_hurt.mp3";
    path[DOOR_CLOSE] = "resource/Sound/level/close_door.mp3";
    path[VICTORY]= "resource/Sound/level/level_clear.mp3";
    path[TRANSITION] = "resource/Sound/transition/transition.mp3";
    path[BOSS_WARNING] = "resource/Sound/boss/boss_warning.mp3";
    path[LIGHTNING] = "resource/Sound/lightning/lightning.mp3";
    path[HEAL] = "resource/Sound/heal/heal.mp3";
}

void AudioManager::playSound(SoundType type,int times){
    QMediaPlayer* player = new QMediaPlayer;
    QAudioOutput* output = new QAudioOutput;
    output->setVolume(m_sfxVolume);
    player->setAudioOutput(output);
    player->setSource(QUrl::fromLocalFile(path[type]));
    player->play();
    player->setLoops(times);
    connect(player, &QMediaPlayer::playbackStateChanged,
            [this, player, output](QMediaPlayer::PlaybackState state) {
                if (state == QMediaPlayer::StoppedState) {
                    player->deleteLater();
                    output->deleteLater();
                }
            });
    return;
}

void AudioManager::playSound(LoopSoundType type){
    outputs[type]->setVolume(m_bgmVolume);
    players[type]->setLoops(QMediaPlayer::Infinite);
    players[type]->play();
    return;
}

void AudioManager::stopSound(LoopSoundType type){
    players[type]->stop();
    return;
}

AudioManager::~AudioManager(){
    int i = 0;
    while(players[i]){
        delete players[i++];
    }
    i = 0;
    while(outputs[i]){
        delete outputs[i++];
    }
}

