#include "base_video_layout_controller.h"
#include "cluster_layout_state.h"
#include "fake_gl.h"
#include "gl_renderer.h"
#include "v2_options.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

static int failures = 0;
static int checks = 0;

static void check(bool passed, const char *expression, int line) {
    ++checks;
    if (!passed) {
        fprintf(stderr, "FAIL line %d: %s\n", line, expression);
        ++failures;
    }
}
#define CHECK(expression) check(!!(expression), #expression, __LINE__)

static bool near(float a, float b, float tolerance = 0.00001f) {
    return fabs(a - b) <= tolerance;
}

static const ClusterLayoutState states[] = {
    ClusterLayoutState(CLUSTER_LAYOUT_CLASSIC, CLUSTER_VIEW_FULL),
    ClusterLayoutState(CLUSTER_LAYOUT_CLASSIC, CLUSTER_VIEW_SMALL),
    ClusterLayoutState(CLUSTER_LAYOUT_SPORT, CLUSTER_VIEW_FULL),
    ClusterLayoutState(CLUSTER_LAYOUT_SPORT, CLUSTER_VIEW_SMALL)
};
static const char *const state_names[] = {
    "CLASSIC_FULL", "CLASSIC_SMALL", "SPORT_FULL", "SPORT_SMALL"
};
static const char *const profile_flags[] = {
    "--classic-full", "--classic-small", "--sport-full", "--sport-small"
};

static BaseVideoGeometryProfile &profile_at(BaseVideoLayoutProfiles &profiles, int index) {
    switch (index) {
        case 0: return profiles.classic_full;
        case 1: return profiles.classic_small;
        case 2: return profiles.sport_full;
        default: return profiles.sport_small;
    }
}

static void set_bounds(BaseVideoGeometryProfile &p, int x, int y, int width, int height,
                       BaseVideoScalingPolicy policy) {
    p.bounds_x = x; p.bounds_y = y; p.bounds_width = width; p.bounds_height = height;
    p.policy = policy;
}

static bool resolve_profile(const BaseVideoGeometryProfile &profile, BaseVideoDestination *out,
                            int source_width = 1024, int source_height = 480) {
    BaseVideoLayoutProfiles profiles;
    profiles.classic_full = profile;
    BaseVideoLayoutController controller(source_width, source_height, 1440, 455, profiles);
    return controller.resolve(states[0], out);
}

static void check_rect(const VideoGeometry &d, int x, int y, int width, int height) {
    CHECK(d.x == x); CHECK(d.y == y);
    CHECK(d.width == width); CHECK(d.height == height);
}

static void check_uv(const VideoGeometry &d, float u0, float v0, float u1, float v1) {
    CHECK(near(d.u0, u0)); CHECK(near(d.v0, v0));
    CHECK(near(d.u1, u1)); CHECK(near(d.v1, v1));
}

static void check_aspect(const VideoGeometry &d, int source_width, int source_height) {
    const float source_aspect = source_width * (d.u1 - d.u0) /
                                (source_height * (d.v1 - d.v0));
    const float destination_aspect = static_cast<float>(d.width) / d.height;
    /* Integer destination dimensions can differ by up to a pixel after rounding. */
    CHECK(fabs(source_aspect - destination_aspect) <= 2.0f / d.height);
}

static void test_legacy_and_defaults() {
    BaseVideoGeometryProfile p;
    CHECK(near(p.scale, 0.80f)); CHECK(p.offset_x == 0); CHECK(p.offset_y == 0);
    CHECK(p.bounds_x == -1); CHECK(p.bounds_y == -1);
    CHECK(p.bounds_width == -1); CHECK(p.bounds_height == -1);
    CHECK(p.policy == BASE_VIDEO_CONTAIN);
    VideoGeometry initial;
    CHECK(initial.width == 0); CHECK(initial.height == 0);
    check_uv(initial, 0, 0, 1, 1);

    BaseVideoDestination d;
    CHECK(resolve_profile(p, &d));
    check_rect(d, 310, 35, 819, 384);
    check_uv(d, 0, 0, 1, 1); CHECK(!d.uses_bounds);

    p.scale = 0.63f; p.offset_y = 8;
    CHECK(resolve_profile(p, &d));
    check_rect(d, 397, 84, 645, 302);
    check_uv(d, 0, 0, 1, 1);

    p.scale = 4; p.offset_x = INT_MAX; p.offset_y = INT_MAX;
    CHECK(resolve_profile(p, &d));
    check_rect(d, 469, 0, 971, 455);
    p.offset_x = INT_MIN; p.offset_y = INT_MIN;
    CHECK(resolve_profile(p, &d));
    check_rect(d, 0, 0, 971, 455);
}

