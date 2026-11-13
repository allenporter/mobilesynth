// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mobilesynth.h"

#include <QAudioDevice>
#include <QAudioSink>
#include <QDebug>
#include <QtEndian>
#include <QtMath>
#include <QVariant>

void MobileSynth::noteOn(int vid, float f)
{
    m_generator->noteOn(vid,f);
}

void MobileSynth::noteOff(int vid)
{
    m_generator->noteOff(vid);
}

void MobileSynth::set_arpeggio_enabled(bool value)
{
    m_generator->set_arpeggio_enabled(value);
}

void MobileSynth::set_arpeggio_samples(int value)
{
    m_generator->set_arpeggio_samples(value);
}

void MobileSynth::set_arpeggio_octaves(int value)
{
    m_generator->set_arpeggio_octaves(value);
}

void MobileSynth::set_arpeggio_step(int value)
{
    m_generator->set_arpeggio_step(value);
}

void MobileSynth::set_filter_cutoff(int value)
{
    m_generator->set_filter_cutoff(value);
}

void MobileSynth::set_filter_resonance(float value)
{
    m_generator->set_filter_resonance(value);
}

void MobileSynth::set_osc1_level(float value)
{
    m_generator->set_osc1_level(value);
}

void MobileSynth::pitch(int vid, float f)
{
    m_generator->pitch(vid,f);
}

synth::Oscillator::WaveType MobileSynth::int2wavetype(int value) {
    synth::Oscillator::WaveType w;
    switch (value) {
    case 1:
        w=synth::Oscillator::WaveType::TRIANGLE;
        break;
    case 2:
        w=synth::Oscillator::WaveType::SAWTOOTH;
        break;
    case 3:
        w=synth::Oscillator::WaveType::REVERSE_SAWTOOTH;
        break;
    case 4:
        w=synth::Oscillator::WaveType::SQUARE;
        break;
    default:
        w=synth::Oscillator::WaveType::SINE;
        break;
    }
    return w;
}

synth::Controller::OctaveShift MobileSynth::int2octaveshift(int value) {
    synth::Controller::OctaveShift o;
    switch (value) {
    case 1:
        o=synth::Controller::OctaveShift::OCTAVE_2;
        break;
    case 2:
        o=synth::Controller::OctaveShift::OCTAVE_4;
        break;
    case 3:
        o=synth::Controller::OctaveShift::OCTAVE_8;
        break;
    case 4:
        o=synth::Controller::OctaveShift::OCTAVE_16;
        break;
    default:
        o=synth::Controller::OctaveShift::OCTAVE_1;
        break;
    }
    return o;
}

qint64 MobileSynth::get_read_data_len()
{
    return m_generator->get_read_data_len();
}

bool MobileSynth::get_clip()
{
    return m_generator->get_clip();
}

qint64 MobileSynth::get_clip_len()
{
    return m_generator->get_clip_len();
}

qreal MobileSynth::get_peak()
{
    return m_generator->get_peak();
}

int MobileSynth::get_buffer_size()
{
    return m_audioOutput->bufferSize();
}

int MobileSynth::get_buffer_bytes_free()
{
    return m_audioOutput->bytesFree();
}

int MobileSynth::get_buffer_frame_count()
{
    return m_audioOutput->bufferFrameCount();
}

int MobileSynth::get_sample_rate()
{
    return m_generator->get_sample_rate();
}

int MobileSynth::get_channel_bytes()
{
    return m_generator->get_channel_bytes();
}

int MobileSynth::get_channel_count()
{
    return m_generator->get_channel_count();
}

int MobileSynth::get_sample_format()
{
    return m_generator->get_sample_format();
}

bool MobileSynth::get_sample_little_endian()
{
    return m_generator->get_sample_little_endian();
}

void MobileSynth::set_osc1_wave_type(int value)
{
    m_generator->set_osc1_wave_type(int2wavetype(value));
}

void MobileSynth::set_osc1_octave(int octave)
{
    m_generator->set_osc1_octave(int2octaveshift(octave));
}

void MobileSynth::set_osc2_level(float value)
{
    m_generator->set_osc2_level(value);
}

void MobileSynth::set_osc2_wave_type(int value)
{
    m_generator->set_osc2_wave_type(int2wavetype(value));
}

void MobileSynth::set_osc2_octave(int octave)
{
    m_generator->set_osc2_octave(int2octaveshift(octave));
}

void MobileSynth::set_osc2_shift(int value)
{
    m_generator->set_osc2_shift(value);
}

void MobileSynth::set_osc_sync(bool v)
{
    m_generator->set_osc_sync(v);
}

void MobileSynth::set_glide_samples(long v)
{
    m_generator->set_glide_samples(v);
}

void MobileSynth::set_modulation_source(int value)
{
    m_generator->set_modulation_source(static_cast<synth::Controller::ModulationSource>(value));
}

void MobileSynth::set_modulation_destination(int value)
{
    m_generator->set_modulation_destination(static_cast<synth::Controller::ModulationDestination>(value));
}

void MobileSynth::set_modulation_amount(float value)
{
    m_generator->set_modulation_amount(value);
}

void MobileSynth::set_modulation_amount(int voiceId, float value)
{
    m_generator->set_modulation_amount(voiceId,value);
}

void MobileSynth::set_modulation_frequency(float value)
{
    m_generator->set_modulation_frequency(value);
}

