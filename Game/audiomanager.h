#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
class AudioManager : public QObject
{
    Q_OBJECT
public:
    enum SoundType{
        NONE = -1,
        SWORD_DRAW,
        BOW,
        CHARACTER_HURT,
        CHARACTER_DEATH,
        FIRE,
        BOOM,
        TIME_STOP,
        SWORD_ATTACK,
        ENEMY_HURT,
        DOOR_CLOSE,
        VICTORY,
        TRANSITION,
        BOSS_WARNING,
        LIGHTNING,
        HEAL
    };
    enum LoopSoundType{
        None = -1,
        BOSS_ATMOSPHERE,
        MOON_ATMOSPHERE
    };
    static AudioManager* instance();
    void playSound(SoundType type,int times = 1);
    void playSound(LoopSoundType type);
    void stopSound(LoopSoundType type);
    void setBGMVolume(qreal vol) {m_bgmVolume = vol;}
    void setSFXVolume(qreal vol) {m_sfxVolume = vol;}
private:
    AudioManager();
    ~AudioManager();
    static AudioManager* m_audioManager;
    QMediaPlayer* players[100] = {0};
    QAudioOutput* outputs[100] = {0};
    QString path[100];
    qreal m_bgmVolume = 1.0;
    qreal m_sfxVolume = 1.0;
};

#endif
