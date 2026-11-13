// /^\v/^\v/^\v/^\v/^\v/^\v/^\v/^\v/^\v/^\v/^\v/^\v/^\v/^\v/^\v/^\
// Copyright (C) 2024 c1audio.com / Claudio Zopfi <c1audio@x21.ch>
// SPDX-License-Identifier: GPL-3.0

#include <QtGlobal>
#ifndef Q_OS_IOS

#include "qt68wraper.h"

#include <QAudioDevice>
#include <QMediaDevices>
#include <QAudioSink>
#include <QDebug>
#include <qmath.h>
#include <qendian.h>


Qt68Wraper::Qt68Wraper()
    :   QIODevice()
{
    VoiceCount = 16;

    QAudioDevice info(QMediaDevices::defaultAudioOutput());
    m_format=info.preferredFormat();
    qDebug() << "defaultAudioOutput: " << info.id() << " " << info.description() ;
    qDebug() << "Preferred format is sr " << m_format.sampleRate() << " sn " << m_format.bytesPerSample() << " ch " << m_format.channelCount() << " sample format " << m_format.sampleFormat();

    setFormat(m_format);

    for(int i=0;i<VoiceCount;i++) {
        synth::Controller * sc = new synth::Controller();
        sc->set_sample_rate(m_format.sampleRate());
        sc->set_volume(0.8);
        sc->set_osc1_level(0.8);
        sc->set_arpeggio_enabled(false);
        sc->set_arpeggio_octaves(1);
        sc->set_arpeggio_samples(50000);
        sc->set_arpeggio_step(synth::Arpeggio::UP_DOWN);
        sc->set_filter_cutoff(1000);
        sc->set_filter_resonance(0.1);
        sc->volume_envelope()->set_attack(100);
        sc->volume_envelope()->set_decay(10000);
        sc->volume_envelope()->set_sustain(1.0f);
        sc->volume_envelope()->set_release(80000);
        sc->filter_envelope()->set_attack(80000);
        sc->filter_envelope()->set_decay(80000);
        sc->filter_envelope()->set_sustain(0.4f);
        sc->filter_envelope()->set_release(80000);
        syctl.append( sc );
        VoiceMap.append( 0 );
        VoiceFreqMap.append( 0 );
    }
    arpeggioEnabled = false;

}

Qt68Wraper::~Qt68Wraper()
{
    //m_audioOutput->stop();
    this->stop();
    for(const auto voice:std::as_const(syctl)) {
        delete(voice);
    }
    syctl.clear();
    VoiceMap.clear();
    VoiceFreqMap.clear();
    delete(m_audioOutput);
}

void Qt68Wraper::setFormat(const QAudioFormat& format) {
    DataSampleRateHz  = format.sampleRate();
    channelBytes = format.bytesPerSample();
    channelCount = format.channelCount();
    sampleFormat = format.sampleFormat();
    sampleLittleEndian = QSysInfo::ByteOrder == 1 ? true : false;
    qDebug() << "sampleType " <<  sampleFormat << " channelCount " << channelCount << " channelBytes " << channelBytes << " sampleLittleEndian " << sampleLittleEndian;

    std::lock_guard<std::mutex> lock(audioSourcesMutex);
    for(const auto source:std::as_const(audioSources)) {
        source->setSampleRate(DataSampleRateHz);
    }
}

void Qt68Wraper::addAudioSource(AudioSource *source)
{
    source->setSampleRate(DataSampleRateHz);

    std::lock_guard<std::mutex> lock(audioSourcesMutex);
    if(!audioSources.contains(source)) {
        audioSources.append(source);
    }
}

void Qt68Wraper::removeAudioSource(AudioSource *source)
{
    // once this returns the audio thread does not touch the source anymore
    std::lock_guard<std::mutex> lock(audioSourcesMutex);
    audioSources.removeAll(source);
}

