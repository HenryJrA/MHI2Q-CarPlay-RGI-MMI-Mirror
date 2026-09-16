# Adaptive Virtual Cockpit geometry

The native mirror retains the four states detected by the Java observer:
`CLASSIC_FULL`, `CLASSIC_SMALL`, `SPORT_FULL`, and `SPORT_SMALL`. A state change
selects the corresponding geometry profile. It does not change Displayable 3,
Java context ownership, planes 98/101/102, or RGI routing.

The BaseVideo window and EGL output remain 1440×455. Every frame uses the full
surface viewport. Layout changes update only the destination quad and source
texture coordinates. Capture remains the complete 1024×480 MMI frame.

## Configuring a profile

Each profile optionally defines a usable destination rectangle in output pixels.
Coordinates have their origin at the top left, with positive x to the right and
positive y downward. All four values must be integers, x/y must be nonnegative,
width/height must be positive, and the entire rectangle must lie inside 1440×455.
The configuration does not clamp an invalid usable rectangle into validity.

The names follow the detected states:

| State | Configuration prefix | CLI prefix |
| --- | --- | --- |
| `CLASSIC_FULL` | `MMI_CLASSIC_FULL_` | `--classic-full-` |
| `CLASSIC_SMALL` | `MMI_CLASSIC_SMALL_` | `--classic-small-` |
| `SPORT_FULL` | `MMI_SPORT_FULL_` | `--sport-full-` |
| `SPORT_SMALL` | `MMI_SPORT_SMALL_` | `--sport-small-` |

For each prefix, the new configuration suffixes are `BOUNDS_X`, `BOUNDS_Y`,
`BOUNDS_WIDTH`, `BOUNDS_HEIGHT`, and `POLICY`. The CLI suffixes are `bounds-x`,
`bounds-y`, `bounds-width`, `bounds-height`, and `policy`. For example,
`MMI_CLASSIC_FULL_BOUNDS_WIDTH` corresponds to `--classic-full-bounds-width`.
Policy values are case-sensitive `CONTAIN` and `COVER`; an omitted policy defaults
to `CONTAIN`.

Measure the usable area for every target vehicle state before enabling bounds.
The developer example and Toolbox configuration deliberately leave them unset;
they do not provide vehicle-specific safe-area dimensions. Configuring one
profile does not opt the other profiles in.

## Scaling and cropping

`CONTAIN` uses the smaller of usable-width/source-width and
usable-height/source-height. The complete source is centered inside the usable
rectangle, preserving its aspect ratio subject to integer destination rounding.
Unused space stays black, and the normalized source UV rectangle remains
`[0,0]` to `[1,1]`.

`COVER` fills the entire usable destination rectangle and preserves aspect ratio
by cropping the source symmetrically through normalized UV coordinates. A wider
destination crops the source vertically; a narrower destination crops it
horizontally. Capture and texture allocation do not change. Cropped source
content is hidden, so verify that the selected area retains needed MMI controls
and information.

Missing, partial, malformed, nonpositive, negative, overflowing, or out-of-surface
bounds, and an invalid policy, fall back to that profile's existing scale/offset
calculation with the full source UV rectangle. With no bounds configured, the
existing installation behaves as before. The legacy `MMI_CONTENT_SCALE`,
`MMI_OFFSET_X`, and `MMI_OFFSET_Y` launcher aliases still supply defaults when
per-profile scale/offset variables are absent. CLI arguments passed to the
launcher come last and retain their existing override precedence.

`--fullscreen` retains its historical meaning: it stretches the full image across
the output and bypasses adaptive layout selection. It is separate from the
observer's `FULL` state.

## Build and rollout

The checked-in Toolbox native executable is unchanged by this source work.
Rebuild and validate the native executable before configuring adaptive bounds;
the older packaged binary cannot parse the new options. The launcher forwards
new adaptive options only when their variables are explicitly set, preserving
compatibility with that packaged binary for the default configuration.

This change does not enable MHI2 installation or alter MHI2Q firmware checks.
Vehicle measurements, a QNX build, and on-vehicle validation are still required
before promoting a replacement binary or selecting final safe areas.
