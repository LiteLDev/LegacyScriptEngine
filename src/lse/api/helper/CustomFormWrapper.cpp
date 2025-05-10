#include "CustomFormWrapper.h"

#include "nlohmann/json.hpp"

namespace lse::form {

CustomFormResult
CustomFormWrapper::convertResult(std::optional<std::string> const& result, std::vector<int> const& resultIndices) {
    if (!result) return {};
    auto data = nlohmann::ordered_json::parse(*result);
    if (!data.is_null() && !data.is_array()) return {};

    auto   iter  = resultIndices.rbegin();
    size_t count = iter == resultIndices.rend() ? 0 : *iter;
    if (data.size() == count) {
        return data;
    }

    auto views = resultIndices | std::views::transform([&](int index) {
                     if (index < 0) return nlohmann::ordered_json();
                     return data[index];
                 });
    return std::vector(views.begin(), views.end());
}

CustomFormResult
CustomFormWrapper::convertResult(std::optional<std::string> const& result, nlohmann::ordered_json const& formData) {
    if (!result) return {};
    auto type    = formData.find("type");
    auto content = formData.find("content");
    if (content == formData.end() || type == formData.end() || *type != "custom") {
        return nlohmann::ordered_json::parse(*result);
    }
    std::vector<int>      resultIndices{};
    int                   index = 0;
    std::set<std::string> fillNullTypes({"header", "label", "divider"});
    for (auto& data : *content) {
        auto type = data.find("type");
        if (type == data.end()) return {};
        if (fillNullTypes.contains(*type)) resultIndices.emplace_back(-1);
        else resultIndices.emplace_back(index++);
    }
    return convertResult(result, resultIndices);
}

ll::form::Form::RawFormCallback
CustomFormWrapper::convertCallback(Callback&& callback, std::vector<int> resultIndices) {
    ll::form::Form::RawFormCallback oriCallback{};
    if (callback)
        oriCallback = [resultIndices = std::move(resultIndices),
                       callback      = std::move(callback
                       )](Player& player, std::optional<std::string> const& data, ll::form::FormCancelReason reason) {
            callback(player, convertResult(data, resultIndices), reason);
        };
    return oriCallback;
}

} // namespace lse::form