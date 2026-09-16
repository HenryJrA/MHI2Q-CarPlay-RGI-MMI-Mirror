#ifndef BASE_VIDEO_LAYOUT_CONTROLLER_H
#define BASE_VIDEO_LAYOUT_CONTROLLER_H

#include "cluster_layout_state.h"
#include "video_geometry.h"

enum BaseVideoScalingPolicy {
    BASE_VIDEO_CONTAIN = 0,
    BASE_VIDEO_COVER,
    BASE_VIDEO_POLICY_INVALID
};

struct BaseVideoGeometryProfile {
    float scale;
    int offset_x;
    int offset_y;

    /* All four bounds must be configured; -1 means unspecified. Invalid or
     * incomplete bounds/policy use the legacy scale/offset fields above. */
    int bounds_x;
    int bounds_y;
    int bounds_width;
    int bounds_height;
    BaseVideoScalingPolicy policy;

    BaseVideoGeometryProfile()
        : scale(0.80f), offset_x(0), offset_y(0),
          bounds_x(-1), bounds_y(-1), bounds_width(-1), bounds_height(-1),
          policy(BASE_VIDEO_CONTAIN) {}
};

struct BaseVideoLayoutProfiles {
    BaseVideoGeometryProfile classic_full;
    BaseVideoGeometryProfile classic_small;
    BaseVideoGeometryProfile sport_full;
    BaseVideoGeometryProfile sport_small;
};

struct BaseVideoDestination : public VideoGeometry {
    float applied_scale;
    int offset_x;
    int offset_y;
    const char *profile_name;
    bool uses_bounds;
    BaseVideoScalingPolicy policy;

    BaseVideoDestination()
        : applied_scale(0.0f), offset_x(0), offset_y(0), profile_name("UNKNOWN"),
          uses_bounds(false), policy(BASE_VIDEO_CONTAIN) {}
};

class BaseVideoLayoutController {
public:
    BaseVideoLayoutController(int source_width,
                              int source_height,
                              int output_width,
                              int output_height,
                              const BaseVideoLayoutProfiles &profiles);

    bool resolve(const ClusterLayoutState &state,
                 BaseVideoDestination *destination) const;

    static const char *profile_name(const ClusterLayoutState &state);

private:
    const BaseVideoGeometryProfile &profile_for(const ClusterLayoutState &state) const;

    int source_width_;
    int source_height_;
    int output_width_;
    int output_height_;
    BaseVideoLayoutProfiles profiles_;
};

#endif
