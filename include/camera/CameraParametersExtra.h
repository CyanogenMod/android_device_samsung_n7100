#define CAMERA_PARAMETERS_EXTRA_C \
const char CameraParameters::KEY_ISO_MODE[] = "iso";\
const char CameraParameters::KEY_SUPPORTED_ISO_MODES[] = "iso-values";

#define CAMERA_PARAMETERS_EXTRA_H \
    static const char KEY_SUPPORTED_ISO_MODES[];\
    static const char KEY_ISO_MODE[];

