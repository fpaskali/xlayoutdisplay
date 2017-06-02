#include "Layout.h"
#include "xrandrrutil.h"

#include <string.h>
#include <iostream>
#include <sstream>

using namespace std;

Layout::Layout(const list<DisplP> &displs) : displs(displs) {}

void Layout::orderDispls(const list<string> &order) {

    // stack all the preferred, available displays
    list<DisplP> preferredDispls;
    for (const auto name : order) {
        for (const auto displ : displs) {
            if (strcasecmp(name.c_str(), displ->name.c_str()) == 0) {
                preferredDispls.push_front(displ);
            }
        }
    }

    // move preferred to the front
    for (const auto preferredDispl : preferredDispls) {
        displs.remove(preferredDispl);
        displs.push_front(preferredDispl);
    }
}

void Layout::activateDispls(const string &primary) {
    Laptop *laptop = Laptop::instance();
    for (const auto displ : displs) {

        // don't use any laptop displays if the lid is closed
        if (laptop->shouldDisableDisplay(displ->name))
            continue;

        // only activate currently active or connected displays
        if (displ->state != Displ::active && displ->state != Displ::connected)
            continue;

        // mark active
        displ->desiredActive(true);

        // default first to primary
        if (!Displ::desiredPrimary)
            Displ::desiredPrimary = displ;

        // user selected primary
        if (!primary.empty() && strcasecmp(primary.c_str(), displ->name.c_str()) == 0)
            Displ::desiredPrimary = displ;
    }
}

void Layout::ltrDispls() {
    int xpos = 0;
    int ypos = 0;
    for (const auto displ : displs) {

        if (displ->desiredActive()) {

            // set the desired mode to optimal
            displ->desiredMode(displ->optimalMode);

            // position the screen
            displ->desiredPos = make_shared<Pos>(xpos, ypos);

            // next position
            xpos += displ->desiredMode()->width;
        }
    }
}

void Layout::mirrorDispls() {

    // find the first active display
    DisplP firstDispl;
    for (const auto displ : displs) {
        if (displ->desiredActive()) {
            firstDispl = displ;
            break;
        }
    }
    if (!firstDispl)
        return;

    // iterate through first active display's modes
    for (const auto possibleMode : firstDispl->modes) {
        bool matched = true;

        // attempt to match mode to each active displ
        for (const auto displ : displs) {
            if (!displ->desiredActive())
                continue;

            // reset failed matches
            ModeP desiredMode;

            // match height and width only
            for (const auto mode : displ->modes) {
                if (mode->width == possibleMode->width && mode->height == possibleMode->height) {

                    // set mode and pos
                    desiredMode = mode;
                    break;
                }
            }

            // match a mode for every display; root it at 0, 0
            matched = matched && desiredMode;
            if (matched) {
                displ->desiredMode(desiredMode);
                displ->desiredPos = make_shared<Pos>(0, 0);
                continue;
            }
        }

        // we've set desiredMode and desiredPos (zero) for all displays, all done
        if (matched)
            return;
    }

    // couldn't find a common mode, exit
    throw runtime_error("unable to find common width/height for mirror");
}

string Layout::calculateDpi() {
    stringstream verbose;
    if (!Displ::desiredPrimary) {
        verbose << "DPI defaulting to " << Displ::desiredDpi << "; no primary display has been set set";
    } else if (!Displ::desiredPrimary->edid) {
        verbose << "DPI defaulting to " << Displ::desiredDpi << "; EDID information not available for primary display "
                << Displ::desiredPrimary->name;
    } else if (!Displ::desiredPrimary->desiredMode()) {
        verbose << "DPI defaulting to " << Displ::desiredDpi << "; desiredMode not available for primary display "
                << Displ::desiredPrimary->name;
    } else {
        const unsigned int desiredDpi = Displ::desiredPrimary->edid->closestDpiForMode(
                Displ::desiredPrimary->desiredMode());
        if (desiredDpi == 0) {
            verbose << "DPI defaulting to " << Displ::desiredDpi << "; no display size EDID information available for "
                    << Displ::desiredPrimary->name;
        } else {
            Displ::desiredDpi = desiredDpi;
            verbose << "DPI rounded to " << Displ::desiredDpi << " for primary display " << Displ::desiredPrimary->name
                    << " with DPI: "
                    << Displ::desiredPrimary->edid->dpiForMode(Displ::desiredPrimary->desiredMode());
        }
    }

    return verbose.str();
}
