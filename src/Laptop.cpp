#include "Laptop.h"

#include <string.h>
#include <dirent.h>

Laptop::Laptop() : lidClosed(calculateLidClosed(LAPTOP_LID_ROOT_PATH)) {}

Laptop::Laptop(const bool lidClosed) : lidClosed(lidClosed) {}

const bool Laptop::shouldDisableDisplay(const std::string name) const {
    return lidClosed && strncasecmp(LAPTOP_DISPLAY_PREFIX, name.c_str(), strlen(LAPTOP_DISPLAY_PREFIX)) == 0;
}

const bool calculateLidClosed(const char *laptopLidRootPath) {
    static char lidFileName[PATH_MAX];
    static char line[512];

    // find the lid state directory
    DIR *dir = opendir(laptopLidRootPath);
    if (dir) {
        struct dirent *dirent;
        while ((dirent = readdir(dir)) != NULL) {
            if (dirent->d_type == DT_DIR && strcmp(dirent->d_name, ".") != 0 && strcmp(dirent->d_name, "..") != 0) {

                // read the lid state file
                snprintf(lidFileName, PATH_MAX, "%s/%s/%s", laptopLidRootPath, dirent->d_name, "state");
                FILE *lidFile = fopen(lidFileName, "r");
                if (lidFile != NULL) {
                    if (fgets(line, 512, lidFile))
                        if (strcasestr(line, "closed"))
                            return true;
                    fclose(lidFile);
                }

                // drivers/acpi/button.c acpi_button_add_fs seems to indicate there will be only one file
                break;
            }
        }
        closedir(dir);
    }
    return false;
}