static void test_contain_and_cover() {
    BaseVideoGeometryProfile p;
    BaseVideoDestination d;
    set_bounds(p, 100, 20, 800, 200, BASE_VIDEO_CONTAIN);
    CHECK(resolve_profile(p, &d, 800, 400));
    check_rect(d, 300, 20, 400, 200);
    check_uv(d, 0, 0, 1, 1); CHECK(d.uses_bounds); CHECK(d.policy == BASE_VIDEO_CONTAIN);
    check_aspect(d, 800, 400);

    set_bounds(p, 100, 30, 200, 300, BASE_VIDEO_CONTAIN);
    CHECK(resolve_profile(p, &d, 800, 400));
    check_rect(d, 100, 130, 200, 100); check_uv(d, 0, 0, 1, 1);
    check_aspect(d, 800, 400);

    set_bounds(p, 70, 40, 600, 300, BASE_VIDEO_CONTAIN);
    CHECK(resolve_profile(p, &d, 800, 400));
    check_rect(d, 70, 40, 600, 300); check_uv(d, 0, 0, 1, 1);
    check_aspect(d, 800, 400);

    set_bounds(p, 90, 20, 1024, 240, BASE_VIDEO_COVER);
    CHECK(resolve_profile(p, &d));
    check_rect(d, 90, 20, 1024, 240); check_uv(d, 0, .25f, 1, .75f);
    CHECK(d.uses_bounds); CHECK(d.policy == BASE_VIDEO_COVER); check_aspect(d, 1024, 480);

    set_bounds(p, 330, 100, 256, 240, BASE_VIDEO_COVER);
    CHECK(resolve_profile(p, &d));
    check_rect(d, 330, 100, 256, 240); check_uv(d, .25f, 0, .75f, 1);
    check_aspect(d, 1024, 480);

    set_bounds(p, 10, 10, 512, 240, BASE_VIDEO_COVER);
    CHECK(resolve_profile(p, &d));
    check_rect(d, 10, 10, 512, 240); check_uv(d, 0, 0, 1, 1);
    check_aspect(d, 1024, 480);

    /* Explicit bounds define the placement; legacy offsets cannot push it outside. */
    p.scale = .1f; p.offset_x = 900; p.offset_y = -900;
    CHECK(resolve_profile(p, &d));
    check_rect(d, 10, 10, 512, 240);
}

static void check_legacy_fallback(const BaseVideoGeometryProfile &p) {
    BaseVideoDestination d;
    /* Poison the reused destination with crop data to catch incomplete resets. */
    d.u0 = .2f; d.v0 = .3f; d.u1 = .6f; d.v1 = .7f;
    d.uses_bounds = true;
    CHECK(resolve_profile(p, &d));
    CHECK(!d.uses_bounds); check_rect(d, 397, 84, 645, 302); check_uv(d, 0, 0, 1, 1);
}

