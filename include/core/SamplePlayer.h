/*
 * SamplePlayer.h
 *
 *  Created on: 13 марта 2016 г.
 *      Author: sadko
 */

#ifndef SAMPLEPLAYER_H_
#define SAMPLEPLAYER_H_

#include <core/types.h>

namespace lsp
{
    class Sample
    {
        private:
            float      *vBuffer;
            size_t      nLength;
            size_t      nMaxLength;
            size_t      nChannels;

        public:
            Sample();
            ~Sample();

        public:
            inline bool valid() const { return (vBuffer != NULL) && (nChannels > 0) && (nLength > 0) && (nMaxLength > 0); }
            inline size_t length() const { return nLength; }
            inline float *getBuffer(size_t channel) { return &vBuffer[nMaxLength * channel]; }
            inline float *getBuffer(size_t channel, size_t offset) { return &vBuffer[nMaxLength * channel + offset]; }
            inline size_t channels() const { return nChannels; };

            inline size_t setLength(size_t length)
            {
                if (length > nMaxLength)
                    length = nMaxLength;
                return nLength = length;
            }

            inline void clear()
            {
                nLength     = 0;
            }

            bool init(size_t channels, size_t max_length, size_t length = 0);
            void destroy();
    };
    
    class SamplePlayer
    {
        protected:
            typedef struct playback_t
            {
                Sample     *pSample;    // Pointer to the sample
                size_t      nChannel;   // Channel to play
                ssize_t     nOffset;    // Current offset
                float       nVolume;    // The volume of the sample
                playback_t *pNext;      // Pointer to the next playback in the list
                playback_t *pPrev;      // Pointer to the previous playback in the list
            } playback_t;

            typedef struct list_t
            {
                playback_t *pHead;      // The head of the list
                playback_t *pTail;      // The tail of the list
            } list_t;

        private:
            Sample        **vSamples;
            size_t          nSamples;
            playback_t     *vPlayback;
            size_t          nPlayback;
            list_t          sActive;
            list_t          sInactive;
            float           fGain;

        protected:
            static inline void list_remove(list_t *list, playback_t *pb);
            static inline playback_t *list_remove_first(list_t *list);
            static inline void list_add_first(list_t *list, playback_t *pb);
            static inline void list_insert_from_tail(list_t *list, playback_t *pb);
            void do_process(float *dst, size_t samples);

        public:
            SamplePlayer();
            ~SamplePlayer();

        public:
            /** Set output gain
             *
             * @param gain output gain
             */
            inline void set_gain(float gain) { fGain = gain; }

            /** Initialize player
             *
             * @param max_samples maximum available samples
             * @param max_playbacks maximum number of simultaneous played samples
             * @return true on success
             */
            bool init(size_t max_samples, size_t max_playbacks);

            /** Destroy player
             * @param cascade destroy the bound samples
             */
            void destroy(bool cascade = true);

            /** Bind sample to specified ID
             *
             * @param id id of the sample
             * @param sample pointer to the sample
             * @return true on success, sample contains pointer to the replaced sample
             */
            bool bind(size_t id, Sample **sample);

            /** Bind sample to specified ID
             *
             * @param id id of the sample
             * @param sample pointer to the sample
             * @param destroy auto-destroy sample
             * @return true on success, previous sample will be automatically destroyed
             */
            bool bind(size_t id, Sample *sample, bool destroy = false);

            /** Unbind sample
             *
             * @param id id of the sample
             * @param sample pointer to store the unbound sample
             * @return true on success, pointer to the sample will be stored in sample variable
             */
            bool unbind(size_t id, Sample **sample);

            /** Unbind sample
             *
             * @param id id of the sample
             * @param destroy destroy the sample
             * @return true on success, the bound sample will be automatically destroyed
             */
            bool unbind(size_t id, bool destroy = false);

            /** Process the audio data
             *
             * @param dst destination buffer to store data
             * @param src source buffer to read data
             * @param samples amount of audio samples to process
             */
            void process(float *dst, const float *src, size_t samples);

            /** Process the audio data
             *
             * @param dst destination buffer to store data
             * @param samples amount of audio samples to process
             */
            void process(float *dst, size_t samples);

            /** Trigger the playback of the sample
             *
             * @param id ID of the sample
             * @param channel ID of the sample's channel
             * @param volume the volume of the sample
             * @param delay the delay (in samples) of the sample relatively to the next process() call
             * @return true if parameters are valid
             */
            bool play(size_t id, size_t channel, float volume, ssize_t delay = 0);

            /** Reset the playback state of the player
             *
             */
            void stop();
    };

} /* namespace lsp */

#endif /* SAMPLEPLAYER_H_ */