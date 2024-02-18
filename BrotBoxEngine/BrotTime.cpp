#include "BBE/BrotTime.h"
#include <time.h>

::tm bbe::TimePoint::toTm() const
{
	std::time_t nowT = std::chrono::system_clock::to_time_t(m_time);
	::tm retVal = *localtime(&nowT);
	return retVal;
}

bbe::TimePoint::TimePoint() :
	m_time(std::chrono::system_clock::now())
{
}

bbe::TimePoint::TimePoint(const std::chrono::system_clock::time_point &time) :
	m_time(time)
{
}

bbe::TimePoint::TimePoint(std::time_t time) :
	m_time(std::chrono::system_clock::from_time_t(time))
{
}

bbe::TimePoint bbe::TimePoint::todayAt(int32_t hour, int32_t minute, int32_t second)
{
	while (second < 0)
	{
		second += 60;
		minute -= 1;
	}
	while (second >= 60)
	{
		second -= 60;
		minute += 1;
	}
	while (minute < 0)
	{
		minute += 60;
		hour -= 1;
	}
	while (minute >= 60)
	{
		minute -= 60;
		hour += 1;
	}
	int32_t daysOffset = 0;
	while (hour < 0)
	{
		hour += 24;
		daysOffset -= 1;
	}
	while (hour >= 24)
	{
		hour -= 24;
		daysOffset += 1;
	}

	bbe::TimePoint now;
	::tm t = now.toTm();
	t.tm_hour = hour;
	t.tm_min = minute;
	t.tm_sec = second;

	return TimePoint(::mktime(&t)).plusDays(daysOffset);
}

bbe::TimePoint bbe::TimePoint::fromDate(int32_t year, Month month, int32_t day, int32_t hour, int32_t minute, int32_t second)
{
	::tm t = {};
	t.tm_year = year - 1900;
	t.tm_mon = (int)month - 1;
	t.tm_mday = day;
	t.tm_hour = hour;
	t.tm_min = minute;
	t.tm_sec = second;

	return TimePoint(::mktime(&t));
}

bbe::TimePoint bbe::TimePoint::fromDate(int32_t year, int32_t month, int32_t day, int32_t hour, int32_t minute, int32_t second)
{
	return fromDate(year, (Month)month, day, hour, minute, second);
}

bbe::TimePoint bbe::TimePoint::epoch()
{
	return TimePoint(0);
}

int32_t bbe::TimePoint::isLeapYear(int32_t year)
{
	if (year % 400 == 0) return true;
	if (year % 100 == 0) return false;
	if (year %   4 == 0) return true;
	return false;
}

int32_t bbe::TimePoint::getDaysInMonth(int32_t year, Month month)
{
	switch (month)
	{
	case Month::JANUARY  : return 31;
	case Month::FEBRUARY : return isLeapYear(year) ? 29 : 28;
	case Month::MARCH    : return 31;
	case Month::APRIL    : return 30;
	case Month::MAY      : return 31;
	case Month::JUNE     : return 30;
	case Month::JULY     : return 31;
	case Month::AUGUST   : return 31;
	case Month::SEPTEMBER: return 30;
	case Month::OCTOBER  : return 31;
	case Month::NOVEMBER : return 30;
	case Month::DECEMBER : return 31;
	default: throw std::runtime_error("Illegal Argument");
	}
	return 0;
}

bbe::Weekday bbe::TimePoint::getFirstWeekdayOfMonth(int32_t year, Month month)
{
	return bbe::TimePoint::fromDate(year, month, 1).getWeekday();
}

bbe::TimePoint bbe::TimePoint::nextMorning(int64_t morningHour) const
{
	std::time_t t = std::chrono::system_clock::to_time_t(m_time);
	::tm timeinfo = *localtime(&t);
	if (timeinfo.tm_hour >= morningHour)
	{
		bbe::TimePoint nextDay(m_time);
		nextDay = nextDay.plusDays(1);
		t = std::chrono::system_clock::to_time_t(nextDay.m_time);
		timeinfo = *localtime(&t);
	}
	timeinfo.tm_sec = 0;
	timeinfo.tm_min = 0;
	timeinfo.tm_hour = morningHour;
	t = ::mktime(&timeinfo);
	return TimePoint(t);
}

