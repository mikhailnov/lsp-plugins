/*
 * Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-plugins
 * Created on: 4 июл. 2020 г.
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

#ifndef PLUGINS_SURGE_FILTER_H_
#define PLUGINS_SURGE_FILTER_H_

#include <core/plugin.h>
#include <core/util/Blink.h>
#include <core/util/Bypass.h>
#include <core/util/Depopper.h>
#include <core/util/MeterGraph.h>
#include <core/util/Delay.h>

#include <metadata/plugins.h>

namespace lsp
{
    class surge_filter_base: public plugin_t, public surge_filter_base_metadata
    {
        protected:
            typedef struct channel_t
            {
                float              *vIn;            // Input buffer
                float              *vOut;           // Output buffer
                float              *vBuffer;        // Buffer for processing
                Bypass              sBypass;        // Bypass
                Delay               sDelay;         // Delay for latency compensation
                Delay               sDryDelay;      // Dry delay
                MeterGraph          sIn;            // Input metering graph
                MeterGraph          sOut;           // Output metering graph
                bool                bInVisible;     // Input signal visibility flag
                bool                bOutVisible;    // Output signal visibility flag

                IPort              *pIn;            // Input port
                IPort              *pOut;           // Output port
                IPort              *pInVisible;     // Input visibility
                IPort              *pOutVisible;    // Output visibility
                IPort              *pMeterIn;       // Input Meter
                IPort              *pMeterOut;      // Output Meter
            } channel_t;

        protected:
            size_t              nChannels;          // Number of channels
            channel_t          *vChannels;          // Array of channels
            float              *vBuffer;            // Buffer for processing
            float              *vEnv;               // Envelope
            float              *vTimePoints;        // Time points
            float               fGainIn;            // Input gain
            float               fGainOut;           // Output gain
            bool                bGainVisible;       // Gain visible
            bool                bEnvVisible;        // Envelope visible
            uint8_t            *pData;              // Allocated data
            float_buffer_t     *pIDisplay;          // Inline display buffer

            MeterGraph          sGain;              // Gain metering graph
            MeterGraph          sEnv;               // Envelop metering graph
            Blink               sActive;            // Activity indicator
            Depopper            sDepopper;          // Depopper module

            IPort              *pModeIn;            // Mode for fade in
            IPort              *pModeOut;           // Mode for fade out
            IPort              *pGainIn;            // Input gain
            IPort              *pGainOut;           // Output gain
            IPort              *pThreshOn;          // Threshold
            IPort              *pThreshOff;         // Threshold
            IPort              *pRmsLen;            // RMS estimation length
            IPort              *pFadeIn;            // Fade in time
            IPort              *pFadeOut;           // Fade out time
            IPort              *pFadeInDelay;       // Fade in time
            IPort              *pFadeOutDelay;      // Fade out time
            IPort              *pActive;            // Active flag
            IPort              *pBypass;            // Bypass port
            IPort              *pMeshIn;            // Input mesh
            IPort              *pMeshOut;           // Output mesh
            IPort              *pMeshGain;          // Gain mesh
            IPort              *pMeshEnv;           // Envelope mesh
            IPort              *pGainVisible;       // Gain mesh visibility
            IPort              *pEnvVisible;        // Envelope mesh visibility
            IPort              *pGainMeter;         // Gain reduction meter
            IPort              *pEnvMeter;          // Envelope meter

        public:
            explicit            surge_filter_base(size_t channels, const plugin_metadata_t &meta);
            virtual            ~surge_filter_base();

            virtual void        init(IWrapper *wrapper);
            virtual void        destroy();

        public:
            virtual void        update_sample_rate(long sr);
            virtual void        update_settings();
            virtual void        process(size_t samples);
            virtual bool        inline_display(ICanvas *cv, size_t width, size_t height);
            virtual void        dump(IStateDumper *v) const;
    };

    class surge_filter_mono: public surge_filter_base, public surge_filter_mono_metadata
    {
        public:
            explicit surge_filter_mono();
    };

    class surge_filter_stereo: public surge_filter_base, public surge_filter_stereo_metadata
    {
        public:
            explicit surge_filter_stereo();
    };
}

#endif /* PLUGINS_SURGE_FILTER_H_ */
