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

#include <metadata/plugins.h>
#include <metadata/ports.h>
#include <metadata/developers.h>

namespace lsp
{
    static const int surge_filter_classes[] = { C_DYNAMICS, -1 };

    static const port_item_t surge_modes[] =
    {
        { "Linear",         "surge.linear"          },
        { "Cubic",          "surge.cubic"           },
        { "Sine",           "surge.sine"            },
        { "Gaussian",       "surge.gaussian"        },
        { "Parabolic",      "surge.parabolic"       },
        { NULL, NULL }
    };

    #define SURGE_FILTER_COMMON(channels)    \
        COMBO("modein", "Fade in mode", 3, surge_modes),      \
        COMBO("modeout", "Fade out mode", 3, surge_modes),      \
        AMP_GAIN("input", "Input gain", 1.0f, GAIN_AMP_P_24_DB), \
        EXT_LOG_CONTROL("thr_on", "Threshold for switching on", U_GAIN_AMP, surge_filter_base_metadata::THRESH), \
        EXT_LOG_CONTROL("thr_off", "Threshold for switching off", U_GAIN_AMP, surge_filter_base_metadata::THRESH), \
        LOG_CONTROL("rms", "RMS estimation time", U_MSEC, surge_filter_base_metadata::RMS), \
        CONTROL("fadein", "Fade in time", U_MSEC, surge_filter_base_metadata::FADEIN), \
        CONTROL("fadeout", "Fade out time", U_MSEC, surge_filter_base_metadata::FADEOUT), \
        CONTROL("fidelay", "Fade in cancel delay time", U_MSEC, surge_filter_base_metadata::PAUSE), \
        CONTROL("fodelay", "Fade out cancel delay time", U_MSEC, surge_filter_base_metadata::PAUSE), \
        BLINK("active", "Activity indicator"), \
        AMP_GAIN("output", "Output gain", 1.0f, GAIN_AMP_P_24_DB), \
        MESH("ig", "Input signal graph", channels+1, surge_filter_base_metadata::MESH_POINTS), \
        MESH("og", "Output signal graph", channels+1, surge_filter_base_metadata::MESH_POINTS), \
        MESH("grg", "Gain reduction graph", 2, surge_filter_base_metadata::MESH_POINTS), \
        MESH("eg", "Envelope graph", 2, surge_filter_base_metadata::MESH_POINTS), \
        SWITCH("grv", "Gain reduction visibility", 1.0f), \
        SWITCH("ev", "Envelope visibility", 1.0f), \
        METER_GAIN("grm", "Gain reduction meter", GAIN_AMP_P_24_DB), \
        METER_GAIN("em", "Envelope meter", GAIN_AMP_P_24_DB)

    static const port_t surge_filter_mono_ports[] =
    {
        PORTS_MONO_PLUGIN,
        BYPASS,
        SURGE_FILTER_COMMON(1),
        SWITCH("igv", "Input graph visibility", 1.0f),
        SWITCH("ogv", "Output graph visibility", 1.0f),
        METER_GAIN("ilm", "Input level meter", GAIN_AMP_P_24_DB),
        METER_GAIN("olm", "Output level meter", GAIN_AMP_P_24_DB),

        PORTS_END
    };

    static const port_t surge_filter_stereo_ports[] =
    {
        PORTS_STEREO_PLUGIN,
        BYPASS,
        SURGE_FILTER_COMMON(2),
        SWITCH("igv_l", "Input graph visibility left", 1.0f),
        SWITCH("ogv_l", "Output graph visibility left", 1.0f),
        METER_GAIN("ilm_l", "Input level meter left", GAIN_AMP_P_24_DB),
        METER_GAIN("olm_l", "Output level meter left", GAIN_AMP_P_24_DB),
        SWITCH("igv_r", "Input graph visibility right", 1.0f),
        SWITCH("ogv_r", "Output graph visibility right", 1.0f),
        METER_GAIN("ilm_r", "Input level meter right", GAIN_AMP_P_24_DB),
        METER_GAIN("olm_r", "Output level meter right", GAIN_AMP_P_24_DB),

        PORTS_END
    };

    const plugin_metadata_t surge_filter_mono_metadata::metadata =
    {
        "Sprungfilter Mono",
        "Surge Filter Mono",
        "SF1M",
        &developers::v_sadovnikov,
        "surge_filter_mono",
        "feli",
        LSP_SURGE_FILTER_BASE + 0,
        LSP_VERSION(1, 0, 0),
        surge_filter_classes,
        E_INLINE_DISPLAY | E_DUMP_STATE,
        surge_filter_mono_ports,
        "util/surge_filter.xml",
        NULL,
        mono_plugin_port_groups
    };

    const plugin_metadata_t surge_filter_stereo_metadata::metadata =
    {
        "Sprungfilter Stereo",
        "Surge Filter Stereo",
        "SF1S",
        &developers::v_sadovnikov,
        "surge_filter_stereo",
        "crjf",
        LSP_SURGE_FILTER_BASE + 1,
        LSP_VERSION(1, 0, 0),
        surge_filter_classes,
        E_INLINE_DISPLAY | E_DUMP_STATE,
        surge_filter_stereo_ports,
        "util/surge_filter.xml",
        NULL,
        stereo_plugin_port_groups
    };

}