bbe::TimePoint bbe::TimePoint::toMorning(int64_t morningHour) const
{
	std::time_t t = std::chrono::system_clock::to_time_t(m_time);
	::tm timeinfo = *localtime(&t);
	timeinfo.tm_sec = 0;
	timeinfo.tm_min = 0;
	timeinfo.tm_hour = morningHour;
	t = ::mktime(&timeinfo);
	return TimePoint(t);
}

bbe::TimePoint bbe::TimePoint::plusDays(int64_t days) const
{
	using namespace std::literals;
	return m_time + 24h * days;
}

bbe::TimePoint bbe::TimePoint::plusHours(int64_t hours) const
{
	using namespace std::literals;
	return m_time + 1h * hours;
}

bbe::TimePoint bbe::TimePoint::plusMinutes(int64_t minutes) const
{
	using namespace std::literals;
	return m_time + 1min * minutes;
}

bbe::TimePoint bbe::TimePoint::plusSeconds(int64_t seconds) const
{
	using namespace std::literals;
	return m_time + 1s * seconds;
}

bbe::TimePoint bbe::TimePoint::plusMilliseconds(int64_t ms) const
{
	using namespace std::literals;
	return m_time + 1ms * ms;
}

bbe::Duration bbe::TimePoint::operator-(const bbe::TimePoint& other) const
{
	return bbe::Duration(m_time - other.m_time);
}

bool bbe::TimePoint::operator<(const bbe::TimePoint& other) const
{
	return m_time < other.m_time;
}

bool bbe::TimePoint::operator>(const bbe::TimePoint& other) const
{
	return m_time > other.m_time;
}

bool bbe::TimePoint::hasPassed() const
{
	// Rationale for >= instead of >:
	// Taking the current time does itself always take a tiny bit of time (maybe sub nano seconds but not 0).
	// So when ever you take the current time, that meassurement must have already passed the moment we even
	// look at it. It can only be == because the measurement isn't precise enough. In other words,
	// TimePoint().hasPassed() shall always return true. Additionally, this assumption does simplify some code
	// in ExampleMother, which may indicate that it's the right choice for other situations as well.
	return std::chrono::system_clock::now() >= m_time;
}

bool bbe::TimePoint::isNight(int64_t fromHour, int64_t toHour) const
{
	std::time_t t = std::chrono::system_clock::to_time_t(m_time);
	::tm timeinfo = *localtime(&t);

	if (fromHour < toHour)
	{
		return timeinfo.tm_hour >= fromHour && timeinfo.tm_hour <= toHour;
	}
	else
	{
		return timeinfo.tm_hour >= fromHour || timeinfo.tm_hour <= toHour;
	}
}

static int getWDay(const std::chrono::system_clock::time_point& tp)
{
	std::time_t t = std::chrono::system_clock::to_time_t(tp);
	::tm timeinfo = *localtime(&t);
	return timeinfo.tm_wday;
}

bbe::Weekday bbe::TimePoint::getWeekday() const
{
	int wday = getWDay(m_time);
	switch (wday)
	{
	case 0: return bbe::Weekday::SUNDAY;
	case 1: return bbe::Weekday::MONDAY;
	case 2: return bbe::Weekday::TUESDAY;
	case 3: return bbe::Weekday::WEDNESDAY;
	case 4: return bbe::Weekday::THURSDAY;
	case 5: return bbe::Weekday::FRIDAY;
	case 6: return bbe::Weekday::SATURDAY;
	default: throw std::runtime_error("Illegal wday");
	}
}

bool bbe::TimePoint::isMonday() const
{
	return getWDay(m_time) == 1;
}

bool bbe::TimePoint::isTuesday() const
{
	return getWDay(m_time) == 2;
}

bool bbe::TimePoint::isWednesday() const
{
	return getWDay(m_time) == 3;
}

