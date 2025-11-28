#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace weave::tests {

struct TestReport {
    std::vector<std::string> failures;

    void AddFailure(std::string_view message) {
        failures.emplace_back(message);
    }

    void AddFailure(std::string message) {
        failures.emplace_back(std::move(message));
    }

    bool Passed() const { return failures.empty(); }
    explicit operator bool() const { return Passed(); }

    void Expect(bool condition, std::string_view message) {
        if (!condition) {
            AddFailure(message);
        }
    }

    void Merge(const TestReport& other) {
        failures.insert(failures.end(), other.failures.begin(), other.failures.end());
    }
};

} // namespace weave::tests
