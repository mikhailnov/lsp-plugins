/*
 * raytrace.cpp
 *
 *  Created on: 21 февр. 2019 г.
 *      Author: sadko
 */


#include <test/mtest.h>
#include <core/3d/rt_context.h>
#include <core/3d/RayTrace3D.h>
#include <core/files/Model3DFile.h>
#include <core/sampling/Sample.h>
#include <core/files/AudioFile.h>

using namespace lsp;

//-----------------------------------------------------------------------------
// Performance test for equalizer module
MTEST_BEGIN("core.3d", raytrace)

    MTEST_MAIN
    {
        // Load scene
        Scene3D scene;
        MTEST_ASSERT(Model3DFile::load(&scene, "res/test/3d/empty-room-4x4x3.obj", true) == STATUS_OK);

        // Initialize sample
        Sample sample;
        MTEST_ASSERT(sample.init(4, 512, 0));

        // Prepare raytracing
        RayTrace3D trace;
        MTEST_ASSERT(trace.init() == STATUS_OK);
        trace.set_sample_rate(48000);
        trace.set_energy_threshold(1e-6f);
        trace.set_tolerance(1e-5f);
        MTEST_ASSERT(trace.set_scene(&scene, true) == STATUS_OK);

        // Add source
        ray3d_t source;
        dsp::init_point_xyz(&source.z, -1.0f, 0.0f, 0.0f);
        dsp::init_vector_dxyz(&source.v, 0.0f, 0.0f, 0.3048f); // 12" speaker source
        MTEST_ASSERT(trace.add_source(&source, RT_AS_OMNI, 1.0f) == STATUS_OK);

        // Add capture
        ray3d_t capture;
        dsp::init_point_xyz(&capture.z, 1.0f, 0.0f, 0.0f);
        dsp::init_vector_dxyz(&capture.v, 0.0f, 0.0f, 0.0508f); // 2" microphone diaphragm
        MTEST_ASSERT(trace.add_capture(&capture, RT_AC_OMNIDIRECTIONAL, 1.0f) == 0);
        trace.bind_capture(0, &sample, 0, 0, 0);    // Direct reflections
        trace.bind_capture(0, &sample, 1, 1, 3);    // Early reflections
        trace.bind_capture(0, &sample, 2, 4, -1);   // Late reflections
        trace.bind_capture(0, &sample, 3, -1, -1);  // All reflections

        // Perform raytracing
        MTEST_ASSERT(trace.process(1.0f) == STATUS_OK);

        // Save sample to file
        AudioFile af;
        LSPString path;

        MTEST_ASSERT(path.fmt_utf8("tmp/utest-%s-4track.wav", this->full_name()));
        MTEST_ASSERT(af.create(&sample, trace.get_sample_rate()) == STATUS_OK);
        MTEST_ASSERT(af.store(&path) == STATUS_OK);

        // Destroy objects
        trace.destroy(false);
        scene.destroy();
    }
MTEST_END
