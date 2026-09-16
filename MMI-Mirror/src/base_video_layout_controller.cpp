#include "base_video_layout_controller.h"

#include <float.h>
#include <stdio.h>

static int offset_inside(int center, int offset, int maximum) {
    if (offset < -center) return 0;
    if (offset > maximum - center) return maximum;
    return center + offset;
}

BaseVideoLayoutController::BaseVideoLayoutController(
        int source_width,
        int source_height,
        int output_width,
        int output_height,
        const BaseVideoLayoutProfiles &profiles)
    : source_width_(source_width),
      source_height_(source_height),
      output_width_(output_width),
      output_height_(output_height),
      profiles_(profiles) {
}

const BaseVideoGeometryProfile &BaseVideoLayoutController::profile_for(
        const ClusterLayoutState &state) const {
    if (state.layout == CLUSTER_LAYOUT_SPORT) {
        return state.view == CLUSTER_VIEW_SMALL
            ? profiles_.sport_small : profiles_.sport_full;
    }
    return state.view == CLUSTER_VIEW_SMALL
        ? profiles_.classic_small : profiles_.classic_full;
}

const char *BaseVideoLayoutController::profile_name(const ClusterLayoutState &state) {
    if (state.layout == CLUSTER_LAYOUT_SPORT)
        return state.view == CLUSTER_VIEW_SMALL ? "SPORT_SMALL" : "SPORT_FULL";
    return state.view == CLUSTER_VIEW_SMALL ? "CLASSIC_SMALL" : "CLASSIC_FULL";
}

bool BaseVideoLayoutController::resolve(const ClusterLayoutState &state,
                                        BaseVideoDestination *destination) const {
    if (!destination || source_width_ <= 0 || source_height_ <= 0 ||
        output_width_ <= 0 || output_height_ <= 0) {
        return false;
    }

    const BaseVideoGeometryProfile &profile = profile_for(state);
    BaseVideoDestination resolved;
    resolved.profile_name = profile_name(state);

    const bool valid_bounds = profile.bounds_x >= 0 && profile.bounds_y >= 0 &&
        profile.bounds_x <= output_width_ && profile.bounds_y <= output_height_ &&
        profile.bounds_width > 0 && profile.bounds_height > 0 &&
        profile.bounds_width <= output_width_ - profile.bounds_x &&
        profile.bounds_height <= output_height_ - profile.bounds_y;
    const bool valid_policy = profile.policy == BASE_VIDEO_CONTAIN ||
        profile.policy == BASE_VIDEO_COVER;

    if (valid_bounds && valid_policy) {
        const double fit_x = (double)profile.bounds_width / source_width_;
        const double fit_y = (double)profile.bounds_height / source_height_;
        resolved.x = profile.bounds_x;
        resolved.y = profile.bounds_y;
        resolved.width = profile.bounds_width;
        resolved.height = profile.bounds_height;
        resolved.uses_bounds = true;
        resolved.policy = profile.policy;

        if (profile.policy == BASE_VIDEO_CONTAIN) {
            const double scale = fit_x < fit_y ? fit_x : fit_y;
            resolved.width = (int)(source_width_ * scale + 0.5);
            resolved.height = (int)(source_height_ * scale + 0.5);
            if (resolved.width < 1) resolved.width = 1;
            if (resolved.height < 1) resolved.height = 1;
            if (resolved.width > profile.bounds_width) resolved.width = profile.bounds_width;
            if (resolved.height > profile.bounds_height) resolved.height = profile.bounds_height;
            resolved.x += (profile.bounds_width - resolved.width) / 2;
            resolved.y += (profile.bounds_height - resolved.height) / 2;
            resolved.applied_scale = (float)scale;
        } else {
            /* Fill the destination without stretching. Keep the quad inside
             * the usable bounds and crop the source via centered UVs. */
            const double scale = fit_x > fit_y ? fit_x : fit_y;
            const double u_span = fit_x / scale;
            const double v_span = fit_y / scale;
            resolved.u0 = (float)((1.0 - u_span) * 0.5);
            resolved.v0 = (float)((1.0 - v_span) * 0.5);
            resolved.u1 = 1.0f - resolved.u0;
            resolved.v1 = 1.0f - resolved.v0;
            resolved.applied_scale = (float)scale;
        }
        if (!resolved.valid_for(output_width_, output_height_)) return false;
        *destination = resolved;
        return true;
    }

    if (profile.bounds_x != -1 || profile.bounds_y != -1 ||
        profile.bounds_width != -1 || profile.bounds_height != -1 || !valid_policy) {
        fprintf(stderr, "layout: %s incomplete/invalid bounds or policy; using legacy scale/offset\n",
                resolved.profile_name);
    }
    if (!(profile.scale > 0.0f && profile.scale <= FLT_MAX)) return false;

    float scale = profile.scale;
    const float fit_x = (float)output_width_ / (float)source_width_;
    const float fit_y = (float)output_height_ / (float)source_height_;
    const float max_scale = fit_x < fit_y ? fit_x : fit_y;
    if (scale > max_scale) scale = max_scale;

    int width = (int)((float)source_width_ * scale + 0.5f);
    int height = (int)((float)source_height_ * scale + 0.5f);
    if (width < 1) width = 1;
    if (height < 1) height = 1;

    if (width > output_width_) width = output_width_;
    if (height > output_height_) height = output_height_;
    resolved.x = offset_inside((output_width_ - width) / 2,
                               profile.offset_x, output_width_ - width);
    resolved.y = offset_inside((output_height_ - height) / 2,
                               profile.offset_y, output_height_ - height);
    resolved.width = width;
    resolved.height = height;
    resolved.applied_scale = scale;
    resolved.offset_x = profile.offset_x;
    resolved.offset_y = profile.offset_y;
    *destination = resolved;
    return true;
}