void MobileSynth::set_volume_attack(long value)
{
    m_generator->set_volume_attack(value);
}

void MobileSynth::set_volume_decay(long value)
{
    m_generator->set_volume_decay(value);
}

void MobileSynth::set_volume_sustain(float value)
{
    m_generator->set_volume_sustain(value);
}

void MobileSynth::set_volume_release(long value)
{
    m_generator->set_volume_release(value);
}

void MobileSynth::set_filter_attack(long value)
{
    m_generator->set_filter_attack(value);
}

void MobileSynth::set_filter_decay(long value)
{
    m_generator->set_filter_decay(value);
}

void MobileSynth::set_filter_sustain(float value)
{
    m_generator->set_filter_sustain(value);
}

void MobileSynth::set_filter_release(long value)
{
    m_generator->set_filter_release(value);
}

MobileSynth::MobileSynth() : m_devices(new QMediaDevices(this)), m_pushTimer(new QTimer(this))
{
    m_generator.reset(new Qt68Wraper());

    QObject::connect(m_generator.get(), &Qt68Wraper::sampleUpdated, this, [this]{emit sampleUpdated();});

    m_device_info = m_devices->defaultAudioOutput();
    set_audio_device_index();

    m_buffer_size = 512;
    m_pullMode = true;
    initializeAudio();
}

MobileSynth::~MobileSynth()
{
    m_pushTimer->stop();
}

void MobileSynth::initializeAudio()
{
    QAudioFormat format = m_device_info.preferredFormat();

    qDebug() << "defaultAudioOutput: " << m_device_info.id() << " " << m_device_info.description() ;
    qDebug() << "Preferred format is sr " << format.sampleRate() << " sn " << format.bytesPerSample() << " ch " << format.channelCount() << " sample format " << format.sampleFormat();

    m_audioOutput.reset(new QAudioSink(m_device_info, format));

    QObject::connect(m_audioOutput.get(), &QAudioSink::stateChanged, this, [this]{emit stateChanged();});

    m_generator->setFormat(format);
    m_generator->start();

    m_audioOutput->setBufferSize(m_buffer_size);

    if(m_pullMode) {
        pull_mode();
    } else {
        push_mode();
    }

    emit formatUpdated();

    /*
    qreal initialVolume = QAudio::convertVolume(m_audioOutput->volume(), QAudio::LinearVolumeScale,
                                                QAudio::LogarithmicVolumeScale);
    m_volumeSlider->setValue(qRound(initialVolume * 100));
    */
}

int MobileSynth::get_state()
{
    return m_audioOutput->state();
}

void MobileSynth::set_audio_device_index()
{
    const QList<QAudioDevice> devices = m_devices->audioOutputs();
    int index=0;
    for (const QAudioDevice &deviceInfo : devices) {
        if(deviceInfo.description()==m_device_info.description()) {
            m_audio_device_index=index;
            break;
        }
        index++;
    }
}

void MobileSynth::deviceChanged(int index)
{
    qDebug() << "deviceChanged " << index;

    m_generator->stop();
    m_audioOutput->stop();

    m_device_info = m_devices->audioOutputs().at(index);
    m_audio_device_index = index;

    initializeAudio();
}

void MobileSynth::volumeChanged(int value)
{
    qreal linearVolume = QAudio::convertVolume(value / qreal(100), QAudio::LogarithmicVolumeScale,
                                               QAudio::LinearVolumeScale);

    m_audioOutput->setVolume(linearVolume);
}

QStringList MobileSynth::deviceList()
{
    QStringList l;

    const QList<QAudioDevice> devices = m_devices->audioOutputs();
    for (const QAudioDevice &deviceInfo : devices) {
        l.append(deviceInfo.description());
    }
    return l;
}

void MobileSynth::pull_mode()
{
    m_pushTimer->stop();

    // Reset audiosink
    m_audioOutput->reset();

    // switch to pull mode (QAudioSink pulls from Generator as needed)
    m_audioOutput->start(m_generator.data());

    m_pullMode = true;

    emit pullModeChanged();
}


void MobileSynth::push_mode()
{
    m_pushTimer->stop();

    // Reset audiosink
    m_audioOutput->reset();

    // switch to push mode (periodically push to QAudioSink using a timer)
    auto io = m_audioOutput->start();
    m_pushTimer->disconnect();

    QObject::connect(m_pushTimer, &QTimer::timeout, this, [this, io]() {
        if (m_audioOutput->state() == QAudio::StoppedState)
            return;

        int len = m_audioOutput->bytesFree();
        QByteArray buffer(len, 0);
        len = m_generator->read(buffer.data(), len);
        if (len)
            io->write(buffer.data(), len);
    });

    m_pushTimer->start(10);

    m_pullMode = false;
    emit pullModeChanged();
}

void MobileSynth::set_buffer_size(int v)
{
    qDebug() << "set_buffer_size " << v;
    m_buffer_size = v * get_channel_count();
    m_generator->setBufferSize(v);
    initializeAudio();
    emit bufferSizeChanged();
}

void MobileSynth::addAudioSource(AudioSource *source)
{
    m_generator->addAudioSource(source);
}

void MobileSynth::removeAudioSource(AudioSource *source)
{
    m_generator->removeAudioSource(source);
}

void MobileSynth::suspendAudio()
{
    m_audioOutput->suspend();
}

void MobileSynth::resumeAudio()
{
    m_audioOutput->resume();
}

//#include "moc_audiooutput.cpp"
