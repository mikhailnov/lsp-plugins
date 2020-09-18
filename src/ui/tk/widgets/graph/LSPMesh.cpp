/*
 * Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-plugins
 * Created on: 19 июл. 2017 г.
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

#include <ui/tk/tk.h>
#include <dsp/dsp.h>

namespace lsp
{
    namespace tk
    {
        const w_class_t LSPMesh::metadata = { "LSPMesh", &LSPGraphItem::metadata };

        LSPMesh::LSPMesh(LSPDisplay *dpy): LSPGraphItem(dpy)
        {
            nWidth      = 1;
            nCenter     = 0;
            nDimensions = 0;
            nPoints     = 0;
            nXIndex     = -1;
            nYIndex     = -1;
            vBuffer     = NULL;
            nBufSize    = 0;

            pClass          = &metadata;
        }

        LSPMesh::~LSPMesh()
        {
            drop_data();
        }

        status_t LSPMesh::init()
        {
            status_t result = LSPGraphItem::init();
            if (result != STATUS_OK)
                return result;

            init_color(C_GRAPH_MESH, &sColor);
            return STATUS_OK;
        }

        void LSPMesh::drop_data()
        {
            if (vBuffer != NULL)
            {
                free(vBuffer);
                vBuffer     = NULL;
            }
            nBufSize    = 0;
        }

        void LSPMesh::destroy()
        {
            drop_data();
            LSPGraphItem::destroy();
        }

        void LSPMesh::set_line_width(size_t value)
        {
            if (nWidth == value)
                return;
            nWidth = value;
            query_draw();
        }

        void LSPMesh::set_x_index(ssize_t value)
        {
            if (nXIndex == value)
                return;
            nXIndex = value;
            query_draw();
        }

        void LSPMesh::set_y_index(ssize_t value)
        {
            if (nYIndex == value)
                return;
            nYIndex = value;
            query_draw();
        }

        void LSPMesh::set_center_id(size_t value)
        {
            if (nCenter == value)
                return;
            nCenter = value;
            query_draw();
        }

        status_t LSPMesh::set_data(size_t dimensions, size_t points, const float **data)
        {
//            lsp_trace("dimensions = %d, points=%d, data=%p", int(dimensions), int(points), data);
            size_t k=0;
            if (nXIndex >= 0)
                ++k;
            if (nYIndex >= 0)
                ++k;

            size_t n = (k > 0) ? k : dimensions;

            // Allocate data
            size_t vec_size = ALIGN_SIZE(points, DEFAULT_ALIGN);
            size_t new_size = (n + 2) * vec_size;

            if (new_size > nBufSize)
            {
                if (vBuffer != NULL)
                {
                    float *ptr  = reinterpret_cast<float *>(::realloc(vBuffer, new_size * sizeof(float)));
                    if (ptr == NULL)
                        return STATUS_NO_MEM;
                    vBuffer     = ptr;
                }
                else
                {
                    vBuffer     = reinterpret_cast<float *>(::malloc(new_size * sizeof(float)));
                    if (vBuffer == NULL)
                        return STATUS_NO_MEM;
                }

                nBufSize    = new_size;
            }

            // Now we are ready to store data
            float *dst      = vBuffer;
            if (k > 0)
            {
                // Copy just specific data
                if (nXIndex >= 0)
                {
                    if (nXIndex < ssize_t(dimensions))
                        dsp::copy(dst, data[nXIndex], points);
                    else
                        dsp::fill_zero(dst, points);
                    dst            += vec_size;
                }
                if (nYIndex >= 0)
                {
                    if (nYIndex < ssize_t(dimensions))
                        dsp::copy(dst, data[nYIndex], points);
                    else
                        dsp::fill_zero(dst, points);
                    dst            += vec_size;
                }
            }
            else
            {
                // Copy all data
                for (size_t i=0; i<n; ++i)
                {
                    dsp::copy(dst, data[i], points);
                    dst            += vec_size;
                }
            }

            // Store dimensions
            nDimensions     = n;
            nPoints         = points;

            query_draw();
            return STATUS_OK;
        }

        void LSPMesh::render(ISurface *s, bool force)
        {
            // Get graph
            LSPGraph *cv = graph();
            if (cv == NULL)
            {
                lsp_trace("cv == null");
                return;
            }

            // Prepare palette
            Color color(sColor);
            color.scale_lightness(brightness());

            // Determine number of dimensions
            size_t basis    = cv->basis_axes();
            size_t n_vecs   = (nDimensions > basis) ? basis : nDimensions;
            if (n_vecs <= 0)
            {
//                lsp_trace("n_vecs <= 0");
                return;
            }

            float cx = 0.0f, cy = 0.0f;
            cv->center(nCenter, &cx, &cy);

            // Ensure that we have enough buffer size
            size_t vec_size     = ALIGN_SIZE(nPoints, DEFAULT_ALIGN);

            // Initialize dimensions as zeros
            float *x_vec        = &vBuffer[nDimensions * vec_size];
            float *y_vec        = &x_vec[vec_size];
            dsp::fill(x_vec, cx, nPoints);
            dsp::fill(y_vec, cy, nPoints);

            // Calculate dot coordinates
            const float *src = vBuffer;
            for (size_t i=0; i<n_vecs; ++i)
            {
                // Try to get new axis
                LSPAxis *axis   = cv->basis_axis(i);
                if (axis == NULL) // There is no axis, leave cycle
                {
                    lsp_trace("axis == null");
                    break;
                }

                // Try to apply axis
                if (!axis->apply(x_vec, y_vec, src, nPoints))
                {
                    lsp_trace("!axis->apply");
                    return;
                }
                src += vec_size;
            }

            // Now we have dots in x_vec[] and y_vec[]
            bool aa = s->set_antialiasing(bSmooth);
            if (sColor.alpha() <= 0.0f)
                s->wire_poly(x_vec, y_vec, nPoints, nWidth, color);
            else
            {
                Color wire(sColor, 0.0f);
                s->draw_poly(x_vec, y_vec, nPoints, nWidth, color, wire);
            }
            s->set_antialiasing(aa);
        }

    } /* namespace tk */
} /* namespace lsp */