static void test_invalid_bounds_fallback() {
    BaseVideoGeometryProfile base;
    base.scale = .63f; base.offset_y = 8;
    check_legacy_fallback(base);
    set_bounds(base, 100, 20, 600, 300, BASE_VIDEO_CONTAIN);
    for (int field = 0; field < 4; ++field) {
        BaseVideoGeometryProfile p = base;
        int *fields[] = { &p.bounds_x, &p.bounds_y, &p.bounds_width, &p.bounds_height };
        *fields[field] = -1;
        check_legacy_fallback(p);
        *fields[field] = INT_MAX;
        check_legacy_fallback(p);
    }
    BaseVideoGeometryProfile p = base;
    p.bounds_width = 0; check_legacy_fallback(p);
    p = base; p.bounds_height = 0; check_legacy_fallback(p);
    p = base; p.bounds_x = -4; check_legacy_fallback(p);
    p = base; p.bounds_y = -4; check_legacy_fallback(p);
    p = base; p.bounds_x = 1000; check_legacy_fallback(p);
    p = base; p.bounds_y = 300; check_legacy_fallback(p);
    p = base; p.bounds_x = INT_MAX - 10; p.bounds_width = 50; check_legacy_fallback(p);
    p = base; p.policy = BASE_VIDEO_POLICY_INVALID; check_legacy_fallback(p);

    BaseVideoLayoutProfiles profiles;
    BaseVideoDestination d;
    BaseVideoLayoutController no_source(0, 480, 1440, 455, profiles);
    CHECK(!no_source.resolve(states[0], &d));
    BaseVideoLayoutController no_output(1024, 480, 1440, 0, profiles);
    CHECK(!no_output.resolve(states[0], &d));
    CHECK(!no_output.resolve(states[0], 0));
}

static bool parse(const std::string &arguments, Options *options) {
    std::istringstream input(arguments);
    std::vector<std::string> words;
    words.push_back("geometry-test");
    std::string word;
    while (input >> word) words.push_back(word);
    std::vector<char *> argv;
    for (size_t i = 0; i < words.size(); ++i)
        argv.push_back(const_cast<char *>(words[i].c_str()));
    return v2_parse_options(static_cast<int>(argv.size()), &argv[0], options);
}

static std::string bounds_options(const char *prefix, const char *policy) {
    const std::string p(prefix);
    return p + "-bounds-x 100 " + p + "-bounds-y 20 " + p + "-bounds-width 600 " +
           p + "-bounds-height 300 " + p + "-policy " + policy;
}

static void test_options_and_legacy_aliases() {
    Options options;
    for (int index = 0; index < 4; ++index) {
        CHECK(parse("--mmi " + bounds_options(profile_flags[index], "COVER"), &options));
        BaseVideoGeometryProfile &p = profile_at(options.profiles, index);
        CHECK(p.bounds_x == 100); CHECK(p.bounds_y == 20);
        CHECK(p.bounds_width == 600); CHECK(p.bounds_height == 300);
        CHECK(p.policy == BASE_VIDEO_COVER);
        for (int other = 0; other < 4; ++other)
            if (index != other) CHECK(profile_at(options.profiles, other).bounds_x == -1);
    }
    CHECK(parse("--mmi --content-scale .63 --offset-x 0 --offset-y 8", &options));
    for (int i = 0; i < 4; ++i) check_legacy_fallback(profile_at(options.profiles, i));
    CHECK(parse("--mmi --content-scale .63 --classic-small-scale .5 --offset-y 8", &options));
    CHECK(near(options.profiles.classic_full.scale, .63f));
    CHECK(near(options.profiles.classic_small.scale, .5f));
    CHECK(options.profiles.sport_small.offset_y == 8);
    CHECK(parse("--mmi --classic-small-scale .5 --content-scale .63", &options));
    CHECK(near(options.profiles.classic_small.scale, .63f));

    const char *const malformed[] = { "garbage", "12px", "1.5", "2147483648", "-2147483649", "nan" };
    for (size_t i = 0; i < sizeof(malformed) / sizeof(malformed[0]); ++i) {
        const std::string args = "--mmi --content-scale .63 --offset-y 8 " +
            bounds_options("--classic-full", "CONTAIN") +
            " --classic-full-bounds-width " + malformed[i] + " --fps 23";
        CHECK(parse(args, &options));
        CHECK(options.fps == 23);
        check_legacy_fallback(options.profiles.classic_full);
    }
    CHECK(parse("--mmi --content-scale .63 --offset-y 8 " +
                bounds_options("--classic-full", "CONTAIN") +
                " --classic-full-bounds-width --fps 24", &options));
    CHECK(options.fps == 24); check_legacy_fallback(options.profiles.classic_full);
    CHECK(parse("--mmi --content-scale .63 --offset-y 8 " +
                bounds_options("--classic-full", "CONTAIN") +
                " --classic-full-policy --fps 25", &options));
    CHECK(options.fps == 25); check_legacy_fallback(options.profiles.classic_full);
    CHECK(parse("--mmi --content-scale .63 --offset-y 8 " +
                bounds_options("--classic-full", "STRETCH"), &options));
    check_legacy_fallback(options.profiles.classic_full);

    /* Reusing Options must not retain a previous parse's adaptive bounds. */
    CHECK(parse("--mmi " + bounds_options("--sport-full", "COVER"), &options));
    CHECK(parse("--mmi", &options));
    for (int i = 0; i < 4; ++i) {
        BaseVideoGeometryProfile &p = profile_at(options.profiles, i);
        CHECK(p.bounds_x == -1); CHECK(p.bounds_y == -1);
        CHECK(p.bounds_width == -1); CHECK(p.bounds_height == -1);
        CHECK(p.policy == BASE_VIDEO_CONTAIN);
    }
}

