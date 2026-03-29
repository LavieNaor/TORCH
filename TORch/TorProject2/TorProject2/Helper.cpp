#include "Helper.h"

unsigned int Helper::stoui(const std::string& s)
{
    // transform to unsigned long
    unsigned long lresult = stoul(s, 0, 10);

    // transform to unsigned int
    unsigned int result = lresult;
    if (result != lresult) throw std::runtime_error("Out of range!");
    return result;
}

std::time_t Helper::fromIso8601UTC(const std::string& iso)
{
    std::tm tm{};
    std::istringstream ss(iso);

    // write the time in UTC format
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");

    if (ss.fail())
    {
        return -1;
    }

#if defined(_WIN32)
    return _mkgmtime(&tm);   // Windows
#else
    return timegm(&tm);      // Linux
#endif
}

std::string Helper::toIso8601UTC(std::time_t t)
{
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif

    // write the time in UTC format
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

