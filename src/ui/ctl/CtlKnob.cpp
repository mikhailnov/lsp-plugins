/*
 * Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-plugins
 * Created on: 11 июл. 2017 г.
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

#include <ui/ctl/ctl.h>

namespace lsp
{
    namespace ctl
    {
        const ctl_class_t CtlKnob::metadata = { "CtlKnob", &CtlWidget::metadata };

        CtlKnob::CtlKnob(CtlRegistry *src, LSPKnob *widget): CtlWidget(src, widget)
        {
            pClass          = &metadata;
            pPort           = NULL;
            bLog            = false;
            bLogSet         = false;
            bCyclingSet     = false;
            fDefaultValue   = 0.0f;
            bBalanceSet     = false;
            fBalance        = 0.0f;
        }

        CtlKnob::~CtlKnob()
        {
        }

        status_t CtlKnob::slot_change(LSPWidget *sender, void *ptr, void *data)
        {
            CtlKnob *_this      = static_cast<CtlKnob *>(ptr);
            if (_this != NULL)
                _this->submit_value();
            return STATUS_OK;
        }

        status_t CtlKnob::slot_dbl_click(LSPWidget *sender, void *ptr, void *data)
        {
            CtlKnob *_this      = static_cast<CtlKnob *>(ptr);
            if (_this != NULL)
                _this->set_default_value();
            return STATUS_OK;
        }

        void CtlKnob::submit_value()
        {
            if (pPort == NULL)
                return;
            if (pWidget == NULL)
                return;

            LSPKnob *knob   = widget_cast<LSPKnob>(pWidget);
            float value     = knob->value();

            const port_t *p = pPort->metadata();
            if (p == NULL)
            {
                pPort->set_value(value);
                pPort->notify_all();
                return;
            }

            if (is_gain_unit(p->unit)) // Gain
            {
                double base     = (p->unit == U_GAIN_AMP) ? M_LN10 * 0.05 : M_LN10 * 0.1;
                value           = exp(value * base);
                float min       = (p->flags & F_LOWER) ? p->min : 0.0f;
                if ((min <= 0.0f) && (value < GAIN_AMP_M_80_DB))
                    value           = 0.0f;
            }
            else if (is_discrete_unit(p->unit)) // Integer type
            {
                 value          = truncf(value);
            }
            else if (bLog)  // Float and other values, logarithmic
            {
                value           = exp(value);
                float min       = (p->flags & F_LOWER) ? p->min : 0.0f;
                if ((min <= 0.0f) && (value < log(GAIN_AMP_M_80_DB)))
                    value           = 0.0f;
            }

            pPort->set_value(value);
            pPort->notify_all();
        }

        void CtlKnob::set_default_value()
        {
            LSPKnob *knob = widget_cast<LSPKnob>(pWidget);
            if (knob == NULL)
                return;

            const port_t *p = pPort->metadata();
            float dfl   = (p != NULL) ? pPort->get_default_value() : fDefaultValue;
            float value = dfl;

            if (p != NULL)
            {
                if (is_gain_unit(p->unit)) // Decibels
                {
                    double base = (p->unit == U_GAIN_AMP) ? 20.0 / M_LN10 : 10.0 / M_LN10;

                    if (value < GAIN_AMP_M_120_DB)
                        value           = GAIN_AMP_M_120_DB;

                    value = base * log(value);
                }
                else if (bLog)
                {
                    if (value < GAIN_AMP_M_120_DB)
                        value           = GAIN_AMP_M_120_DB;
                    value = log(value);
                }
            }

            knob->set_value(value);
            pPort->set_value(dfl);
            pPort->notify_all();
        }

        void CtlKnob::commit_value(float value)
        {
            if (pWidget == NULL)
                return;

            LSPKnob *knob = widget_cast<LSPKnob>(pWidget);
            if (knob == NULL)
                return;

            const port_t *p = pPort->metadata();
            if (p == NULL)
                return;

            if (is_gain_unit(p->unit)) // Decibels
            {
                double base = (p->unit == U_GAIN_AMP) ? 20.0 / M_LN10 : 10.0 / M_LN10;

                if (value < GAIN_AMP_M_120_DB)
                    value           = GAIN_AMP_M_120_DB;

                knob->set_value(base * log(value));
            }
            else if (is_discrete_unit(p->unit)) // Integer type
                knob->set_value(truncf(value));
            else if (bLog)
            {
                if (value < GAIN_AMP_M_120_DB)
                    value           = GAIN_AMP_M_120_DB;
                knob->set_value(log(value));
            }
            else
                knob->set_value(value);
        }

        void CtlKnob::init()
        {
            CtlWidget::init();

            LSPKnob *knob = widget_cast<LSPKnob>(pWidget);
            if (knob == NULL)
                return;

            // Initialize color controllers
            sColor.init_hsl(pRegistry, knob, knob->color(), A_COLOR, A_HUE_ID, A_SAT_ID, A_LIGHT_ID);
            sScaleColor.init_hsl(pRegistry, knob, knob->scale_color(), A_SCALE_COLOR, A_SCALE_HUE_ID, A_SCALE_SAT_ID, A_SCALE_LIGHT_ID);
            sScaleColor.map_static_hsl(A_SCALE_HUE, -1, -1);

            // Bind slots
            knob->slots()->bind(LSPSLOT_CHANGE, slot_change, this);
            knob->slots()->bind(LSPSLOT_MOUSE_DBL_CLICK, slot_dbl_click, this);
        }

        void CtlKnob::set(widget_attribute_t att, const char *value)
        {
            LSPKnob *knob = widget_cast<LSPKnob>(pWidget);

            switch (att)
            {
                case A_ID:
                    BIND_PORT(pRegistry, pPort, value);
                    break;
                case A_SIZE:
                    if (knob != NULL)
                        PARSE_INT(value, knob->set_size(__));
                    break;
                case A_VALUE:
                    if (knob != NULL)
                        PARSE_FLOAT(value, knob->set_value(__));
                    break;
                case A_DEFAULT:
                    if (knob != NULL)
                        PARSE_FLOAT(value, fDefaultValue = __);
                    break;
                case A_MIN:
                    if (knob != NULL)
                        PARSE_FLOAT(value, knob->set_min_value(__));
                    break;
                case A_MAX:
                    if (knob != NULL)
                        PARSE_FLOAT(value, knob->set_max_value(__));
                    break;
                case A_LOGARITHMIC:
                    PARSE_BOOL(value, bLog = __);
                    bLogSet     = true;
                    break;
                case A_STEP:
                    if (knob != NULL)
                        PARSE_FLOAT(value, knob->set_step(__));
                    break;
                case A_TINY_STEP:
                    if (knob != NULL)
                        PARSE_FLOAT(value, knob->set_tiny_step(__));
                    break;
                case A_BALANCE:
                    bBalanceSet = true;
                    if (knob != NULL)
                        PARSE_FLOAT(value, knob->set_balance(fBalance = __));
                    break;
                case A_CYCLE:
                    bCyclingSet = true;
                    if (knob != NULL)
                        PARSE_BOOL(value, knob->set_cycling(__); );
                    break;
                default:
                {
                    sColor.set(att, value);
                    sScaleColor.set(att, value);
                    CtlWidget::set(att, value);
                    break;
                }
            }
        }

        void CtlKnob::notify(CtlPort *port)
        {
            CtlWidget::notify(port);

            if (port == pPort)
                commit_value(pPort->get_value());
        }

        static inline float limit(float v, float min, float max)
        {
            if (min < max)
            {
                if (v < min)
                    return min;
                return (v > max) ? max : v;
            }
            else
            {
                if (v < max)
                    return max;
                return (v > min) ? min : v;
            }
        }

        void CtlKnob::end()
        {
            // Call parent controller
            CtlWidget::end();

            // Ensure that widget is set
            if (pWidget == NULL)
                return;
            LSPKnob *knob = widget_cast<LSPKnob>(pWidget);

            // Ensure that port is set
            const port_t *p = (pPort != NULL) ? pPort->metadata() : NULL;
            if (p == NULL)
            {
                knob->set_value(fDefaultValue);
                return;
            }

            // Update logarithmic flag
            if (!bLogSet)
                bLog        = (p->flags & F_LOG);

            if (is_gain_unit(p->unit)) // Gain
            {
                double base     = (p->unit == U_GAIN_AMP) ? 20.0 / M_LN10 : 10.0 / M_LN10;

                float min       = (p->flags & F_LOWER) ? p->min : 0.0f;
                float max       = (p->flags & F_UPPER) ? p->max : GAIN_AMP_P_12_DB;
                float dfl       = (bBalanceSet) ? fBalance : min;

                double step     = base * log((p->flags & F_STEP) ? p->step + 1.0f : 1.01f) * 0.1f;
                double thresh   = ((p->flags & F_EXT) ? GAIN_AMP_M_140_DB : GAIN_AMP_M_80_DB);

                double db_min   = (fabs(min) < thresh) ? (base * log(thresh) - step) : (base * log(min));
                double db_max   = (fabs(max) < thresh) ? (base * log(thresh) - step) : (base * log(max));
                double db_dfl   = (fabs(max) < thresh) ? (base * log(thresh) - step) : (base * log(dfl));
                float balance   = limit(db_dfl, db_min, db_max);

                knob->set_min_value(db_min);
                knob->set_max_value(db_max);
                knob->set_balance(balance);
                knob->set_step(step * 10.0f);
                knob->set_tiny_step(step);
                fDefaultValue   = base * log(p->start);
            }
            else if (is_discrete_unit(p->unit)) // Integer type
            {
                float min       = (p->flags & F_LOWER) ? p->min : 0.0f;
                float max       = (p->unit == U_ENUM) ? min + list_size(p->items) - 1.0f :
                                  (p->flags & F_UPPER) ? p->max : 1.0f;
                float dfl       = (bBalanceSet) ? fBalance : p->min;
                float balance   = limit(dfl, min, max);
                ssize_t step    = (p->flags & F_STEP) ? p->step : 1;
                step            = (step == 0) ? 1 : step;

                knob->set_min_value(min);
                knob->set_max_value(max);
                knob->set_balance(balance);
                knob->set_step(step);
                knob->set_tiny_step(step);
                fDefaultValue   = p->start;
                if (!bCyclingSet)
                    knob->set_cycling(p->flags & F_CYCLIC);
            }
            else if (bLog)  // Float and other values, logarithmic
            {
                float min       = (p->flags & F_LOWER) ? p->min : 0.0f;
                float max       = (p->flags & F_UPPER) ? p->max : GAIN_AMP_P_12_DB;
                float dfl       = (bBalanceSet) ? fBalance : min;

                double step     = log((p->flags & F_STEP) ? p->step + 1.0f : 1.01f);
                double l_min    = (fabs(min) < GAIN_AMP_M_80_DB) ? log(GAIN_AMP_M_80_DB) - step : log(min);
                double l_max    = (fabs(max) < GAIN_AMP_M_80_DB) ? log(GAIN_AMP_M_80_DB) - step : log(max);
                double l_dfl    = (fabs(dfl) < GAIN_AMP_M_80_DB) ? log(GAIN_AMP_M_80_DB) - step : log(dfl);
                float balance   = limit(l_dfl, l_min, l_max);

                knob->set_min_value(l_min);
                knob->set_max_value(l_max);
                knob->set_balance(balance);
                knob->set_step(step * 10.0f);
                knob->set_tiny_step(step);
                fDefaultValue = log(p->start);
            }
            else // Float and other values, non-logarithmic
            {
                float min       = (p->flags & F_LOWER) ? p->min : 0.0f;
                float max       = (p->flags & F_UPPER) ? p->max : 1.0f;
                float dfl       = (bBalanceSet) ? fBalance : min;
                float balance   = limit(dfl, min, max);

                knob->set_min_value(min);
                knob->set_max_value(max);
                knob->set_balance(balance);
                knob->set_tiny_step((p->flags & F_STEP) ? p->step : (max - min) * 0.01f);
                knob->set_step(knob->tiny_step() * 10.0f);
                fDefaultValue   = p->start;
                if (!bCyclingSet)
                    knob->set_cycling(p->flags & F_CYCLIC);
            }

            // Set default value to the knob
            knob->set_value(fDefaultValue);
        }

    } /* namespace ctl */
} /* namespace lsp */
