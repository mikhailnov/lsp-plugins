/*
 * Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-plugins
 * Created on: 29 июн. 2020 г.
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

#include <core/types.h>
#include <core/debug.h>
#include <plugins/loud_comp.h>
#include <core/colors.h>
#include <core/util/Color.h>

#include <generated/iso226/fletcher_munson.h>
#include <generated/iso226/robinson_dadson.h>
#include <generated/iso226/iso226-2003.h>

#define BUF_SIZE            0x1000
#define NUM_CURVES          (sizeof(freq_curves)/sizeof(freq_curve_t *))

#define TRACE_PORT(p) lsp_trace("  port id=%s", (p)->metadata()->id);

namespace lsp
{
    static const freq_curve_t *freq_curves[]=
    {
        &iso226_2003_curve,
        &fletcher_munson_curve,
        &robinson_dadson_curve
    };

    //-------------------------------------------------------------------------
    // Loudness compensator

    loud_comp_base::loud_comp_base(const plugin_metadata_t &mdata, size_t channels): plugin_t(mdata)
    {
        nChannels       = channels;
        nMode           = 0;
        nRank           = FFT_RANK_MIN;
        fGain           = 0.0f;
        fVolume         = -1.0f;
        bBypass         = false;
        bRelative       = false;
        bReference      = false;
        bHClipOn        = false;
        fHClipLvl       = 1.0f;
        vChannels[0]    = NULL;
        vChannels[1]    = NULL;
        vTmpBuf         = NULL;
        vFreqApply      = NULL;
        vFreqMesh       = NULL;
        vAmpMesh        = NULL;
        bSyncMesh       = false;
        pData           = NULL;
        pIDisplay       = NULL;

        pBypass         = NULL;
        pGain           = NULL;
        pMode           = NULL;
        pRank           = NULL;
        pVolume         = NULL;
        pMesh           = NULL;
        pRelative       = NULL;
        pReference      = NULL;
        pHClipOn        = NULL;
        pHClipRange     = NULL;
        pHClipReset     = NULL;
    }

    loud_comp_base::~loud_comp_base()
    {
        destroy();
    }

    void loud_comp_base::init(IWrapper *wrapper)
    {
        // Pass wrapper
        plugin_t::init(wrapper);

        // Initialize oscillator
        if (!sOsc.init())
            return;
        sOsc.set_amplitude(1.0f);
        sOsc.set_dc_offset(0.0f);
        sOsc.set_dc_reference(DC_ZERO);
        sOsc.set_duty_ratio(0.5f);
        sOsc.set_frequency(1000.0f);
        sOsc.set_oversampler_mode(OM_NONE);
        sOsc.set_phase(0.0f);
        sOsc.set_function(FG_SINE);

        // Estimate temporary buffer size
        size_t sz_tmpbuf    = 0;
        for (size_t i=0; i<NUM_CURVES; ++i)
        {
            const freq_curve_t *c = freq_curves[i];
            if (sz_tmpbuf < c->hdots)
                sz_tmpbuf       = c->hdots;
        }
        sz_tmpbuf           = ALIGN_SIZE(sz_tmpbuf*sizeof(float), 0x100);

        // Compute size of data to allocate
        size_t sz_channel   = ALIGN_SIZE(sizeof(channel_t), DEFAULT_ALIGN);
        size_t sz_fft       = (2 << FFT_RANK_MAX) * sizeof(float);
        size_t sz_sync      = ALIGN_SIZE(sizeof(float) * CURVE_MESH_SIZE, DEFAULT_ALIGN);
        size_t sz_buf       = BUF_SIZE * sizeof(float);

        // Total amount of data to allocate
        size_t sz_alloc     = (sz_channel + sz_buf*2) * nChannels + sz_fft + sz_sync*2 + sz_tmpbuf;
        uint8_t *ptr        = alloc_aligned<uint8_t>(pData, sz_alloc);
        if (ptr == NULL)
            return;

        // Allocate channels
        for (size_t i=0; i<nChannels; ++i)
        {
            channel_t *c        = reinterpret_cast<channel_t *>(ptr);
            ptr                += sz_channel;

            c->sDelay.construct();
            c->sBypass.construct();
            c->sProc.construct();
            c->sClipInd.construct();

            c->sDelay.init(1 << (FFT_RANK_MAX - 1));
            c->sProc.init(FFT_RANK_MAX);
            c->sProc.bind(process_callback, this, c);
            c->sProc.set_phase(0.5f * i);

            c->vIn              = NULL;
            c->vOut             = NULL;
            c->vDry             = NULL;
            c->vBuffer          = NULL;
            c->fInLevel         = 0.0f;
            c->fOutLevel        = 0.0f;
            c->bHClip           = false;

            c->pIn              = NULL;
            c->pOut             = NULL;
            c->pMeterIn         = NULL;
            c->pMeterOut        = NULL;
            c->pHClipInd        = NULL;

            vChannels[i]        = c;
        }

        // Allocate buffers
        for (size_t i=0; i<nChannels; ++i)
        {
            channel_t *c        = vChannels[i];
            c->vDry             = reinterpret_cast<float *>(ptr);
            ptr                += sz_buf;
            c->vBuffer          = reinterpret_cast<float *>(ptr);
            ptr                += sz_buf;

            dsp::fill_zero(c->vBuffer, BUF_SIZE);
        }

        // Initialize buffers
        vFreqApply          = reinterpret_cast<float *>(ptr);
        ptr                += sz_fft;
        vFreqMesh           = reinterpret_cast<float *>(ptr);
        ptr                += sz_sync;
        vAmpMesh            = reinterpret_cast<float *>(ptr);
        ptr                += sz_sync;
        vTmpBuf             = reinterpret_cast<float *>(ptr);
        ptr                += sz_tmpbuf;

        lsp_trace("Binding ports...");
        size_t port_id      = 0;

        // Bind input audio ports
        for (size_t i=0; i<nChannels; ++i)
        {
            TRACE_PORT(vPorts[port_id]);
            vChannels[i]->pIn   = vPorts[port_id++];
        }

        // Bind output audio ports
        for (size_t i=0; i<nChannels; ++i)
        {
            TRACE_PORT(vPorts[port_id]);
            vChannels[i]->pOut  = vPorts[port_id++];
        }

        // Bind common ports
        TRACE_PORT(vPorts[port_id]);
        pBypass             = vPorts[port_id++];
        TRACE_PORT(vPorts[port_id]);
        pGain               = vPorts[port_id++];
        TRACE_PORT(vPorts[port_id]);
        pMode               = vPorts[port_id++];
        TRACE_PORT(vPorts[port_id]);
        pRank               = vPorts[port_id++];
        TRACE_PORT(vPorts[port_id]);
        pVolume             = vPorts[port_id++];
        TRACE_PORT(vPorts[port_id]);
        pReference          = vPorts[port_id++];
        TRACE_PORT(vPorts[port_id]);
        pHClipOn            = vPorts[port_id++];
        TRACE_PORT(vPorts[port_id]);
        pHClipRange         = vPorts[port_id++];
        TRACE_PORT(vPorts[port_id]);
        pHClipReset         = vPorts[port_id++];
        TRACE_PORT(vPorts[port_id]);
        pMesh               = vPorts[port_id++];
        TRACE_PORT(vPorts[port_id]);
        pRelative           = vPorts[port_id++];

        // Bind input level meters
        for (size_t i=0; i<nChannels; ++i)
        {
            TRACE_PORT(vPorts[port_id]);
            vChannels[i]->pMeterIn  = vPorts[port_id++];
        }

        // Bind hard clip indicators
        for (size_t i=0; i<nChannels; ++i)
        {
            TRACE_PORT(vPorts[port_id]);
            vChannels[i]->pHClipInd = vPorts[port_id++];
        }

        // Bind output level meters
        for (size_t i=0; i<nChannels; ++i)
        {
            TRACE_PORT(vPorts[port_id]);
            vChannels[i]->pMeterOut = vPorts[port_id++];
        }
    }

    void loud_comp_base::destroy()
    {
        sOsc.destroy();

        if (pIDisplay != NULL)
        {
            pIDisplay->detroy();
            pIDisplay   = NULL;
        }

        for (size_t i=0; i<nChannels; ++i)
        {
            channel_t *c    = vChannels[i];
            if (c == NULL)
                continue;

            c->sDelay.destroy();
            c->sProc.destroy();
            vChannels[i]    = NULL;
        }

        vTmpBuf         = NULL;
        vFreqApply      = NULL;
        vFreqMesh       = NULL;

        if (pData != NULL)
        {
            free_aligned(pData);
            pData   = NULL;
        }
    }

    void loud_comp_base::ui_activated()
    {
        bSyncMesh           = true;
    }

    void loud_comp_base::update_sample_rate(long sr)
    {
        sOsc.set_sample_rate(sr);

        for (size_t i=0; i<nChannels; ++i)
        {
            channel_t *c        = vChannels[i];

            // Update processor settings
            c->sBypass.init(sr);
            c->sClipInd.init(sr, 0.2f);
        }
    }

    void loud_comp_base::update_settings()
    {
        bool rst_clip       = pHClipReset->getValue() >= 0.5f;
        bool bypass         = pBypass->getValue() >= 0.5f;
        size_t mode         = pMode->getValue();
        size_t rank         = FFT_RANK_MIN + ssize_t(pRank->getValue());
        if (rank < FFT_RANK_MIN)
            rank                = FFT_RANK_MIN;
        else if (rank > FFT_RANK_MAX)
            rank                = FFT_RANK_MAX;
        float volume        = pVolume->getValue();
        bool relative       = pRelative->getValue() >= 0.5f;
        bool osc            = pReference->getValue() >= 0.5f;

        // Need to update curve?
        if ((mode != nMode) || (rank != nRank) || (volume != fVolume))
        {
            nMode               = mode;
            nRank               = rank;
            fVolume             = volume;
            bSyncMesh           = true;

            update_response_curve();
        }

        if (osc != bReference)
            sOsc.reset_phase_accumulator();
        if (bRelative != relative)
            bSyncMesh           = true;
        if ((bypass != bBypass) || (bSyncMesh))
            pWrapper->query_display_draw();

        fGain               = pGain->getValue();
        bHClipOn            = pHClipOn->getValue() >= 0.5f;
        bBypass             = bypass;
        bRelative           = relative;
        bReference          = osc;

        if (bHClipOn)
        {
            float min, max;
            dsp::abs_minmax(vFreqApply, 2 << nRank, &min, &max);
            fHClipLvl           = db_to_gain(pHClipRange->getValue()) * sqrtf(min * max);
        }
        else
            fHClipLvl           = 1.0f;

        for (size_t i=0; i<nChannels; ++i)
        {
            channel_t *c        = vChannels[i];
            c->sBypass.set_bypass(bypass);
            c->sProc.set_rank(rank);
            c->sDelay.set_delay(c->sProc.latency());
            if (rst_clip)
                c->bHClip       = false;
        }
    }

    void loud_comp_base::update_response_curve()
    {
        const freq_curve_t *c   = ((nMode > 0) && (nMode <= NUM_CURVES)) ? freq_curves[nMode-1] : NULL;
        size_t fft_size         = 1 << nRank;
        size_t fft_csize        = (fft_size >> 1) + 1;

        if (c != NULL)
        {
            // Get the volume
            float vol   = fVolume - PHONS_MIN;
            if (vol > c->amax)
                vol = c->amax;
            else if (vol < c->amin)
                vol = c->amin;
            vol        -= c->amin;

            // Compute interpolatoin coefficients
            float range = c->amax - c->amin;
            float step  = range / (c->curves-1);
            ssize_t nc  = vol / step;
            if (nc >= ssize_t(c->curves-1))
                --nc;
            float k2    = 0.05f * M_LN10 * (vol/step - nc);
            float k1    = 0.05f * M_LN10 - k2;

            // Interpolate curves to the temporary buffer, translate decibels to gain
            dsp::mix_copy2(vTmpBuf, c->data[nc], c->data[nc+1], k1, k2, c->hdots);
            dsp::exp1(vTmpBuf, c->hdots);

            // Compute frequency response
            ssize_t idx;
            float *v    = vFreqApply;
            range       = 1.0f / logf(c->fmax / c->fmin);
            float kf    = float(fSampleRate) / float(fft_size);
            for (size_t i=0; i < fft_csize; ++i, v += 2)
            {
                float f     = kf * i; // Target frequency
                if (f <= c->fmin)
                    idx         = 0;
                else if (f >= c->fmax)
                    idx         = c->hdots - 1;
                else
                {
                    f               = logf(f / c->fmin);
                    idx             = (f * c->hdots) * range;
                }

                f           = vTmpBuf[idx];
                v[0]        = f;
                v[1]        = f;
            }

            // Create reverse copy to complete the FFT response
            dsp::reverse2(&vFreqApply[fft_size+2], &vFreqApply[2], fft_size-2);

            if (fft_size == 256)
            {
                for (size_t i=0; i<fft_size; ++i)
                    lsp_trace("i=%d; vfa={%.2f, %.2f}", int(i), vFreqApply[i*2], vFreqApply[i*2+1]);
            }
        }
        else
        {
            float vol   = db_to_gain(fVolume);
            dsp::fill(vFreqApply, vol, fft_size * 2);
        }

        // Initialize list of frequencies
        float norm          = logf(FREQ_MAX/FREQ_MIN) / (CURVE_MESH_SIZE - 1);
        for (size_t i=0; i<CURVE_MESH_SIZE; ++i)
            vFreqMesh[i]    = i * norm;
        dsp::exp1(vFreqMesh, CURVE_MESH_SIZE);
        dsp::mul_k2(vFreqMesh, FREQ_MIN, CURVE_MESH_SIZE);

        // Build amp mesh
        float xf                = float(fft_size) / float(fSampleRate);
        for (size_t i=0; i<CURVE_MESH_SIZE; ++i)
        {
            size_t ix       = xf * vFreqMesh[i];
            if (ix > fft_csize)
                ix                  = fft_csize;
            vAmpMesh[i]     = vFreqApply[ix << 1];
        }
    }

    void loud_comp_base::process_callback(void *object, void *subject, float *buf, size_t rank)
    {
        loud_comp_base *_this   = static_cast<loud_comp_base *>(object);
        channel_t *c            = static_cast<channel_t *>(subject);
        _this->process_spectrum(c, buf);
    }

    void loud_comp_base::process_spectrum(channel_t *c, float *buf)
    {
        // Apply spectrum changes to the FFT image
        size_t count = 2 << nRank;
        dsp::mul2(buf, vFreqApply, count);
    }

    void loud_comp_base::process(size_t samples)
    {
        //---------------------------------------------------------------------
        // Bind ports
        for (size_t i=0; i<nChannels; ++i)
        {
            channel_t *c    = vChannels[i];
            c->vIn          = c->pIn->getBuffer<float>();
            c->vOut         = c->pOut->getBuffer<float>();
            c->fInLevel     = 0.0f;
            c->fOutLevel    = 0.0f;
        }

        //---------------------------------------------------------------------
        // Perform main processing
        if (bReference) // Reference signal generation
        {
            sOsc.process_overwrite(vChannels[0]->vOut, samples);
            vChannels[0]->fInLevel  = dsp::abs_max(vChannels[0]->vIn, samples) * fGain;
            vChannels[0]->fOutLevel = dsp::abs_max(vChannels[0]->vOut, samples);

            for (size_t i=1; i<nChannels; ++i)
            {
                dsp::copy(vChannels[i]->vOut, vChannels[0]->vOut, samples);
                vChannels[i]->fInLevel  = dsp::abs_max(vChannels[i]->vIn, samples) * fGain;
                vChannels[i]->fOutLevel = vChannels[0]->fOutLevel;
            }

            // Perform clipping
            for (size_t i=0; i<nChannels; ++i)
            {
                channel_t *c    = vChannels[i];
                c->sClipInd.process(samples);
                if (bHClipOn)
                    c->pHClipInd->setValue((c->bHClip) ? 1.0f : 0.0f);
                else
                    c->pHClipInd->setValue((c->sClipInd.value()) ? 1.0f : 0.0f);
            }
        }
        else // Audio processing
        {
            float lvl;

            for (size_t nleft=samples; nleft > 0; )
            {
                size_t to_process   = (nleft > BUF_SIZE) ? BUF_SIZE : nleft;

                for (size_t i=0; i<nChannels; ++i)
                {
                    channel_t *c    = vChannels[i];

                    // Process the signal
                    c->sDelay.process(c->vDry, c->vIn, to_process);

                    // Apply input gain
                    dsp::mul_k3(c->vBuffer, c->vIn, fGain, to_process);
                    lvl             = dsp::abs_max(c->vBuffer, samples);
                    c->fInLevel     = (c->fInLevel < lvl) ? lvl : c->fInLevel;

                    // Apply volume attenuation
                    c->sProc.process(c->vBuffer, c->vBuffer, to_process);

                    // Perform clipping
                    lvl             = dsp::abs_max(c->vBuffer, to_process);
                    c->sClipInd.process(to_process);
                    bool clip       = lvl > fHClipLvl;
                    if (bHClipOn)
                    {
                        if (clip)
                        {
                            lvl             = fHClipLvl;
                            c->bHClip       = true;
                        }

                        dsp::limit1(c->vBuffer, -fHClipLvl, +fHClipLvl, to_process);
                        c->pHClipInd->setValue((c->bHClip) ? 1.0f : 0.0f);
                    }
                    else
                    {
                        if (clip)
                            c->sClipInd.blink();
                        c->pHClipInd->setValue((c->sClipInd.value()) ? 1.0f : 0.0f);
                    }
                    c->fOutLevel    = (c->fOutLevel < lvl) ? lvl : c->fOutLevel;

                    // Apply bypass
                    c->sBypass.process(c->vOut, c->vDry, c->vBuffer, to_process);

                    // Update pointers
                    c->vIn         += to_process;
                    c->vOut        += to_process;
                }

                nleft          -= to_process;
            }
        }

        //---------------------------------------------------------------------
        // Update meters
        for (size_t i=0; i<nChannels; ++i)
        {
            channel_t *c    = vChannels[i];
            c->pMeterIn->setValue(c->fInLevel);
            c->pMeterOut->setValue(c->fOutLevel);
        }

        // Report latency
        set_latency(vChannels[0]->sDelay.get_delay());

        // Sync output mesh
        mesh_t *mesh = pMesh->getBuffer<mesh_t>();
        if ((bSyncMesh) && (mesh != NULL) && (mesh->isEmpty()))
        {
            // Output mesh data
            dsp::copy(mesh->pvData[0], vFreqMesh, CURVE_MESH_SIZE);

            if (bRelative)
            {
                float kf        = expf(-0.05f * M_LN10 * fVolume);
                dsp::mul_k3(mesh->pvData[1], vAmpMesh, kf, CURVE_MESH_SIZE);
            }
            else
                dsp::copy(mesh->pvData[1], vAmpMesh, CURVE_MESH_SIZE);
            mesh->data(2, CURVE_MESH_SIZE);
            bSyncMesh   = NULL;
        }
    }

    bool loud_comp_base::inline_display(ICanvas *cv, size_t width, size_t height)
    {
        // Check proportions
        if (height > (R_GOLDEN_RATIO * width))
            height  = R_GOLDEN_RATIO * width;

        // Init canvas
        if (!cv->init(width, height))
            return false;
        width   = cv->width();
        height  = cv->height();

        // Clear background
        bool bypass     = bBypass;
        bool relative   = bRelative;
        float volume    = fVolume;
        cv->set_color_rgb((bypass) ? CV_DISABLED : CV_BACKGROUND);
        cv->paint();

        // Draw axis
        if (relative)
        {
            // Draw axis
            cv->set_line_width(1.0);

            float zx    = 1.0f/SPEC_FREQ_MIN;
            float zy    = 1.0f/GAIN_AMP_M_12_DB;
            float dx    = width/(logf(SPEC_FREQ_MAX/SPEC_FREQ_MIN));
            float dy    = height/(logf(GAIN_AMP_M_12_DB/GAIN_AMP_P_72_DB));

            // Draw vertical lines
            cv->set_color_rgb(CV_YELLOW, 0.5f);
            for (float i=100.0f; i<SPEC_FREQ_MAX; i *= 10.0f)
            {
                float ax = dx*(logf(i*zx));
                cv->line(ax, 0, ax, height);
            }

            // Draw horizontal lines
            for (float i=GAIN_AMP_M_12_DB; i<GAIN_AMP_P_72_DB; i *= GAIN_AMP_P_12_DB)
            {
                float ay = height + dy*(logf(i*zy));
                if ((i >= (GAIN_AMP_0_DB - 1e-4)) && ((i <= (GAIN_AMP_0_DB + 1e-4))))
                    cv->set_color_rgb(CV_WHITE, 0.5f);
                else
                    cv->set_color_rgb(CV_YELLOW, 0.5f);
                cv->line(0, ay, width, ay);
            }

            // Allocate buffer: f, a(f), x, y
            pIDisplay           = float_buffer_t::reuse(pIDisplay, 4, width);
            float_buffer_t *b   = pIDisplay;
            if (b == NULL)
                return false;

            float ni        = float(CURVE_MESH_SIZE) / width; // Normalizing index
            volume          = expf(-0.05f * M_LN10 * volume);

            for (size_t j=0; j<width; ++j)
            {
                size_t k        = j*ni;
                b->v[0][j]      = vFreqMesh[k];
                b->v[1][j]      = vAmpMesh[k];
            }

            dsp::mul_k2(b->v[1], volume, width);
            dsp::fill(b->v[2], 0.0f, width);
            dsp::fill(b->v[3], height, width);
            dsp::axis_apply_log1(b->v[2], b->v[0], zx, dx, width);
            dsp::axis_apply_log1(b->v[3], b->v[1], zy, dy, width);

            // Draw the mesh
            cv->set_color_rgb((bypass) ? CV_SILVER : CV_MESH);
            cv->set_line_width(2);
            cv->draw_lines(b->v[2], b->v[3], width);
        }
        else
        {
            // Draw axis
            cv->set_line_width(1.0);

            float zx    = 1.0f/SPEC_FREQ_MIN;
            float zy    = 1.0f/GAIN_AMP_M_96_DB;
            float dx    = width/(logf(SPEC_FREQ_MAX/SPEC_FREQ_MIN));
            float dy    = height/(logf(GAIN_AMP_M_96_DB/GAIN_AMP_P_12_DB));

            // Draw vertical lines
            cv->set_color_rgb(CV_YELLOW, 0.5f);
            for (float i=100.0f; i<SPEC_FREQ_MAX; i *= 10.0f)
            {
                float ax = dx*(logf(i*zx));
                cv->line(ax, 0, ax, height);
            }

            // Draw horizontal lines
            for (float i=GAIN_AMP_M_96_DB; i<GAIN_AMP_P_12_DB; i *= GAIN_AMP_P_12_DB)
            {
                float ay = height + dy*(logf(i*zy));
                if ((i >= (GAIN_AMP_0_DB - 1e-4)) && ((i <= (GAIN_AMP_0_DB + 1e-4))))
                    cv->set_color_rgb(CV_WHITE, 0.5f);
                else
                    cv->set_color_rgb(CV_YELLOW, 0.5f);
                cv->line(0, ay, width, ay);
            }

            // Allocate buffer: f, a(f), x, y
            pIDisplay           = float_buffer_t::reuse(pIDisplay, 4, width);
            float_buffer_t *b   = pIDisplay;
            if (b == NULL)
                return false;

            float ni        = float(CURVE_MESH_SIZE) / width; // Normalizing index
            for (size_t j=0; j<width; ++j)
            {
                size_t k        = j*ni;
                b->v[0][j]      = vFreqMesh[k];
                b->v[1][j]      = vAmpMesh[k];
            }

            dsp::fill(b->v[2], 0.0f, width);
            dsp::fill(b->v[3], height, width);
            dsp::axis_apply_log1(b->v[2], b->v[0], zx, dx, width);
            dsp::axis_apply_log1(b->v[3], b->v[1], zy, dy, width);

            // Draw the volume line
            volume      = expf(0.05f * M_LN10 * fVolume);
            float vy    = height + dy * logf(volume * zy);
            cv->set_color_rgb((bypass) ? CV_GRAY : CV_GREEN, 0.5f);
            cv->line(0, vy, width, vy);

            // Draw the mesh
            cv->set_color_rgb((bypass) ? CV_SILVER : CV_MESH);
            cv->set_line_width(2);
            cv->draw_lines(b->v[2], b->v[3], width);
        }

        return true;
    }

    void loud_comp_base::dump(IStateDumper *v) const
    {
        v->write("nChannels", nChannels);
        v->write("nMode", nMode);
        v->write("nRank", nRank);
        v->write("fGain", fGain);
        v->write("fVolume", fVolume);
        v->write("bBypass", bBypass);
        v->write("bRelative", bRelative);
        v->write("bReference", bReference);
        v->write("bHClipOn", bHClipOn);
        v->write("fHClipLvl", fHClipLvl);
        v->begin_array("vChannels", vChannels, nChannels);
        for (size_t i=0; i<nChannels; ++i)
        {
            channel_t *c = vChannels[i];
            v->begin_object(c, sizeof(channel_t));
            {
                v->write("vIn", c->vIn);
                v->write("vOut", c->vOut);
                v->write("vDry", c->vDry);
                v->write("vBuffer", c->vBuffer);
                v->write("fInLevel", c->fInLevel);
                v->write("fOutLevel", c->fOutLevel);
                v->write("bHClip", c->bHClip);

                v->write_object("sBypass", &c->sBypass);
                v->write_object("sDelay", &c->sDelay);
                v->write_object("sProc", &c->sProc);
                v->write_object("sClipInd", &c->sClipInd);

                v->write("pIn", c->pIn);
                v->write("pOut", c->pOut);
                v->write("pMeterIn", c->pMeterIn);
                v->write("pMeterOut", c->pMeterOut);
                v->write("pHClipInd", c->pHClipInd);
            }
            v->end_object();
        }
        v->end_array();
        v->write("vTmpBuf", vTmpBuf);
        v->write("vFreqApply", vFreqApply);
        v->write("vFreqMesh", vFreqMesh);
        v->write("vAmpMesh", vAmpMesh);
        v->write("bSyncMesh", bSyncMesh);
        v->write("pIDisplay", pIDisplay);
        v->write_object("sOsc", &sOsc);
        v->write("pData", pData);

        v->write("pBypass", pBypass);
        v->write("pGain", pGain);
        v->write("pMode", pMode);
        v->write("pRank", pRank);
        v->write("pVolume", pVolume);
        v->write("pMesh", pMesh);
        v->write("pRelative", pRelative);
        v->write("pReference", pReference);
        v->write("pHClipOn", pHClipOn);
        v->write("pHClipRange", pHClipRange);
        v->write("pHClipReset", pHClipReset);
    }

    //-------------------------------------------------------------------------
    // Different instances
    loud_comp_mono::loud_comp_mono(): loud_comp_base(metadata, 1)
    {
    }

    loud_comp_stereo::loud_comp_stereo(): loud_comp_base(metadata, 2)
    {
    }
}

