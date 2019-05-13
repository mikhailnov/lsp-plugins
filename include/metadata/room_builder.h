/*
 * room_builder.h
 *
 *  Created on: 25 апр. 2019 г.
 *      Author: sadko
 */

#ifndef METADATA_ROOM_BUILDER_H_
#define METADATA_ROOM_BUILDER_H_

namespace lsp
{
    //-------------------------------------------------------------------------
    // Room builder metadata
    struct room_builder_base_metadata
    {
        static const float CONV_LENGTH_MIN          = 0.0f;     // Minimum convolution length (ms)
        static const float CONV_LENGTH_MAX          = 10000.0f; // Maximum convoluition length (ms)
        static const float CONV_LENGTH_DFL          = 0.0f;     // Convolution length (ms)
        static const float CONV_LENGTH_STEP         = 0.1f;     // Convolution step (ms)

        static const float PREDELAY_MIN             = 0.0f;     // Minimum pre-delay length (ms)
        static const float PREDELAY_MAX             = 100.0f;   // Maximum pre-delay length (ms)
        static const float PREDELAY_DFL             = 0.0f;     // Pre-delay length (ms)
        static const float PREDELAY_STEP            = 0.01f;    // Pre-delay step (ms)

        static const size_t MESH_SIZE               = 600;      // Maximum mesh size
        static const size_t TRACKS_MAX              = 8;        // Maximum tracks per mesh/sample

        static const size_t CONVOLVERS              = 4;        // Number of IR convolvers

        static const size_t CAPTURES                = 8;        // Number of captures

        static const size_t FFT_RANK_MIN            = 9;        // Minimum FFT rank

        static const float POSITION_MIN             = -100.0f;
        static const float POSITION_MAX             = +100.0f;
        static const float POSITION_DFL             = 0.0f;
        static const float POSITION_STEP            = 0.01f;

        static const float CAPSULE_MIN              = 10.0f;
        static const float CAPSULE_MAX              = 50.0f;
        static const float CAPSULE_DFL              = 22.0f;
        static const float CAPSULE_STEP             = 0.1f;

        static const float ANGLE_MIN                = 0.0f;
        static const float ANGLE_MAX                = 180.0f;
        static const float ANGLE_DFL                = 90.0f;
        static const float ANGLE_STEP               = 0.1f;

        static const float DISTANCE_MIN             = 0.0f;
        static const float DISTANCE_MAX             = +10.0f;
        static const float DISTANCE_DFL             = 2.0f;
        static const float DISTANCE_STEP            = 0.01f;

        static const float LCF_MIN                  = 10.0f;
        static const float LCF_MAX                  = 1000.0f;
        static const float LCF_DFL                  = 50.0f;
        static const float LCF_STEP                 = 0.001f;

        static const float HCF_MIN                  = 2000.0f;
        static const float HCF_MAX                  = 22000.0f;
        static const float HCF_DFL                  = 10000.0f;
        static const float HCF_STEP                 = 0.001f;

        static const float BA_MIN                   = GAIN_AMP_M_12_DB;
        static const float BA_MAX                   = GAIN_AMP_P_12_DB;
        static const float BA_DFL                   = GAIN_AMP_0_DB;
        static const float BA_STEP                  = 0.0025f;

        static const size_t EQ_BANDS                = 8;        // 8 bands for equalization

        enum fft_rank_t
        {
            FFT_RANK_512,
            FFT_RANK_1024,
            FFT_RANK_2048,
            FFT_RANK_4096,
            FFT_RANK_8192,
            FFT_RANK_16384,
            FFT_RANK_32767,
            FFT_RANK_65536,

            FFT_RANK_DEFAULT = FFT_RANK_32767
        };
    };

    struct room_builder_mono_metadata: public room_builder_base_metadata
    {
        static const plugin_metadata_t metadata;
    };

    struct room_builder_stereo_metadata: public room_builder_base_metadata
    {
        static const plugin_metadata_t metadata;
    };

}

#endif /* METADATA_ROOM_BUILDER_H_ */