static void check_rendered_geometry(const VideoGeometry &d) {
    const float left = -1 + 2.0f * d.x / 1440;
    const float right = -1 + 2.0f * (d.x + d.width) / 1440;
    const float top = 1 - 2.0f * d.y / 455;
    const float bottom = 1 - 2.0f * (d.y + d.height) / 455;
    const float expected_vertices[] = { left, top, left, bottom, right, top, right, bottom };
    const float expected_uv[] = { d.u0, d.v0, d.u0, d.v1, d.u1, d.v0, d.u1, d.v1 };
    for (int i = 0; i < 8; ++i) {
        CHECK(near(fake_gl.vertices[i], expected_vertices[i]));
        CHECK(near(fake_gl.texcoords[i], expected_uv[i]));
    }
    CHECK(fake_gl.viewport[0] == 0); CHECK(fake_gl.viewport[1] == 0);
    CHECK(fake_gl.viewport[2] == 1440); CHECK(fake_gl.viewport[3] == 455);
    CHECK(fake_gl.primitive == GL_TRIANGLE_STRIP); CHECK(fake_gl.vertex_count == 4);
    CHECK(fake_gl.first_vertex == 0); CHECK(fake_gl.clear_mask == GL_COLOR_BUFFER_BIT);
}

static void test_all_state_transitions() {
    BaseVideoLayoutProfiles profiles;
    set_bounds(profiles.classic_full, 10, 20, 1024, 240, BASE_VIDEO_COVER);
    set_bounds(profiles.classic_small, 80, 70, 400, 300, BASE_VIDEO_CONTAIN);
    set_bounds(profiles.sport_full, 400, 50, 256, 240, BASE_VIDEO_COVER);
    profiles.sport_small.scale = .63f;
    profiles.sport_small.offset_y = 8; // Missing bounds deliberately selects legacy.
    BaseVideoLayoutController controller(1024, 480, 1440, 455, profiles);
    fake_gl_reset();
    GlRenderer renderer;
    CHECK(renderer.init(1440, 455));
    const unsigned char pixel[] = { 30, 20, 10, 255 };
    CHECK(renderer.upload_rgba(pixel, 1, 1));
    for (int from = 0; from < 4; ++from) {
        for (int to = 0; to < 4; ++to) {
            BaseVideoDestination previous, next;
            CHECK(controller.resolve(states[from], &previous));
            CHECK(renderer.set_geometry(previous)); renderer.draw();
            check_rendered_geometry(previous);
            CHECK(controller.resolve(states[to], &next));
            CHECK(strcmp(next.profile_name, state_names[to]) == 0);
            CHECK(renderer.set_geometry(next)); renderer.draw();
            check_rendered_geometry(next); check_aspect(next, 1024, 480);
            if (to == 1 || to == 3) check_uv(next, 0, 0, 1, 1);
        }
    }
    CHECK(fake_gl.draws == 32);
    CHECK(fake_gl.programs_created == 1); CHECK(fake_gl.textures_created == 1);
    CHECK(fake_gl.image_uploads == 1); CHECK(fake_gl.subimage_uploads == 0);
    CHECK(fake_gl.clears == 32);
    CHECK(near(fake_gl.clear_color[0], 0)); CHECK(near(fake_gl.clear_color[3], 1));
}

