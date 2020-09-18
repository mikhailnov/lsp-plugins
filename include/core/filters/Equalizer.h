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

#ifndef CORE_FILTERS_EQUALIZER_H_
#define CORE_FILTERS_EQUALIZER_H_

#include <core/IStateDumper.h>
#include <core/filters/FilterBank.h>
#include <core/filters/Filter.h>

namespace lsp
{
    enum equalizer_mode_t
    {
        EQM_BYPASS, // Bypass signal
        EQM_IIR,    // All filters are recursive filters with infinite impulse response filters
        EQM_FIR,    // All filters are non-recursive filters with finite impulse response filters
        EQM_FFT     // Approximation of the frequency chart in the frequency range
    };

    /**
     * Set of multiple controllable sequential filters combined into one managed object
     */
    class Equalizer
    {
        private:
            Equalizer & operator = (const Equalizer &);

        protected:
            enum eq_flags_t
            {
                EF_REBUILD = 1 << 0,
                EF_CLEAR   = 1 << 1
            };

        protected:
            FilterBank          sBank;              // Filter bank
            Filter             *vFilters;           // List of filters
            size_t              nFilters;           // Number of filters
            size_t              nSampleRate;        // Sample rate
            size_t              nConvSize;          // Convolution size
            size_t              nFftRank;           // FFT rank
            size_t              nLatency;           // Equalizer latency
            size_t              nBufSize;           // Buffer size
            equalizer_mode_t    nMode;              // Equalizer mode
            float              *vFftRe;             // FFT buffer (real part)
            float              *vFftIm;             // FFT buffer (imaginary part)
            float              *vConvRe;            // Convolution (real part)
            float              *vConvIm;            // Convolution (imaginary part)
            float              *vBuffer;            // Processing buffer
            float              *vTmp;               // Temporary buffer for various calculations
            float              *pData;              // Allocation data
            size_t              nFlags;             // Flag that identifies that equalizer has to be rebuilt

        protected:
            void                reconfigure();

        public:
            explicit Equalizer();
            ~Equalizer();

            /**
             * Construct the object being part of memory chunk
             */
            void                construct();

            /** Initialize equalizer
             *
             * @param filters number of filters
             * @param conv_rank convolution size rank
             * @return true on success
             */
            bool                init(size_t filters, size_t conv_rank);

            /** Destroy equalizer
             *
             */
            void                destroy();

        public:
            /** Update filter parameters
             * @param id ID of the filter
             * @param params  filter parameters
             * @return true on success
             */
            bool                set_params(size_t id, const filter_params_t *params);

            /** Get filter parameters
             * @param id ID of the filter
             * @param params  filter parameters
             * @return true on success
             */
            bool                get_params(size_t id, filter_params_t *params);

            /** Check that filter is active
             *
             * @param id ID of filter
             * @return true if filter is active
             */
            inline bool         filter_active(size_t id) const { return (id < nFilters) ? vFilters[id].active() : false; }

            /** Check that filter is inactive
             *
             * @param id ID of filter
             * @return true if filter is inactive
             */
            inline bool         filter_inactive(size_t id) const { return (id < nFilters) ? vFilters[id].inactive() : false; }

            /** Set equalizer mode
             *
             * @param mode equalizer mode
             */
            void                set_mode(equalizer_mode_t mode);

            /** Set sample rate
             *
             * @param sr sample rate
             */
            void                set_sample_rate(size_t sr);

            /** Get equalizer mode
             *
             * @return equalizer mode
             */
            inline equalizer_mode_t get_mode() const { return nMode; }

            /** Get equalizer latency
             *
             * @return equalizer latency
             */
            inline size_t       get_latency() const { return nLatency; }

            /** Get frequency chart of the specific filter
             *
             * @param id ID of the filter
             * @param re real part of the frequency chart
             * @param im imaginary part of the frequency chart
             * @param f frequencies to calculate value
             * @param count number of dots for the chart
             * @return status of operation
             */
            bool                freq_chart(size_t id, float *re, float *im, const float *f, size_t count);

            /** Get frequency chart of the specific filter
             *
             * @param id ID of the filter
             * @param c complex numbers that contain the filter transfer function
             * @param f frequencies to calculate filter transfer function
             * @param count number of points
             * @return status of operation
             */
            bool                freq_chart(size_t id, float *c, const float *f, size_t count);

            /**
             * Get frequency chart of the whole equalizer
             * @param re real part of the frequency chart
             * @param im imaginary part of the frequency chart
             * @param f frequencies to calculate value
             * @param count number of dots for the chart
             */
            void                freq_chart(float *re, float *im, const float *f, size_t count);

            /**
             * Get frequency chart of the whole equalizer
             * @param c complex numbers that contain the filter transfer function
             * @param f frequencies to calculate filter transfer function
             * @param count number of points
             */
            void                freq_chart(float *c, const float *f, size_t count);

            /** Process the signal
             *
             * @param out output signal samples
             * @param in input signal samples
             * @param samples number of samples to process
             */
            void                process(float *out, const float *in, size_t samples);

            /**
             * Dump the state
             * @param dumper dumper
             */
            void                dump(IStateDumper *v) const;
    };

} /* namespace lsp */

#endif /* CORE_FILTERS_EQUALIZER_H_ */
