/*
 * Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-plugins
 * Created on: 3 нояб. 2016 г.
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
#include <metadata/plugins.h>
#include <metadata/ports.h>
#include <metadata/developers.h>

namespace lsp
{
    //-------------------------------------------------------------------------
    // Expander
    static const int expander_classes[] = { C_EXPANDER, -1 };

    static const port_item_t exp_sc_modes[] =
    {
        { "Peak",       "sidechain.peak"           },
        { "RMS",        "sidechain.rms"            },
        { "Low-Pass",   "sidechain.lowpass"        },
        { "Uniform",    "sidechain.uniform"        },
        { NULL, NULL }
    };

    static const port_item_t exp_sc_sources[] =
    {
        { "Middle",     "sidechain.middle" },
        { "Side",       "sidechain.side" },
        { "Left",       "sidechain.left" },
        { "Right",      "sidechain.right" },
        { NULL, NULL }
    };

    static const port_item_t exp_sc_type[] =
    {
        { "Internal",   "sidechain.internal" },
        { "External",   "sidechain.external" },
        { NULL, NULL }
    };

    static const port_item_t exp_modes[] =
    {
        { "Down",       "expander.down_ward" },
        { "Up",         "expander.up_ward" },
        { NULL, NULL }
    };

    static const port_item_t exp_filter_slope[] =
    {
        { "off",        "eq.slope.off"      },
        { "12 dB/oct",  "eq.slope.12dbo"    },
        { "24 dB/oct",  "eq.slope.24dbo"    },
        { "36 dB/oct",  "eq.slope.36dbo"    },
        { NULL, NULL }
    };

    #define EXP_COMMON     \
        BYPASS,             \
        IN_GAIN,            \
        OUT_GAIN,           \
        SWITCH("pause", "Pause graph analysis", 0.0f), \
        TRIGGER("clear", "Clear graph analysis")

    #define EXP_MS_COMMON  \
        EXP_COMMON,        \
        SWITCH("msl", "Mid/Side listen", 0.0f)

    #define EXP_MONO_CHANNEL \
        COMBO("scm", "Sidechain mode", expander_base_metadata::SC_MODE_DFL, exp_sc_modes), \
        CONTROL("sla", "Sidechain lookahead", U_MSEC, expander_base_metadata::LOOKAHEAD), \
        SWITCH("scl", "Sidechain listen", 0.0f), \
        LOG_CONTROL("scr", "Sidechain reactivity", U_MSEC, expander_base_metadata::REACTIVITY), \
        AMP_GAIN100("scp", "Sidechain preamp", GAIN_AMP_0_DB), \
        COMBO("shpm", "High-pass filter mode", 0, exp_filter_slope),      \
        LOG_CONTROL("shpf", "High-pass filter frequency", U_HZ, expander_base_metadata::HPF),   \
        COMBO("slpm", "Low-pass filter mode", 0, exp_filter_slope),      \
        LOG_CONTROL("slpf", "Low-pass filter frequency", U_HZ, expander_base_metadata::LPF)

    #define EXP_SC_MONO_CHANNEL \
        COMBO("sci", "Sidechain input", expander_base_metadata::SC_TYPE_DFL, exp_sc_type), \
        EXP_MONO_CHANNEL

    #define EXP_STEREO_CHANNEL(id, label) \
        COMBO("scm" id, "Sidechain mode" label, expander_base_metadata::SC_MODE_DFL, exp_sc_modes), \
        CONTROL("sla" id, "Sidechain lookahead" label, U_MSEC, expander_base_metadata::LOOKAHEAD), \
        SWITCH("scl" id, "Sidechain listen" label, 0.0f), \
        COMBO("scs" id, "Sidechain source" label, expander_base_metadata::SC_SOURCE_DFL, exp_sc_sources), \
        LOG_CONTROL("scr" id, "Sidechain reactivity" label, U_MSEC, expander_base_metadata::REACTIVITY), \
        AMP_GAIN100("scp" id, "Sidechain preamp" label, GAIN_AMP_0_DB), \
        COMBO("shpm" id, "High-pass filter mode" label, 0, exp_filter_slope),      \
        LOG_CONTROL("shpf" id, "High-pass filter frequency" label, U_HZ, expander_base_metadata::HPF),   \
        COMBO("slpm" id, "Low-pass filter mode" label, 0, exp_filter_slope),      \
        LOG_CONTROL("slpf" id, "Low-pass filter frequency" label, U_HZ, expander_base_metadata::LPF)

    #define EXP_SC_STEREO_CHANNEL(id, label) \
        COMBO("sci" id, "Sidechain input" label, expander_base_metadata::SC_TYPE_DFL, exp_sc_type), \
        EXP_STEREO_CHANNEL(id, label)

    #define EXP_CHANNEL(id, label) \
        COMBO("em" id, "Expander mode" label, expander_base_metadata::EM_DEFAULT, exp_modes), \
        LOG_CONTROL("al" id, "Attack threshold" label, U_GAIN_AMP, expander_base_metadata::ATTACK_LVL), \
        LOG_CONTROL("at" id, "Attack time" label, U_MSEC, expander_base_metadata::ATTACK_TIME), \
        LOG_CONTROL("rrl" id, "Release threshold" label, U_GAIN_AMP, expander_base_metadata::RELEASE_LVL), \
        LOG_CONTROL("rt" id, "Release time" label, U_MSEC, expander_base_metadata::RELEASE_TIME), \
        LOG_CONTROL("er" id, "Ratio" label, U_NONE, expander_base_metadata::RATIO), \
        LOG_CONTROL("kn" id, "Knee" label, U_GAIN_AMP, expander_base_metadata::KNEE), \
        LOG_CONTROL("mk" id, "Makeup gain" label, U_GAIN_AMP, expander_base_metadata::MAKEUP), \
        AMP_GAIN10("cdr" id, "Dry gain" label, GAIN_AMP_M_INF_DB),     \
        AMP_GAIN10("cwt" id, "Wet gain" label, GAIN_AMP_0_DB), \
        METER_OUT_GAIN("rl" id, "Release level" label, 20.0f), \
        SWITCH("slv" id, "Sidechain level visibility" label, 1.0f), \
        SWITCH("elv" id, "Envelope level visibility" label, 1.0f), \
        SWITCH("grv" id, "Gain reduction visibility" label, 1.0f), \
        MESH("ecg" id, "Expander curve graph" label, 2, expander_base_metadata::CURVE_MESH_SIZE), \
        MESH("scg" id, "Expander sidechain graph" label, 2, expander_base_metadata::TIME_MESH_SIZE), \
        MESH("evg" id, "Expander envelope graph" label, 2, expander_base_metadata::TIME_MESH_SIZE), \
        MESH("grg" id, "Expander gain reduciton" label, 2, expander_base_metadata::TIME_MESH_SIZE), \
        METER_OUT_GAIN("slm" id, "Sidechain level meter" label, GAIN_AMP_P_24_DB), \
        METER_OUT_GAIN("clm" id, "Curve level meter" label, GAIN_AMP_P_24_DB), \
        METER_OUT_GAIN("elm" id, "Envelope level meter" label, GAIN_AMP_P_24_DB), \
        METER_GAIN_DFL("rlm" id, "Reduction level meter" label, GAIN_AMP_P_48_DB, GAIN_AMP_0_DB)

    #define EXP_AUDIO_METER(id, label) \
        SWITCH("ilv" id, "Input level visibility" label, 1.0f), \
        SWITCH("olv" id, "Output level visibility" label, 1.0f), \
        MESH("icg" id, "Expander input" label, 2, expander_base_metadata::TIME_MESH_SIZE), \
        MESH("ocg" id, "Expander output" label, 2, expander_base_metadata::TIME_MESH_SIZE), \
        METER_GAIN("ilm" id, "Input level meter" label, GAIN_AMP_P_24_DB), \
        METER_GAIN("olm" id, "Output level meter" label, GAIN_AMP_P_24_DB)

    static const port_t expander_mono_ports[] =
    {
        PORTS_MONO_PLUGIN,
        EXP_COMMON,
        EXP_MONO_CHANNEL,
        EXP_CHANNEL("", ""),
        EXP_AUDIO_METER("", ""),

        PORTS_END
    };

    static const port_t expander_stereo_ports[] =
    {
        PORTS_STEREO_PLUGIN,
        EXP_COMMON,
        EXP_STEREO_CHANNEL("", ""),
        EXP_CHANNEL("", ""),
        EXP_AUDIO_METER("_l", " Left"),
        EXP_AUDIO_METER("_r", " Right"),

        PORTS_END
    };

    static const port_t expander_lr_ports[] =
    {
        PORTS_STEREO_PLUGIN,
        EXP_COMMON,
        EXP_STEREO_CHANNEL("_l", " Left"),
        EXP_STEREO_CHANNEL("_r", " Right"),
        EXP_CHANNEL("_l", " Left"),
        EXP_CHANNEL("_r", " Right"),
        EXP_AUDIO_METER("_l", " Left"),
        EXP_AUDIO_METER("_r", " Right"),

        PORTS_END
    };

    static const port_t expander_ms_ports[] =
    {
        PORTS_STEREO_PLUGIN,
        EXP_MS_COMMON,
        EXP_STEREO_CHANNEL("_m", " Mid"),
        EXP_STEREO_CHANNEL("_s", " Side"),
        EXP_CHANNEL("_m", " Mid"),
        EXP_CHANNEL("_s", " Side"),
        EXP_AUDIO_METER("_m", " Mid"),
        EXP_AUDIO_METER("_s", " Side"),

        PORTS_END
    };

    static const port_t sc_expander_mono_ports[] =
    {
        PORTS_MONO_PLUGIN,
        PORTS_MONO_SIDECHAIN,
        EXP_COMMON,
        EXP_SC_MONO_CHANNEL,
        EXP_CHANNEL("", ""),
        EXP_AUDIO_METER("", ""),

        PORTS_END
    };

    static const port_t sc_expander_stereo_ports[] =
    {
        PORTS_STEREO_PLUGIN,
        PORTS_STEREO_SIDECHAIN,
        EXP_COMMON,
        EXP_SC_STEREO_CHANNEL("", ""),
        EXP_CHANNEL("", ""),
        EXP_AUDIO_METER("_l", " Left"),
        EXP_AUDIO_METER("_r", " Right"),

        PORTS_END
    };

    static const port_t sc_expander_lr_ports[] =
    {
        PORTS_STEREO_PLUGIN,
        PORTS_STEREO_SIDECHAIN,
        EXP_COMMON,
        EXP_SC_STEREO_CHANNEL("_l", " Left"),
        EXP_SC_STEREO_CHANNEL("_r", " Right"),
        EXP_CHANNEL("_l", " Left"),
        EXP_CHANNEL("_r", " Right"),
        EXP_AUDIO_METER("_l", " Left"),
        EXP_AUDIO_METER("_r", " Right"),

        PORTS_END
    };

    static const port_t sc_expander_ms_ports[] =
    {
        PORTS_STEREO_PLUGIN,
        PORTS_STEREO_SIDECHAIN,
        EXP_MS_COMMON,
        EXP_SC_STEREO_CHANNEL("_m", " Mid"),
        EXP_SC_STEREO_CHANNEL("_s", " Side"),
        EXP_CHANNEL("_m", " Mid"),
        EXP_CHANNEL("_s", " Side"),
        EXP_AUDIO_METER("_m", " Mid"),
        EXP_AUDIO_METER("_s", " Side"),

        PORTS_END
    };

    // Expander
    const plugin_metadata_t  expander_mono_metadata::metadata =
    {
        "Expander Mono",
        "Expander Mono",
        "E1M",
        &developers::v_sadovnikov,
        "expander_mono",
        "tddq",
        LSP_EXPANDER_BASE + 0,
        LSP_VERSION(1, 0, 2),
        expander_classes,
        E_INLINE_DISPLAY,
        expander_mono_ports,
        "dynamics/expander/single/mono.xml",
        NULL,
        mono_plugin_port_groups
    };

    const plugin_metadata_t  expander_stereo_metadata::metadata =
    {
        "Expander Stereo",
        "Expander Stereo",
        "E1S",
        &developers::v_sadovnikov,
        "expander_stereo",
        "au0f",
        LSP_EXPANDER_BASE + 1,
        LSP_VERSION(1, 0, 2),
        expander_classes,
        E_INLINE_DISPLAY,
        expander_stereo_ports,
        "dynamics/expander/single/stereo.xml",
        NULL,
        stereo_plugin_port_groups
    };

    const plugin_metadata_t  expander_lr_metadata::metadata =
    {
        "Expander LeftRight",
        "Expander LeftRight",
        "E1LR",
        &developers::v_sadovnikov,
        "expander_lr",
        "zvsf",
        LSP_EXPANDER_BASE + 2,
        LSP_VERSION(1, 0, 2),
        expander_classes,
        E_INLINE_DISPLAY,
        expander_lr_ports,
        "dynamics/expander/single/lr.xml",
        NULL,
        stereo_plugin_port_groups
    };

    const plugin_metadata_t  expander_ms_metadata::metadata =
    {
        "Expander MidSide",
        "Expander MidSide",
        "E1MS",
        &developers::v_sadovnikov,
        "expander_ms",
        "ebhk",
        LSP_EXPANDER_BASE + 3,
        LSP_VERSION(1, 0, 2),
        expander_classes,
        E_INLINE_DISPLAY,
        expander_ms_ports,
        "dynamics/expander/single/ms.xml",
        NULL,
        stereo_plugin_port_groups
    };

    // Sidechain expander
    const plugin_metadata_t  sc_expander_mono_metadata::metadata =
    {
        "Sidechain-Expander Mono",
        "Sidechain Expander Mono",
        "SCE1M",
        &developers::v_sadovnikov,
        "sc_expander_mono",
        "utul",
        LSP_EXPANDER_BASE + 4,
        LSP_VERSION(1, 0, 2),
        expander_classes,
        E_INLINE_DISPLAY,
        sc_expander_mono_ports,
        "dynamics/expander/single/mono.xml",
        NULL,
        mono_plugin_sidechain_port_groups
    };

    const plugin_metadata_t  sc_expander_stereo_metadata::metadata =
    {
        "Sidechain-Expander Stereo",
        "Sidechain Expander Stereo",
        "SCE1S",
        &developers::v_sadovnikov,
        "sc_expander_stereo",
        "lwqv",
        LSP_EXPANDER_BASE + 5,
        LSP_VERSION(1, 0, 2),
        expander_classes,
        E_INLINE_DISPLAY,
        sc_expander_stereo_ports,
        "dynamics/expander/single/stereo.xml",
        NULL,
        stereo_plugin_sidechain_port_groups
    };

    const plugin_metadata_t  sc_expander_lr_metadata::metadata =
    {
        "Sidechain-Expander LeftRight",
        "Sidechain Expander LeftRight",
        "SCE1LR",
        &developers::v_sadovnikov,
        "sc_expander_lr",
        "pnvw",
        LSP_EXPANDER_BASE + 6,
        LSP_VERSION(1, 0, 2),
        expander_classes,
        E_INLINE_DISPLAY,
        sc_expander_lr_ports,
        "dynamics/expander/single/lr.xml",
        NULL,
        stereo_plugin_sidechain_port_groups
    };

    const plugin_metadata_t  sc_expander_ms_metadata::metadata =
    {
        "Sidechain-Expander MidSide",
        "Sidechain Expander MidSide",
        "SCE1MS",
        &developers::v_sadovnikov,
        "sc_expander_ms",
        "zc0d",
        LSP_EXPANDER_BASE + 7,
        LSP_VERSION(1, 0, 2),
        expander_classes,
        E_INLINE_DISPLAY,
        sc_expander_ms_ports,
        "dynamics/expander/single/ms.xml",
        NULL,
        stereo_plugin_sidechain_port_groups
    };
}