void Qt68Wraper::RenderAudioSources(int frames)
{
    extFrames = 0;

    // never block the audio thread: skip the sources while they are reconfigured
    std::unique_lock<std::mutex> lock(audioSourcesMutex, std::try_to_lock);
    if(!lock.owns_lock() || audioSources.isEmpty() || frames<=0) {
        return;
    }

    if(srcL.size()<frames) {
        srcL.resize(frames);
        srcR.resize(frames);
        extL.resize(frames);
        extR.resize(frames);
    }

    for(const auto source:std::as_const(audioSources)) {
        if(!source->render(srcL.data(), srcR.data(), frames)) {
            continue;
        }
        if(extFrames==0) {
            std::copy_n(srcL.constData(), frames, extL.data());
            std::copy_n(srcR.constData(), frames, extR.data());
            extFrames = frames;
        } else {
            for(int i=0;i<frames;i++) {
                extL[i] += srcL.at(i);
                extR[i] += srcR.at(i);
            }
        }
    }
}

void Qt68Wraper::setBufferSize(int s)
{
    BufferSize=s;
}

void Qt68Wraper::start()
{
    qDebug() << "mobileSynthQT68::start";
    open(QIODevice::ReadOnly);
}

void Qt68Wraper::stop()
{
    qDebug() << "mobileSynthQT68::stop";
    close();
}

void Qt68Wraper::GetSampleSum(int frame, qreal *x) {

    // mix the value of all synth voices (mono)
    qreal v=0;
    for(const auto voice:std::as_const(syctl)) {
        v+=voice->GetSample();
    }

    // add the additional audio sources (stereo)
    qreal l=v;
    qreal r=v;
    if(frame<extFrames) {
        l+=extL.at(frame);
        r+=extR.at(frame);
        if(channelCount<2) {
            l=r=(l+r)/2;
        }
    }

    l = fmax(-1.0, fmin(1.0, l));
    r = fmax(-1.0, fmin(1.0, r));

    // calculate peak and clip
    qreal xAbs = fmax(fabs(l),fabs(r));
    peakValue = fmax(peakValue,xAbs);
    if(xAbs==1) {
        clip=true;
        clipLen++;
        clipLenMax = qMax(clipLen,clipLenMax);
    } else {
        clipLen=0;
    }

    x[0]=l;
    x[1]=r;
}

qint64 Qt68Wraper::readData(char *data, qint64 len)
{
    readDataLen=len;
    peakValue = 0;
    clip = false;
    clipLen = 0;
    clipLenMax = 0;

    if(channelBytes>0) {
        // x holds the left and right value of the current frame
        qreal x[2];
        int frame=0;
        RenderAudioSources(len / (channelBytes*channelCount));

        unsigned char *ptr = reinterpret_cast<unsigned char *>(data);

        if (channelBytes == 1 && sampleFormat == 1 /* QAudioFormat::UInt8 */) {
            while (len>0) {
                GetSampleSum(frame++, x);

                for(int i=0;i<channelCount;i++) {
                    const quint8 value = static_cast<quint8>((1.0 + x[i&1]) / 2 * 255);
                    *reinterpret_cast<quint8*>(ptr) = value;

                    ptr += channelBytes;
                    len -= channelBytes;
                }
            }
        } else if (channelBytes == 2 && sampleFormat == 2 && sampleLittleEndian == true  /* QAudioFormat::Int16 */) {
            while (len>0) {
                GetSampleSum(frame++, x);

                for(int i=0;i<channelCount;i++) {
                    qint16 value = static_cast<qint16>(x[i&1] * 32767);
                    qToLittleEndian<qint16>(value, ptr);

                    ptr += channelBytes;
                    len -= channelBytes;
                }
            }
        } else if (channelBytes == 2 && sampleFormat == 2 && sampleLittleEndian == false  /* QAudioFormat::Int16 */) {
            while (len>0) {
                GetSampleSum(frame++, x);

                for(int i=0;i<channelCount;i++) {
                    qint16 value = static_cast<qint16>(x[i&1] * 32767);
                    qToBigEndian<qint16>(value, ptr);

                    ptr += channelBytes;
                    len -= channelBytes;
                }
            }
        } else if (channelBytes == 4 && sampleFormat == 3 && sampleLittleEndian == true  /* QAudioFormat::Int32 */) {
            while (len>0) {
                GetSampleSum(frame++, x);

                for(int i=0;i<channelCount;i++) {
                    qint32 value = static_cast<qint32>(x[i&1] * 2147483647);
                    qToLittleEndian<qint32>(value, ptr);

                    ptr += channelBytes;
                    len -= channelBytes;
                }
            }
        } else if (channelBytes == 4 && sampleFormat == 3 && sampleLittleEndian == false  /* QAudioFormat::Int32 */) {
            while (len>0) {
                GetSampleSum(frame++, x);

                for(int i=0;i<channelCount;i++) {
                    qint32 value = static_cast<qint32>(x[i&1] * 2147483647);
                    qToBigEndian<qint32>(value, ptr);

                    ptr += channelBytes;
                    len -= channelBytes;
                }
            }
        } else if (channelBytes == 4 && sampleFormat == 4 /* QAudioFormat::Float */) {
            while (len>0) {
                GetSampleSum(frame++, x);

                for(int i=0;i<channelCount;i++) {
                    *reinterpret_cast<float*>(ptr) = x[i&1];

                    ptr += channelBytes;
                    len -= channelBytes;
                }
            }
        } else {
            while (len>0) {
                *ptr = 0;
                ptr++;
                len--;
            }
        }
    }

    //qDebug() << "mobileSynthQT68::readData done " << len ;

    emit sampleUpdated();
    return readDataLen;
}