static void test_renderer_rejection_and_resets() {
    fake_gl_reset();
    GlRenderer renderer;
    CHECK(renderer.init(1440, 455));
    VideoGeometry valid;
    valid.x = 200; valid.y = 40; valid.width = 500; valid.height = 300;
    valid.u0 = .2f; valid.v0 = .1f; valid.u1 = .8f; valid.v1 = .9f;
    CHECK(renderer.set_geometry(valid)); renderer.draw(); check_rendered_geometry(valid);
    for (int invalid_case = 0; invalid_case < 12; ++invalid_case) {
        VideoGeometry invalid = valid;
        switch (invalid_case) {
            case 0: invalid.x = -1; break;
            case 1: invalid.y = -1; break;
            case 2: invalid.width = 0; break;
            case 3: invalid.height = -1; break;
            case 4: invalid.x = INT_MAX; invalid.width = INT_MAX; break;
            case 5: invalid.y = INT_MAX; invalid.height = INT_MAX; break;
            case 6: invalid.u0 = -.1f; break;
            case 7: invalid.v1 = 1.1f; break;
            case 8: invalid.u0 = invalid.u1; break;
            case 9: invalid.v0 = invalid.v1 + .1f; break;
            case 10: invalid.u0 = std::numeric_limits<float>::quiet_NaN(); break;
            case 11: invalid.v1 = std::numeric_limits<float>::infinity(); break;
        }
        CHECK(!renderer.set_geometry(invalid));
        renderer.draw(); check_rendered_geometry(valid);
    }
    CHECK(renderer.set_destination_rect(30, 40, 600, 300));
    renderer.draw();
    VideoGeometry uncropped;
    uncropped.x = 30; uncropped.y = 40; uncropped.width = 600; uncropped.height = 300;
    check_rendered_geometry(uncropped);
    CHECK(renderer.set_geometry(valid));
    renderer.set_fullscreen_destination(); renderer.draw();
    VideoGeometry fullscreen;
    fullscreen.width = 1440; fullscreen.height = 455;
    check_rendered_geometry(fullscreen);
}

static void write_state(const char *path, const char *text) {
    FILE *fp = fopen(path, "w");
    CHECK(fp != 0);
    if (!fp) return;
    CHECK(fputs(text, fp) >= 0); CHECK(fclose(fp) == 0);
}

static void test_state_reader() {
    char path[] = "/tmp/mmi-geometry-state-XXXXXX";
    const int fd = mkstemp(path);
    CHECK(fd >= 0);
    if (fd < 0) return;
    close(fd);
    ClusterLayoutStateReader reader(path);
    ClusterLayoutState state = states[3];
    write_state(path, "layout=CLASSIC\nview=FULL\nlayout_name=diagnostic\n");
    CHECK(reader.read(&state)); CHECK(state == states[0]);
    write_state(path, " skin = sport\nview_mode=smallscreen\n");
    CHECK(reader.read(&state)); CHECK(state == states[3]);
    write_state(path, "layout=CLASSIC\n");
    CHECK(!reader.read(&state)); CHECK(state == states[3]);
    write_state(path, "layout=SPORT\nview=unknown\n");
    CHECK(!reader.read(&state)); CHECK(state == states[3]);
    CHECK(unlink(path) == 0);
    CHECK(!reader.read(&state)); CHECK(state == states[3]);
}

int main() {
    test_legacy_and_defaults();
    test_contain_and_cover();
    test_invalid_bounds_fallback();
    test_options_and_legacy_aliases();
    test_all_state_transitions();
    test_renderer_rejection_and_resets();
    test_state_reader();
    if (failures) {
        fprintf(stderr, "%d of %d checks failed\n", failures, checks);
        return 1;
    }
    printf("Adaptive geometry: %d checks passed (16 state transitions, real renderer with fake GL).\n", checks);
    return 0;
}
