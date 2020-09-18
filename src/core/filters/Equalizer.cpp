/*
 * Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-plugins
 * Created on: 26 июля 2016 г.
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

#include <dsp/dsp.h>
#include <core/windows.h>
#include <core/filters/Equalizer.h>
#include <core/debug.h>

#define BUFFER_SIZE         0x400U

namespace lsp
{
    Equalizer::Equalizer()
    {
        construct();
    }

    Equalizer::~Equalizer()
    {
        destroy();
    }

    void Equalizer::construct()
    {
        sBank.construct();

        vFilters        = NULL;
        nFilters        = 0;
        nSampleRate     = 0;
        nConvSize       = 0;
        nFftRank        = 0;
        nLatency        = 0;
        nBufSize        = 0;
        nMode           = EQM_BYPASS;
        vFftRe          = NULL;
        vFftIm          = NULL;
        vConvRe         = NULL;
        vConvIm         = NULL;
        vBuffer         = NULL;
        vTmp            = NULL;
        pData           = NULL;
        nFlags          = EF_REBUILD | EF_CLEAR;
    }

    bool Equalizer::init(size_t filters, size_t conv_rank)
    {
        destroy();

        // Initialize filter bank
        sBank.init(filters * FILTER_CHAINS_MAX);

        // Initialize filters
        nSampleRate     = 0;
        vFilters        = new Filter[filters];
        if (vFilters == NULL)
        {
            destroy();
            return false;
        }
        nFilters        = filters;

        // Allocate buffers for convolution
        if (conv_rank > 0)
        {
            nConvSize           = 1 << conv_rank;
            nFftRank            = conv_rank;
            size_t conv_size    = nConvSize * 2;
            size_t tmp_size     = lsp_max(conv_size*2, BUFFER_SIZE);
            size_t allocate     = conv_size * 4 + tmp_size;             // fft + conv*2 + buffer + tmp
            pData               = new float[allocate];
            if (pData == NULL)
            {
                destroy();
                return false;
            }

            dsp::fill_zero(pData, allocate);

            // Assign pointers
            float *ptr          = pData;
            vFftRe              = ptr;
            ptr                += conv_size;
            vFftIm              = ptr;
            ptr                += conv_size;
            vConvRe             = ptr;
            ptr                += conv_size;
            vConvIm             = ptr;
            ptr                += conv_size;
            vBuffer             = ptr;
            ptr                += conv_size;
            vTmp                = ptr;
            ptr                += tmp_size;
        }
        else
        {
            pData               = new float[BUFFER_SIZE];
            if (pData == NULL)
            {
                destroy();
                return false;
            }

            dsp::fill_zero(pData, BUFFER_SIZE);

            nConvSize           = 0;
            nFftRank            = 0;
            vFftRe              = NULL;
            vFftIm              = NULL;
            vConvRe             = NULL;
            vConvIm             = NULL;
            vBuffer             = NULL;
            vTmp                = pData;
        }

        // Initialize filters
        for (size_t i=0; i<filters; ++i)
        {
            if (!vFilters[i].init(&sBank))
            {
                destroy();
                return false;
            }
        }

        // Mark equalizer for rebuild
        nFlags              = EF_REBUILD | EF_CLEAR;
        nLatency            = 0;
        nBufSize            = 0;

        return true;
    }

    void Equalizer::destroy()
    {
        if (vFilters != NULL)
        {
            for (size_t i=0; i<nFilters; ++i)
                vFilters[i].destroy();
            delete [] vFilters;
            vFilters        = NULL;
            nFilters        = 0;
        }

        if (pData != NULL)
        {
            delete [] pData;
            vFftRe          = NULL;
            vFftIm          = NULL;
            vConvRe         = NULL;
            vConvIm         = NULL;
            vBuffer         = NULL;
            vTmp            = NULL;
            pData           = NULL;
        }

        sBank.destroy();
    }

    void Equalizer::set_sample_rate(size_t sr)
    {
        if (nSampleRate == sr)
            return;

        filter_params_t fp;
        nSampleRate     = sr;

        for (size_t i=0; i<nFilters; ++i)
        {
            vFilters[i].get_params(&fp);
            vFilters[i].update(nSampleRate, &fp);
        }
    }

    bool Equalizer::set_params(size_t id, const filter_params_t *params)
    {
        if (id >= nFilters)
            return false;

        Filter *f = &vFilters[id];
        f->update(nSampleRate, params);
        nFlags     |= EF_REBUILD;
        return true;
    }

    bool Equalizer::get_params(size_t id, filter_params_t *params)
    {
        if (id >= nFilters)
            return false;
        vFilters[id].get_params(params);
        return true;
    }

    void Equalizer::reconfigure()
    {
        if (nMode == EQM_BYPASS)
            return;

        // Initialize bank
        sBank.begin();
        for (size_t i=0; i<nFilters; ++i)
            vFilters[i].rebuild();
        sBank.end(nFlags & EF_CLEAR);
        nFlags              = 0;

        // Quit if working in IIR mode
        if (nMode == EQM_IIR)
        {
            nLatency        = 0;
            return;
        }

        size_t conv_len     = nConvSize << 1;
        size_t half_size    = nConvSize >> 1;
        float *conv_re      = vConvRe;
        float *conv_im      = vConvIm;

        // Backup input buffer
        dsp::copy(vTmp, vFftRe, conv_len);

        // Init convolution
        dsp::fill_one(conv_re, nConvSize);

        if (nMode == EQM_FIR)
        {
            // Clear buffers
            windows::window(conv_im, nConvSize*2, windows::BLACKMAN_NUTTALL);

            // Get impulse response
            sBank.impulse_response(vFftRe, nConvSize);
            dsp::fill_zero(vFftIm, nConvSize);
            dsp::mul2(vFftRe, &conv_im[nConvSize], nConvSize);  // Apply window function to the impulse response

            // Do the FFT of the impulse response
            dsp::direct_fft(vFftRe, vFftIm, vFftRe, vFftIm, nFftRank);
            dsp::complex_mod(vFftRe, vFftRe, vFftIm, nConvSize);
            dsp::mul2(conv_re, vFftRe, nConvSize);             // Apply the frequency chart relative to the IR
        }
        else if (nMode == EQM_FFT)
        {
            // Initialize frequencies
            ssize_t n_freqs         = nConvSize >> 1;
            float kf                = float(nSampleRate) / nConvSize;
            for (ssize_t i=0; i<=n_freqs; ++i)
                conv_im[i]              = i * kf;

            // Build frequency chart for all filters
            for (size_t i=0; i<nFilters; ++i)
            {
                if (vFilters[i].inactive())
                    continue;

                // Get the frequency chart of the filter
                vFilters[i].freq_chart(vFftRe, vFftIm, conv_im, n_freqs+1);
                dsp::complex_mod(vFftRe, vFftRe, vFftIm, n_freqs+1);
                dsp::mul2(conv_re, vFftRe, n_freqs+1);
            }

            // Finally, build the correct frequency chart for reverse FFT
            float *tail_re          = &conv_re[nConvSize];
            for (ssize_t i=1; i<n_freqs; ++i)
                tail_re[-i]     = conv_re[i];
        }

        // Now we have the frequency chart in conv_re
        dsp::fill_zero(conv_im, nConvSize);
        dsp::reverse_fft(vFftRe, vFftIm, conv_re, conv_im, nFftRank);   // Apply reversive FFT transform to get impulse response

        dsp::copy(vFftIm, &vFftRe[half_size], half_size);               // Make the impulse response symmetric
        dsp::copy(&vFftIm[half_size], vFftRe, half_size);

        windows::window(conv_im, nConvSize, windows::BLACKMAN_NUTTALL);
        dsp::mul3(vFftRe, vFftIm, conv_im, nConvSize);              // Apply window to the impulse response

        // Get the final convolution spectrum
        dsp::fill_zero(&vFftRe[nConvSize], nConvSize);
        dsp::fill_zero(vFftIm, conv_len);
        dsp::direct_fft(conv_re, conv_im, vFftRe, vFftIm, nFftRank+1);

        // Restore input buffer
        dsp::copy(vFftRe, vTmp, conv_len);
        nLatency    = nConvSize;
    }

    void Equalizer::set_mode(equalizer_mode_t mode)
    {
        if (mode == nMode)
            return;
        nMode       = mode;
        nFlags     |= EF_REBUILD | EF_CLEAR;
    }

    bool Equalizer::freq_chart(size_t id, float *re, float *im, const float *f, size_t count)
    {
        if (id >= nFilters)
            return false;
        if (nFlags != 0)
            reconfigure();

        vFilters[id].freq_chart(re, im, f, count);
        return true;
    }

    bool Equalizer::freq_chart(size_t id, float *c, const float *f, size_t count)
    {
        if (id >= nFilters)
            return false;
        if (nFlags != 0)
            reconfigure();

        vFilters[id].freq_chart(c, f, count);
        return true;
    }

    void Equalizer::freq_chart(float *re, float *im, const float *f, size_t count)
    {
        if (nFlags != 0)
            reconfigure();

        float *xre      = vTmp;
        float *xim      = &xre[BUFFER_SIZE/2];

        // Fill initial values
        dsp::fill_one(re, count);
        dsp::fill_zero(im, count);

        while (count > 0)
        {
            // Estimate number of frequencies to process
            size_t to_do    = lsp_min(count, size_t(BUFFER_SIZE/2));

            for (size_t i=0; i<nFilters; ++i)
            {
                Filter *xf      = &vFilters[i];
                if (!xf->active())
                    continue;

                xf->freq_chart(xre, xim, f, to_do);
                dsp::complex_mul2(re, im, xre, xim, to_do);
            }

            // Update pointers
            re             += to_do;
            im             += to_do;
            f              += to_do;
            count          -= to_do;
        }
    }

    void Equalizer::freq_chart(float *c, const float *f, size_t count)
    {
        if (nFlags != 0)
            reconfigure();

        // Fill initial values
        dsp::pcomplex_fill_ri(c, 1.0f, 0.0f, count);

        while (count > 0)
        {
            // Estimate number of frequencies to process
            size_t to_do    = lsp_min(count, size_t(BUFFER_SIZE/2));

            for (size_t i=0; i<nFilters; ++i)
            {
                Filter *xf      = &vFilters[i];
                if (!xf->active())
                    continue;

                xf->freq_chart(vTmp, f, to_do);
                dsp::pcomplex_mul2(c, vTmp, to_do);
            }

            // Update pointers
            c              += to_do * 2;
            f              += to_do;
            count          -= to_do;
        }
    }

    void Equalizer::process(float *out, const float *in, size_t samples)
    {
        if (nFlags != 0)
            reconfigure();

        switch (nMode)
        {
            case EQM_BYPASS:
            {
                dsp::copy(out, in, samples);
                break;
            }

            case EQM_IIR:
            {
                sBank.process(out, in, samples);
                break;
            }

            case EQM_FIR:
            case EQM_FFT:
            default:
            {
                while (samples > 0)
                {
                    if (nBufSize >= nConvSize)
                    {
                        size_t conv_len = nConvSize * 2;

                        dsp::fill_zero(&vFftRe[nConvSize], nConvSize);

                        // Perform the direct FFT of the input signal
                        dsp::fill_zero(vFftIm, conv_len);
                        dsp::direct_fft(vFftRe, vFftIm, vFftRe, vFftIm, nFftRank + 1);

                        // Perform convolution
                        dsp::complex_mul2(vFftRe, vFftIm, vConvRe, vConvIm, conv_len);

                        // Perform the reverse FFT
                        dsp::reverse_fft(vFftRe, vFftIm, vFftRe, vFftIm, nFftRank + 1);

                        // Apply previous convolution tail
                        dsp::add2(vFftRe, &vBuffer[nConvSize], nConvSize);

                        // Update the buffer
                        dsp::copy(vBuffer, vFftRe, conv_len);

                        // Reset the buffer size
                        nBufSize    = 0;
                    }

                    // Determine number of samples to process
                    size_t to_process = nConvSize - nBufSize;
                    if (to_process > samples)
                        to_process      = samples;

                    // Push new data for processing and emit processed data
                    dsp::copy(&vFftRe[nBufSize], in, to_process);
                    dsp::copy(out, &vBuffer[nBufSize], to_process);

                    // Update pointers and counters
                    nBufSize       += to_process;
                    out            += to_process;
                    in             += to_process;
                    samples        -= to_process;
                }

                break;
            }
        }
    }

    void Equalizer::dump(IStateDumper *v) const
    {
        v->write_object("sBank", &sBank);

        v->begin_array("vFilters", vFilters, nFilters);
        for (size_t i=0; i<nFilters; ++i)
            v->write_object(&vFilters[i]);
        v->end_array();

        v->write("nFilters", nFilters);
        v->write("nSampleRate", nSampleRate);
        v->write("nConvSize", nConvSize);
        v->write("nFftRank", nFftRank);
        v->write("nLatency", nLatency);
        v->write("nBufSize", nBufSize);
        v->write("nMode", nMode);
        v->write("vFftRe", vFftRe);
        v->write("vFftIm", vFftIm);
        v->write("vConvRe", vConvRe);
        v->write("vConvIm", vConvIm);
        v->write("vBuffer", vBuffer);
        v->write("vTmp", vTmp);
        v->write("pData", pData);
        v->write("nFlags", nFlags);
    }

} /* namespace lsp */
