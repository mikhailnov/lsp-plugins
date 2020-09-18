/*
 * Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-plugins
 * Created on: 21 апр. 2017 г.
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

#ifndef CORE_FILES_3D_OBJFILEPARSER_H_
#define CORE_FILES_3D_OBJFILEPARSER_H_

#include <core/buffer.h>
#include <core/files/3d/IObjHandler.h>
#include <core/status.h>
#include <data/cstorage.h>
#include <data/cvector.h>

#include <core/io/Path.h>
#include <core/io/IInSequence.h>
#include <core/LSPString.h>

namespace lsp
{
    namespace obj
    {
        class Parser
        {
            protected:
                typedef struct file_buffer_t
                {
                    io::IInSequence    *in;
                    LSPString           line;
                    lsp_wchar_t        *data;
                    size_t              off;
                    size_t              len;
                    bool                skip_wc;
                } file_buffer_t;

                struct ofp_point3d_t: public point3d_t
                {
                    ssize_t     oid;    // Object identifier
                    ssize_t     idx;    // Point index
                };

                struct ofp_vector3d_t: public vector3d_t
                {
                    ssize_t     oid;    // Object identifier
                    ssize_t     idx;    // Vector index
                };

                typedef struct parse_state_t
                {
                    IObjHandler             *pHandler;
                    ssize_t                     nObjectID;
                    ssize_t                     nPointID;
                    ssize_t                     nFaceID;
                    ssize_t                     nLineID;
                    size_t                      nLines;

                    cstorage<ofp_point3d_t>     sVx;
                    cstorage<ofp_point3d_t>     sTexVx;
                    cstorage<ofp_point3d_t>     sParVx;
                    cstorage<ofp_vector3d_t>    sNorm;

                    cstorage<ssize_t>           sVxIdx;
                    cstorage<ssize_t>           sTexVxIdx;
                    cstorage<ssize_t>           sNormIdx;
                } parse_state_t;

            protected:
                static void eliminate_comments(LSPString *s);

                static status_t read_line(file_buffer_t *fb);

                static status_t parse_lines(file_buffer_t *fb, IObjHandler *handler);

                static status_t parse_line(parse_state_t *st, const char *s);

                static status_t parse_finish(parse_state_t *st);

                static const char *skip_spaces(const char *s);

                static inline bool is_space(char ch);

                static inline bool end_of_line(const char *s);

                static inline bool prefix_match(const char *s, const char *prefix);
    
                static bool parse_float(float *dst, const char **s);

                static bool parse_int(ssize_t *dst, const char **s);

            public:
                static status_t parse(const char *path, IObjHandler *handler);

                static status_t parse(const LSPString *path, IObjHandler *handler);

                static status_t parse(const io::Path *path, IObjHandler *handler);
        };
    }
} /* namespace lsp */

#endif /* CORE_FILES_3D_OBJFILEPARSER_H_ */
