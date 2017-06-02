#ifndef XLAYOUTDISPLAYS_STDUTIL_H
#define XLAYOUTDISPLAYS_STDUTIL_H

#include <memory>
#include <limits.h>

// sorting function for shared pointers... this must be in STL somewhere...
template<typename T>
inline bool sortSharedPtr(const std::shared_ptr<T> &l, const std::shared_ptr<T> &r) {
    return (*l) < (*r);
}

// copy list of shared_ptr, sort it, return it as const
template<typename T>
inline const std::list<std::shared_ptr<T>> sortSharedPtrList(const std::list<std::shared_ptr<T>> &list) {
    std::list<std::shared_ptr<T>> sorted = list;
    sorted.sort(sortSharedPtr<T>);
    return sorted;
}

// copy list of shared_ptr, reverse it, return it as const
template<typename T>
inline const std::list<std::shared_ptr<T>> reverseSharedPtrList(const std::list<std::shared_ptr<T>> &list) {
    std::list<std::shared_ptr<T>> reversed = list;
    reversed.reverse();
    return reversed;
}

// todo: test, you know...
// todo: use a better string constructor... bikeshedding
// resolve a UNIX path prefixed with ~ to an absolute path, using the HOME environment variable
inline const std::string resolveTildePath(const char *path) {
    char settingsFilePath[PATH_MAX];
    snprintf(settingsFilePath, PATH_MAX, "%s/%s", getenv("HOME"), path);
    return std::string(settingsFilePath);
}

#endif //XLAYOUTDISPLAYS_STDUTIL_H
