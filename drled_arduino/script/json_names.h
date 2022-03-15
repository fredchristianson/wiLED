#ifndef SCRIPT_JSON_NAMES_H
#define SCRIPT_JSON_NAMES_H

namespace DevRelief {
const char * S_SCRIPT_NAME="name";
const char * S_FREQUENCY="frequency";
const char * S_ELEMENTS="elements";
const char * S_TYPE = "type";
const char * S_RGB = "rgb";
const char * S_HSL = "hsl";  // spectrum - hsl
const char * S_RHSL = "rhsl"; // rainbow - hsl
const char * S_RED = "red";
const char * S_GREEN = "green";
const char * S_BLUE = "blue";
const char * S_HUE = "hue";
const char * S_SATURATION = "saturation";
const char * S_LIGHTNESS = "lightness";
const char * S_POSITION = "position";
const char * S_LAYOUT = "layout";
const char * S_VALUES = "values";
const char * S_START = "start";
const char * S_END = "end";
const char * S_COUNT = "count";
const char * S_WRAP = "wrap";
const char * S_DURATION = "duration";
const char * S_SPEED = "speed";
const char * S_BASE = "base";
const char * S_UNIT = "unit";
const char * S_PIXEL = "pixel";
const char * S_PERCENT = "percent";
const char * S_SEC = "sec";
const char * S_VAR = "var";
const char * S_STEP = "step";
const char * S_REPEAT = "repeat";
const char * S_EASE = "ease";
const char * S_IN = "in";
const char * S_OUT = "out";
const char * S_STATE = "state";
const char * S_RUNNING = "running";
const char * S_COMPLETE = "complete";
const char * S_NEXT = "next";
const char * S_ANIMATION = "animation";
const char * S_FOLD = "fold";
const char * S_UNFOLD = "unfold";
const char * S_RAND = "rand";
const char * S_ADD = "add";
const char * S_SUB = "sub";
const char * S_CONTROL = "control";
const char * S_VARIABLE = "variable";
const char * S_UPDATE = "update";
const char * S_PATTERN = "pattern";
const char * S_SEGMENT = "segment";
const char * S_MAKER = "maker";
const char * S_CREATE = "create";
const char * S_STRIP = "strip";
const char * S_MIRROR = "mirror";
const char * S_COPY = "copy";

const char * S_ROOT_CONTAINER = "root";

bool matchName(const char * want, const char * have) {
    bool i= strcasecmp(want,have) == 0;
    return i;
};

}
#endif