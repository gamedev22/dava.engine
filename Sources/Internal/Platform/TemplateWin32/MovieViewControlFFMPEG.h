/*==================================================================================
Copyright (c) 2008, binaryzebra
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the binaryzebra nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __DAVAENGINE_MOVIEVIEWCONTROL_H__
#define __DAVAENGINE_MOVIEVIEWCONTROL_H__

#include "UI/IMovieViewControl.h"
#include "UI/UIControl.h"
#include "Render\PixelFormatDescriptor.h"

namespace AV
{
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/rational.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#ifdef __cplusplus
};
#endif
}

#include "Sound/FMODUtils.h"
#include "fmod.h"

#include "FileSystem/DynamicMemoryFile.h"

#include <queue>

namespace DAVA
{
    class MovieViewControl : public IMovieViewControl, public UIControl
    {
    public:
        Mutex pcmMutex;
        DynamicMemoryFile* pcmData = nullptr;
        uint32 writePos = 0;
        uint32 readPos = 0;

        ~MovieViewControl() override;

        // IMovieViewControl Interface implementation

        // Initialize the control.
        void Initialize(const Rect& rect) override;

        // Position/visibility.
        void SetRect(const Rect& rect) override;
        void SetVisible(bool isVisible) override;

        // Open the Movie.
        void OpenMovie(const FilePath& moviePath, const OpenMovieParams& params) override;

        // Start/stop the video playback.
        void Play() override;
        void Stop() override;

        // Pause/resume the playback.
        void Pause() override;
        void Resume() override;

        // Whether the movie is being played?
        bool IsPlaying() const override;


        // UIControl update and draw implementation
        void Update(float32 timeElapsed) override;

    private:
        AV::AVFormatContext* CreateContext(const String& path);

        bool InitVideo();
        void UpdateVideo(AV::AVPacket * packet, float32 timeElapsed);
        bool InitAudio();
        void UpdateAudio(AV::AVPacket* packet, float32 timeElapsed);

        void VideoDecodingThread(BaseObject* caller, void* callerData, void* userData);
        void AudioDecodingThread(BaseObject* caller, void* callerData, void* userData);
        void ReadingThread(BaseObject* caller, void* callerData, void* userData);

        String filepath = "D:/Projects/Win10/wot.blitz/Data/Video/WG_Logo.m4v";

        static bool isFFMGEGInited;
        bool isPlaying = false;
        bool isAudioVideoStreamsInited = false;
        AV::AVFormatContext* avFormatAudioContext = nullptr;
        AV::AVFormatContext* avFormatVideoContext = nullptr;

        Thread* audioDecodingThread = nullptr;
        Thread* videoDecodingThread = nullptr;
        Thread* readingDataThread = nullptr;

        float32 lastUpdateTime = 0.f;

        const uint8 emptyPixelColor = 255;

        Texture * videoTexture = nullptr;
        float32 videoFramerate = 0.f;

        uint32 textureWidth = 0;
        uint32 textureHeight = 0;
        uint32 frameHeight = 0;
        uint32 frameWidth = 0;
        const PixelFormat textureFormat = PixelFormat::FORMAT_RGBA8888;
        uint32 textureBufferSize = 0;
        uint8 * rgbTextureBufferHolder[1];
        uint8 * rgbTextureBuffer = nullptr;

        uint32_t len = 0;
        int32 index = 0;
        int64_t in_channel_layout = -1;
        struct SwrContext* videoConvertContext = nullptr;

        // AV::AVPacket* packet = nullptr;
        unsigned int videoStreamIndex = -1;
        AV::AVCodecContext* videoCodecContext = nullptr;
        AV::AVCodec* videoCodec = nullptr;
        AV::AVFrame * decodedFrame = nullptr;
        AV::AVFrame * yuvDecodedScaledFrame = nullptr;
        uint8 * out_buffer = nullptr;
        AV::SwsContext * img_convert_ctx = nullptr;

        const uint32 maxAudioFrameSize = 192000; // 1 second of 48khz 32bit audio
        unsigned int audioStreamIndex = -1;
        AV::AVCodecContext* audioCodecContext = nullptr;
        AV::AVCodec* audioCodec = nullptr;
        AV::AVFrame* audioFrame = nullptr;
        AV::SwrContext* audioConvertContext = nullptr;
        uint8* outAudioBuffer = nullptr;
        uint32 outAudioBufferSize = 0;

        Deque<AV::AVPacket*> audioPackets;
        Mutex audioPacketsMutex;

        Deque<AV::AVPacket*> videoPackets;
        Mutex videoPacketsMutex;
    };

    inline void MovieViewControl::Stop()
    {
        if (!isAudioVideoStreamsInited)
            return;

        isPlaying = false;
        videoDecodingThread->Cancel();
        audioDecodingThread->Cancel();
        readingDataThread->Cancel();
        SafeRelease(videoDecodingThread);
        SafeRelease(audioDecodingThread);
        SafeRelease(readingDataThread);
    }

    // Pause/resume the playback.
    inline void MovieViewControl::Pause()
    {
        if (!isAudioVideoStreamsInited)
            return;

        isPlaying = false;
    }

    inline void MovieViewControl::Resume()
    {
        if (!isAudioVideoStreamsInited)
            return;

        isPlaying = true;
    }

    // Whether the movie is being played?
    inline bool MovieViewControl::IsPlaying() const
    {
        return isPlaying;
    }
}



#endif