#include "CustomFormWrapper.h"

#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"

namespace lse::form {

std::set<std::string> const CustomFormWrapper::COMMON_ELEMENT_TYPENAMES{"header", "label", "divider"};

CustomFormResult
CustomFormWrapper::convertResult(std::optional<std::string> const& result, std::vector<int> const& resultIndices) {
    if (!result) return {};
    auto data = nlohmann::ordered_json::parse(*result);
    if (!data.is_null() && !data.is_array()) return {};

    size_t count = resultIndices.size();
    if (data.size() == 0) return nlohmann::ordered_json::array({});
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
    auto formType = formData.find("type");
    auto content  = formData.find("content");
    if (content == formData.end() || formType == formData.end() || *formType != "custom_form") {
        return nlohmann::ordered_json::parse(*result);
    }
    std::vector<int> resultIndices{};
    resultIndices.reserve(content->size());
    int index = 0;
    for (auto& element : *content) {
        auto elementType = element.find("type");
        if (elementType == element.end()) return {};
        if (COMMON_ELEMENT_TYPENAMES.contains(*elementType)) resultIndices.emplace_back(-1);
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