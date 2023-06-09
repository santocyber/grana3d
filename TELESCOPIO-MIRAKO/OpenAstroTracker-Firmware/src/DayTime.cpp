#include "inc/Globals.hpp"
#include "../Configuration.hpp"
#include "Utility.hpp"
#include "DayTime.hpp"

///////////////////////////////////
// DayTime (and Declination below)
//
// A class to handle hours, minutes, seconds in a unified manner, allowing
// addition of hours, minutes, seconds, other times and conversion to string.

// Parses the RA or DEC from a string that has an optional sign, a two digit degree, a seperator, a two digit minute, a seperator and a two digit second.
// Does not correct for hemisphere (derived class Declination takes care of that)
// For example:   -45*32:11 or 23:44:22
DayTime DayTime::ParseFromMeade(String const &s)
{
    DayTime result;
    int i    = 0;
    long sgn = 1;
    LOG(DEBUG_MEADE, "[DAYTIME]: Parse Coord from [%s]", s.c_str());
    // Check whether we have a sign. This should be able to parse RA and DEC strings (RA never has a sign, and DEC should always have one).
    if ((s[i] == '-') || (s[i] == '+'))
    {
        sgn = s[i] == '-' ? -1 : +1;
        i++;
    }

    // Degs can be 2 or 3 digits
    long degs = s[i++] - '0';
    LOG(DEBUG_MEADE, "[DAYTIME]: 1st digit [%c] -> degs=%l", s[i - 1], degs);
    degs = degs * 10 + s[i++] - '0';
    LOG(DEBUG_MEADE, "[DAYTIME]: 2nd digit [%c] -> degs=%l", s[i - 1], degs);

    // Third digit?
    if ((s[i] >= '0') && (s[i] <= '9'))
    {
        degs = degs * 10 + s[i++] - '0';
        LOG(DEBUG_MEADE, "[DAYTIME]: 3rd digit [%c] -> degs=%d", s[i - 1], degs);
    }
    i++;  // Skip seperator

    int mins = s.substring(i, i + 2).toInt();
    LOG(DEBUG_MEADE, "[DAYTIME]: Minutes are [%s] -> mins=%d", s.substring(i, i + 2).c_str(), mins);
    int secs = 0;
    if (int(s.length()) > i + 4)
    {
        secs = s.substring(i + 3, i + 5).toInt();
        LOG(DEBUG_MEADE, "[DAYTIME]: Seconds are [%s] -> secs=%d", s.substring(i + 3, i + 5).c_str(), secs);
    }
    else
    {
        LOG(DEBUG_MEADE, "[DAYTIME]: No Seconds. slen %d is not > %d", s.length(), i + 4);
    }
    // Get the signed total seconds specified....
    result.totalSeconds = sgn * (((degs * 60L + mins) * 60L) + secs);

    LOG(DEBUG_MEADE, "[DAYTIME]: TotalSeconds are %l from %lh %dm %ds", result.totalSeconds, degs, mins, secs);

    return result;
}

DayTime::DayTime()
{
    totalSeconds = 0;
}

DayTime::DayTime(const DayTime &other)
{
    totalSeconds = other.totalSeconds;
}

DayTime::DayTime(int h, int m, int s)
{
    long sgn     = sign(h);
    h            = abs(h);
    totalSeconds = sgn * ((60L * h + m) * 60L + s);
}

DayTime::DayTime(float timeInHours)
{
    long sgn     = fsign(timeInHours);
    timeInHours  = fabsf(timeInHours);
    totalSeconds = sgn * static_cast<long>(roundf(timeInHours * 60.0f * 60.0f));
}

int DayTime::getHours() const
{
    int h, m, s;
    getTime(h, m, s);
    return h;
}

int DayTime::getMinutes() const
{
    int h, m, s;
    getTime(h, m, s);
    return m;
}

int DayTime::getSeconds() const
{
    int h, m, s;
    getTime(h, m, s);
    return s;
}

float DayTime::getTotalHours() const
{
    return 1.0f * totalSeconds / 3600.0f;
}

float DayTime::getTotalMinutes() const
{
    return 1.0f * totalSeconds / 60.0f;
}

long DayTime::getTotalSeconds() const
{
    return totalSeconds;
}

void DayTime::getTime(int &h, int &m, int &s) const
{
    long seconds = labs(totalSeconds);

    h       = (int) (seconds / 3600L);
    seconds = seconds - (h * 3600L);
    m       = (int) (seconds / 60L);
    s       = (int) (seconds - (m * 60L));

    h *= sign(totalSeconds);
}

void DayTime::set(int h, int m, int s)
{
    DayTime dt(h, m, s);
    totalSeconds = dt.totalSeconds;
    checkHours();
}