qint64 Qt68Wraper::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

qint64 Qt68Wraper::bytesAvailable() const
{
    return BufferSize;
}

qint64 Qt68Wraper::size() const
{
    return BufferSize;
}

qint64 Qt68Wraper::get_read_data_len()
{
    return readDataLen;
}

bool Qt68Wraper::get_clip()
{
    return clip;
}

qint64 Qt68Wraper::get_clip_len()
{
    return clipLenMax;
}

qreal Qt68Wraper::get_peak()
{
    return peakValue;
}

int Qt68Wraper::get_sample_rate()
{
    return DataSampleRateHz;
}

int Qt68Wraper::get_channel_bytes()
{
    return channelBytes;
}

int Qt68Wraper::get_channel_count()
{
    return channelCount;
}

int Qt68Wraper::get_sample_format()
{
    return sampleFormat;
}

bool Qt68Wraper::get_sample_little_endian()
{
    return sampleLittleEndian;
}

void Qt68Wraper::set_arpeggio_enabled(bool v)
{
    arpeggioEnabled = v;

    syctl.at(0)->set_arpeggio_enabled(v);

    if(arpeggioEnabled) {
        // transfer all active voices to voice instance 0
        for(int i=1;i<VoiceMap.count();i++) {
            if(VoiceMap.at(i)>0) {
                syctl.at(i)->NoteOff(VoiceMap.at(i));
                syctl.at(0)->NoteOn(VoiceMap.at(i),VoiceFreqMap.at(i));
            }
        }
    } else {
        // transfer all active voices to separate voice instances
        for(int i=1;i<VoiceMap.count();i++) {
            if(VoiceMap.at(i)>0) {
                syctl.at(0)->NoteOff(VoiceMap.at(i));
                syctl.at(i)->NoteOn(VoiceMap.at(i),VoiceFreqMap.at(i));
            }
        }
    }
}

void Qt68Wraper::set_arpeggio_samples(int v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->set_arpeggio_samples(v);
    }
}

void Qt68Wraper::set_arpeggio_octaves(int v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->set_arpeggio_octaves(v);
    }
}

void Qt68Wraper::set_arpeggio_step(int v)
{
    synth::Arpeggio::Step step;

    switch(v) {
    case 0:
        step=synth::Arpeggio::Step::DOWN;
        break;
    case 1:
        step=synth::Arpeggio::Step::UP;
        break;
    case 2:
        step=synth::Arpeggio::Step::RANDOM;
        break;
    default:
        step=synth::Arpeggio::Step::UP_DOWN;
        break;
    }

    for(const auto voice:std::as_const(syctl)) {
        voice->set_arpeggio_step(step);
    }
}

void Qt68Wraper::set_filter_cutoff(int v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->set_filter_cutoff(v);
    }
}

void Qt68Wraper::set_filter_resonance(float v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->set_filter_resonance(v);
    }
}

void Qt68Wraper::set_osc1_level(float v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->set_osc1_level(v);
    }
}

void Qt68Wraper::set_osc1_wave_type(synth::Oscillator::WaveType v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->set_osc1_wave_type(v);
    }
}

void Qt68Wraper::set_osc1_octave(synth::Controller::OctaveShift v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->set_osc1_octave(v);
    }
}

void Qt68Wraper::set_osc2_level(float v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->set_osc2_level(v);
    }
}

void Qt68Wraper::set_osc2_wave_type(synth::Oscillator::WaveType v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->set_osc2_wave_type(v);
    }
}

