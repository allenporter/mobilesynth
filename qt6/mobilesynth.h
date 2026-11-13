// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MOBILESYNTH_H
#define MOBILESYNTH_H

#include <QtQmlIntegration/qqmlintegration.h>

#include <QAudioSink>
#include <QByteArray>
#include <QIODevice>
#include <QMediaDevices>
#include <QObject>
#include <QScopedPointer>
#include <QTimer>

#include <polymobilesynth/qt6/qt68wraper.h>

#include <math.h>

class MobileSynth : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool clip READ get_clip NOTIFY sampleUpdated);
    Q_PROPERTY(qint64 clipLen READ get_clip_len NOTIFY sampleUpdated);
    Q_PROPERTY(qreal peak READ get_peak NOTIFY sampleUpdated);
    Q_PROPERTY(int readDataLen READ get_read_data_len NOTIFY sampleUpdated);
    Q_PROPERTY(int sampleRate READ get_sample_rate NOTIFY formatUpdated);
    Q_PROPERTY(int channelBytes READ get_channel_bytes NOTIFY formatUpdated);
    Q_PROPERTY(int channelCount READ get_channel_count NOTIFY formatUpdated);
    Q_PROPERTY(int sampleFormat READ get_sample_format NOTIFY formatUpdated);
    Q_PROPERTY(int sampleLittleEndian READ get_sample_little_endian NOTIFY formatUpdated);
    Q_PROPERTY(int bufferSize READ get_buffer_size NOTIFY bufferSizeChanged);
    Q_PROPERTY(int bufferBytesFree READ get_buffer_bytes_free NOTIFY sampleUpdated);
    Q_PROPERTY(int bufferFrameCount READ get_buffer_frame_count NOTIFY formatUpdated);

    Q_PROPERTY(int audioState READ get_state NOTIFY stateChanged);
    Q_PROPERTY(bool pullMode MEMBER m_pullMode NOTIFY pullModeChanged);
    Q_PROPERTY(int audioDeviceIndex MEMBER m_audio_device_index NOTIFY formatUpdated);

    Q_PROPERTY(QStringList deviceList READ deviceList() CONSTANT);

    QML_ELEMENT
    QML_UNCREATABLE("Cannot instantiate MobileSynth")

public:
    MobileSynth();
    ~MobileSynth();

    Q_INVOKABLE void noteOn(int vid, float f);
    Q_INVOKABLE void noteOff(int vid);
    Q_INVOKABLE void set_arpeggio_enabled(bool value);
    Q_INVOKABLE void set_arpeggio_samples(int value);
    Q_INVOKABLE void set_arpeggio_octaves(int value);
    Q_INVOKABLE void set_arpeggio_step(int value);
    Q_INVOKABLE void set_filter_cutoff(int value);
    Q_INVOKABLE void set_filter_resonance(float value);
    Q_INVOKABLE void set_osc1_level(float value);
    Q_INVOKABLE void set_osc1_wave_type(int value);
    Q_INVOKABLE void set_osc1_octave(int octave);
    Q_INVOKABLE void set_osc2_level(float value);
    Q_INVOKABLE void set_osc2_wave_type(int value);
    Q_INVOKABLE void set_osc2_octave(int octave);
    Q_INVOKABLE void set_osc2_shift(int value);
    Q_INVOKABLE void set_osc_sync(bool v);
    Q_INVOKABLE void set_glide_samples(long v);
    Q_INVOKABLE void set_modulation_source(int value);
    Q_INVOKABLE void set_modulation_destination(int value);
    Q_INVOKABLE void set_modulation_amount(float value);
    Q_INVOKABLE void set_modulation_amount(int voiceId, float value);
    Q_INVOKABLE void set_modulation_frequency(float value);
    Q_INVOKABLE void set_volume_attack(long value);
    Q_INVOKABLE void set_volume_decay(long value);
    Q_INVOKABLE void set_volume_sustain(float value);
    Q_INVOKABLE void set_volume_release(long value);
    Q_INVOKABLE void set_filter_attack(long value);
    Q_INVOKABLE void set_filter_decay(long value);
    Q_INVOKABLE void set_filter_sustain(float value);
    Q_INVOKABLE void set_filter_release(long value);
    Q_INVOKABLE void pitch(int vid, float f);
    Q_INVOKABLE void deviceChanged(int index);
    Q_INVOKABLE void suspendAudio();
    Q_INVOKABLE void resumeAudio();
    Q_INVOKABLE void pull_mode();
    Q_INVOKABLE void push_mode();
    Q_INVOKABLE void set_buffer_size(int v);

    //! Mix an additional generator (e.g. a soundfont player) into the output
    void addAudioSource(AudioSource *source);
    void removeAudioSource(AudioSource *source);

signals:
    void sampleUpdated();
    void formatUpdated();
    void stateChanged();
    void pullModeChanged();
    void bufferSizeChanged();

private:
    void initializeAudio();

private:
    QMediaDevices *m_devices = nullptr;
    QTimer *m_pushTimer = nullptr;

    QAudioDevice m_device_info;
    int m_buffer_size;

    QScopedPointer<Qt68Wraper> m_generator;
    QScopedPointer<QAudioSink> m_audioOutput;

    bool m_pullMode;

    //! Audio device index
    /*!
      Contains the index of the activ audio device in the deviceList
      Is used to select the default device in the AudioDevice.qml view
    */
    int m_audio_device_index;

    synth::Oscillator::WaveType int2wavetype(int value);
    synth::Controller::OctaveShift int2octaveshift(int value);

    qint64 get_read_data_len();
    bool get_clip();
    qint64 get_clip_len();
    qreal get_peak();
    int get_buffer_size();
    int get_buffer_bytes_free();
    int get_buffer_frame_count();
    int get_sample_rate();
    int get_channel_bytes();
    int get_channel_count();
    int get_sample_format();
    bool get_sample_little_endian();
    int get_state();
    void set_audio_device_index();

    QStringList deviceList();

private slots:
    void volumeChanged(int);
};

#endif // MOBILESYNTH_H
