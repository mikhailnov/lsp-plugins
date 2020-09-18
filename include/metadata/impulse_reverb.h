/*
 * Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-plugins
 * Created on: 13 фев. 2017 г.
 *
 * lsp-plugins is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * lsp-plugins is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with lsp-plugins. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef METADATA_IMPULSE_REVERB_H_
#define METADATA_IMPULSE_REVERB_H_

namespace lsp
{
    //-------------------------------------------------------------------------
    // Impulse reverb metadata
    struct impulse_reverb_base_metadata
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

        static const size_t FILES                   = 4;        // Number of IR files
        static const size_t CONVOLVERS              = 4;        // Number of IR convolvers

        static const size_t FFT_RANK_MIN            = 9;        // Minimum FFT rank

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

    struct impulse_reverb_mono_metadata: public impulse_reverb_base_metadata
    {
        static const plugin_metadata_t metadata;
    };

    struct impulse_reverb_stereo_metadata: public impulse_reverb_base_metadata
    {
        static const plugin_metadata_t metadata;
    };

}

#endif /* METADATA_IMPULSE_REVERB_H_ */
