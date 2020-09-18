/*
 * Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-plugins
 * Created on: 19 нояб. 2017 г.
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

#ifndef UI_TK_WIDGETS_LSPFADER_H_
#define UI_TK_WIDGETS_LSPFADER_H_

namespace lsp
{
    namespace tk
    {
        
        class LSPFader: public LSPWidget
        {
            public:
                static const w_class_t    metadata;

            protected:
                enum flags_t
                {
                    F_IGNORE        = 1 << 0,
                    F_PRECISION     = 1 << 1,
                    F_MOVER         = 1 << 2
                };

            protected:
                float           fMin;
                float           fMax;
                float           fValue;
                float           fStep;
                float           fTinyStep;
                ssize_t         nMinSize;
                size_t          nAngle;
                ssize_t         nLastV;
                size_t          nButtons;
                size_t          nBtnLength;
                size_t          nBtnWidth;
                size_t          nXFlags;
                float           fLastValue;
                float           fCurrValue;

                LSPColor        sColor;

            protected:
                float           limit_value(float value);
                float           get_normalized_value();
                bool            check_mouse_over(ssize_t x, ssize_t y);
                void            do_destroy();
                void            update();
                void            update_cursor_state(ssize_t x, ssize_t y, bool set);

                static status_t slot_on_change(LSPWidget *sender, void *ptr, void *data);

            public:
                explicit LSPFader(LSPDisplay *dpy);
                virtual ~LSPFader();

                virtual status_t        init();
                virtual void            destroy();

            public:
                inline float            value() const { return fValue; }
                inline float            step() const { return fStep; }
                inline float            tiny_step() const { return fTinyStep; }
                inline float            min_value() const { return fMin; }
                inline float            max_value() const { return fMax; }
                inline LSPColor        *color() { return &sColor; }
                inline size_t           angle() const { return nAngle; }
                inline size_t           button_length() const { return nBtnLength; }
                inline size_t           button_width() const { return nBtnWidth; }
                virtual mouse_pointer_t active_cursor() const;

            public:
                void                    set_value(float value);
                void                    set_default_value(float value);
                void                    set_step(float value);
                void                    set_tiny_step(float value);
                void                    set_min_value(float value);
                void                    set_max_value(float value);
                void                    set_min_size(ssize_t value);
                void                    set_angle(size_t value);
                void                    set_button_width(size_t value);
                void                    set_button_length(size_t value);

            public:
                virtual void size_request(size_request_t *r);

                virtual status_t on_change();

                virtual status_t on_mouse_down(const ws_event_t *e);

                virtual status_t on_mouse_up(const ws_event_t *e);

                virtual status_t on_mouse_move(const ws_event_t *e);

                virtual status_t on_mouse_scroll(const ws_event_t *e);

                virtual void draw(ISurface *s);
        };
    
    } /* namespace tk */
} /* namespace lsp */

#endif /* UI_TK_WIDGETS_LSPFADER_H_ */