void DayTime::set(const DayTime &other)
{
    totalSeconds = other.totalSeconds;
    checkHours();
}

// Add hours, wrapping days (which are not tracked)
void DayTime::addHours(float deltaHours)
{
    totalSeconds += long(deltaHours * 3600L);
    checkHours();
}

void DayTime::checkHours()
{
    while (totalSeconds >= secondsPerDay)
    {
        totalSeconds -= secondsPerDay;
    }

    while (totalSeconds < 0)
    {
        totalSeconds += secondsPerDay;
    }
}

// Add minutes, wrapping hours if needed
void DayTime::addMinutes(int deltaMins)
{
    totalSeconds += deltaMins * 60;
    checkHours();
}

// Add seconds, wrapping minutes and hours if needed
void DayTime::addSeconds(long deltaSecs)
{
    totalSeconds += deltaSecs;
    checkHours();
}

// Add another time, wrapping seconds, minutes and hours if needed
void DayTime::addTime(const DayTime &other)
{
    totalSeconds += other.totalSeconds;
    checkHours();
}

// Subtract another time, wrapping seconds, minutes and hours if needed
void DayTime::subtractTime(const DayTime &other)
{
    totalSeconds -= other.totalSeconds;
    checkHours();
}

char achBuf[32];

// Convert to a standard string (like 14:45:06)
const char *DayTime::ToString() const
{
    char *p = achBuf;
    int hours, mins, secs;
    getTime(hours, mins, secs);

    int absHours = abs(hours);
    if (totalSeconds < 0)
    {
        *p++ = '-';
    }
    if (absHours < 10)
    {
        *p++ = '0';
    }
    else
    {
        *p++ = '0' + (absHours / 10);
    }

    *p++ = '0' + (absHours % 10);

    *p++ = ':';
    if (mins < 10)
    {
        *p++ = '0';
    }
    else
    {
        *p++ = '0' + (mins / 10);
    }

    *p++ = '0' + (mins % 10);
    *p++ = ':';
    if (secs < 10)
    {
        *p++ = '0';
    }
    else
    {
        *p++ = '0' + (secs / 10);
    }

    *p++ = '0' + (secs % 10);
    *p++ = ' ';
    *p++ = '(';
    strcpy(p, String(this->getTotalHours(), 5).c_str());
    strcat(p, ")");
    return achBuf;
}
void DayTime::printTwoDigits(char *achDegs, int num) const
{
    achDegs[0] = '0' + (num / 10);
    achDegs[1] = '0' + (num % 10);
    achDegs[2] = 0;
}

const char *DayTime::formatStringImpl(char *targetBuffer, const char *format, char sgn, long degs, long mins, long secs) const
{
    char achDegs[5];
    char achMins[3];
    char achSecs[3];
    const char *f = format;
    char *p       = targetBuffer;

    int i = 0;
    if (sgn != '\0')
    {
        achDegs[0] = sgn;
        i++;
    }

    long absdegs = labs(degs);
    if (absdegs >= 100)
    {
        achDegs[i++] = '0' + min(9L, (absdegs / 100));
        absdegs      = absdegs % 100;
    }

    printTwoDigits(achDegs + i, absdegs);
    printTwoDigits(achMins, mins);
    printTwoDigits(achSecs, secs);

    char macro   = '\0';
    bool inMacro = false;
    while (*f)
    {
        switch (*f)
        {
            case '{':
                {
                    inMacro = true;
                }
                break;
            case '}':
                {
                    if (inMacro)
                    {
                        switch (macro)
                        {
                            case '+':
                                {
                                    *p++ = (degs < 0 ? '-' : '+');
                                }
                                break;
                            case 'd':
                                {
                                    strcpy(p, achDegs);
                                    p += strlen(achDegs);
                                }
                                break;
                            case 'm':
                                {
                                    strcpy(p, achMins);
                                    p += 2;
                                }
                                break;
                            case 's':
                                {
                                    strcpy(p, achSecs);
                                    p += 2;
                                }
                                break;
                        }
                        inMacro = false;
                    }
                }
                break;
            default:
                {
                    if (inMacro)
                    {
                        macro = *f;
                    }
                    else
                    {
                        *p++ = *f;
                    }
                }
        }
        f++;
    }

    *p = '\0';
    return targetBuffer;
}

const char *DayTime::formatString(char *targetBuffer, const char *format, long *pSecs) const
{
    long secs = pSecs == nullptr ? totalSeconds : *pSecs;
    char sgn  = secs < 0 ? '-' : '+';
    secs      = labs(secs);
    long degs = secs / 3600;
    secs      = secs - degs * 3600;
    long mins = secs / 60;
    secs      = secs - mins * 60;

    return formatStringImpl(targetBuffer, format, sgn, degs, mins, secs);
}