bool bbe::TimePoint::isThursday() const
{
	return getWDay(m_time) == 4;
}

bool bbe::TimePoint::isFriday() const
{
	return getWDay(m_time) == 5;
}

bool bbe::TimePoint::isSaturday() const
{
	return getWDay(m_time) == 6;
}

bool bbe::TimePoint::isSunday() const
{
	return getWDay(m_time) == 0;
}

bool bbe::TimePoint::isToday() const
{
	::tm thisTimeinfo = toTm();
	::tm nowTimeinfo = bbe::TimePoint().toTm();

	return thisTimeinfo.tm_year == nowTimeinfo.tm_year && thisTimeinfo.tm_yday == nowTimeinfo.tm_yday;
}

bbe::String bbe::TimePoint::toString() const
{
	::time_t t = std::chrono::system_clock::to_time_t(m_time);
	const char* c = ::ctime(&t);
	if (!c) c = "ERROR";
	return bbe::String(c);
}

void bbe::TimePoint::serialize(bbe::ByteBuffer& buffer) const
{
	int64_t val = (int64_t)std::chrono::system_clock::to_time_t(m_time);
	buffer.write(val);
}

bbe::TimePoint bbe::TimePoint::deserialize(bbe::ByteBufferSpan& buffer)
{
	int64_t val;
	buffer.read(val);
	return TimePoint((::time_t)val);
}

int32_t bbe::TimePoint::getYear() const
{
	return toTm().tm_year + 1900;
}

bbe::Month bbe::TimePoint::getMonth() const
{
	return bbe::Month(toTm().tm_mon + 1);
}

int32_t bbe::TimePoint::getDay() const
{
	return toTm().tm_mday;
}

bbe::Duration::Duration()
{
}

bbe::Duration::Duration(const std::chrono::system_clock::duration& duration) :
	m_duration(duration)
{
}

bbe::String bbe::Duration::toString() const
{
	int32_t seconds = toSeconds();
	int32_t minutes = seconds / 60;
	seconds %= 60;
	int32_t hours = minutes / 60;
	minutes %= 60;
	int32_t days = hours / 24;
	hours %= 24;

	if(days == 0)
		if(hours == 0)
			return bbe::String::format("%.2d:%.2d", minutes, seconds);
		else
			return bbe::String::format("%.2d:%.2d:%.2d", hours, minutes, seconds);
	else
		return bbe::String::format("%d:%.2d:%.2d:%.2d", days, hours, minutes, seconds);
}

int32_t bbe::Duration::toSeconds() const
{
	return std::chrono::duration_cast<std::chrono::seconds>(m_duration).count();
}

int32_t bbe::Duration::toMinutes() const
{
	return std::chrono::duration_cast<std::chrono::minutes>(m_duration).count();
}

int32_t bbe::Duration::toHours() const
{
	return std::chrono::duration_cast<std::chrono::hours>(m_duration).count();
}

int32_t bbe::Duration::toDays() const
{
	return toHours() / 24;
}

bool bbe::Duration::isNegative() const
{
	return toSeconds() < 0;
}

bool bbe::TimeGate::everyMilliseconds(int32_t ms)
{
	if (m_nextExec.hasPassed())
	{
		m_nextExec = bbe::TimePoint().plusMilliseconds(ms);
		return true;
	}
	return false;
}

bool bbe::TimeGate::everySeconds(int32_t seconds)
{
	if (m_nextExec.hasPassed())
	{
		m_nextExec = bbe::TimePoint().plusSeconds(seconds);
		return true;
	}
	return false;
}

bool bbe::TimeGate::everyMinutes(int32_t minutes)
{
	if (m_nextExec.hasPassed())
	{
		m_nextExec = bbe::TimePoint().plusMinutes(minutes);
		return true;
	}
	return false;
}

bool bbe::TimeGate::everyHours(int32_t hours)
{
	if (m_nextExec.hasPassed())
	{
		m_nextExec = bbe::TimePoint().plusHours(hours);
		return true;
	}
	return false;
}
