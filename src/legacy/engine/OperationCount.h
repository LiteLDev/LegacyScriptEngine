#pragma once
#include <string>
using std::string;

class OperationCount {
private:
    string name;

public:
    OperationCount(string const& name);
    static OperationCount create(string const& name);
    static bool           exists(string const& name);
    bool                  remove() const;
                          operator bool() const { return exists(name); }

    bool        done() const;
    inline bool finish() const { return done(); }

    int  get() const;
    bool hasReachCount(int count) const;
    bool hasReachMaxEngineCount() const;
    bool hasReachMaxBackendCount() const;
};
