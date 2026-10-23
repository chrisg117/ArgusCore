/*
 * Local dev tooling (chore(local), not for upstream): appends one JSON object per line to
 * <LogsDir>/devtrace.jsonl so an agent can read what happened during a test run.
 */

#ifndef DEV_TRACE_H
#define DEV_TRACE_H

#include "Define.h"
#include <initializer_list>
#include <string>
#include <string_view>

namespace DevTrace
{
    std::string Escape(std::string_view text);

    // Pre-built "key":value fragments for Write's field list.
    std::string Str(char const* key, std::string_view value);
    std::string Num(char const* key, int64 value);
    std::string Bool(char const* key, bool value);

    // {"t":..,"src":..,"char":..,"kind":..,<fields>} + newline, flushed at once.
    void Write(std::string_view src, std::string_view charName, std::string_view kind, std::initializer_list<std::string> fields = {});
}

#endif