void Qt68Wraper::set_osc2_octave(synth::Controller::OctaveShift v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->set_osc2_octave(v);
    }
}

void Qt68Wraper::set_osc2_shift(int v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->set_osc2_shift(v);
    }
}

void Qt68Wraper::set_osc_sync(bool v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->set_osc_sync(v);
    }
}

void Qt68Wraper::set_glide_samples(long v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->set_glide_samples(v);
    }
}

void Qt68Wraper::set_modulation_source(synth::Controller::ModulationSource v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->set_modulation_source(v);
    }
}

void Qt68Wraper::set_modulation_destination(synth::Controller::ModulationDestination v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->set_modulation_destination(v);
    }
}

void Qt68Wraper::set_modulation_amount(float value)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->set_modulation_amount(value);
    }
}

void Qt68Wraper::set_modulation_amount(int voiceId, float value)
{
    for(int i=0;i<VoiceMap.count();i++) {
        if(VoiceMap.at(i)==voiceId) {
            syctl.at(i)->set_modulation_amount(value);
            break;
        }
    }
}

void Qt68Wraper::set_modulation_frequency(float v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->set_modulation_frequency(v);
    }
}

void Qt68Wraper::set_volume_attack(long v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->volume_envelope()->set_attack(v);
    }
}

void Qt68Wraper::set_volume_decay(long v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->volume_envelope()->set_decay(v);
    }
}

void Qt68Wraper::set_volume_sustain(float v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->volume_envelope()->set_sustain(v);
    }
}

void Qt68Wraper::set_volume_release(long v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->volume_envelope()->set_release(v);
    }
}

void Qt68Wraper::set_filter_attack(long v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->filter_envelope()->set_attack(v);
    }
}

void Qt68Wraper::set_filter_decay(long v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->filter_envelope()->set_decay(v);
    }
}

void Qt68Wraper::set_filter_sustain(float v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->filter_envelope()->set_sustain(v);
    }
}

void Qt68Wraper::set_filter_release(long v)
{
    for(const auto voice:std::as_const(syctl)) {
        voice->filter_envelope()->set_release(v);
    }
}

void Qt68Wraper::pitch(int vid, float f)
{
    for(int i=0;i<VoiceMap.count();i++) {
        if(VoiceMap[i]==vid) {
            syctl.at(i)->Pitch(f);
            qDebug() << "mobileSynthQT68::pitch vid: " << vid << "mapid: " << i << " f: " << f;
            return;
        }
    }
    qDebug() << "mobileSynthQT68::pitch ignored: vid: " << vid << " f: " << f;
}

void Qt68Wraper::noteOn(int vid, float f)
{
    qDebug() << "mobileSynthQT68::noteOn vid:" << vid << " f: " << f;
    if(arpeggioEnabled) {
        for(int i=0;i<VoiceMap.count();i++) {
            if(VoiceMap.at(i)==0) {
                syctl.at(0)->NoteOn(vid,f);
                VoiceMap[i]=vid;
                VoiceFreqMap[i]=f;
                qDebug() << "mobileSynthQT68::noteOn mapid:" << i;
                break;
            }
        }
    } else {
        for(int i=0;i<VoiceMap.count();i++) {
            if(syctl.at(i)->released()) {
                syctl.at(i)->NoteOn(vid,f);
                VoiceMap[i]=vid;
                VoiceFreqMap[i]=f;
                qDebug() << "mobileSynthQT68::noteOn mapid:" << i;
                break;
            }
        }
    }
}

void Qt68Wraper::noteOff(int vid)
{
    qDebug() << "mobileSynthQT68::noteOff vid:" << vid;
    if(arpeggioEnabled) {
        for(int i=0;i<VoiceMap.count();i++) {
            if(VoiceMap.at(i)==vid) {
                syctl.at(0)->NoteOff(vid);
                VoiceMap[i]=0;
                qDebug() << "mobileSynthQT68::noteOff mapid:" << i;
                break;
            }
        }
    } else {
        for(int i=0;i<VoiceMap.count();i++) {
            if(VoiceMap.at(i)==vid) {
                syctl.at(i)->NoteOff(vid);
                VoiceMap[i]=0;
                qDebug() << "mobileSynthQT68::noteOff mapid:" << i;
                break;
            }
        }
    }
}

#endif
