#include "legacyapi/db/Row.h"

#include "ll/api/utils/HashUtils.h"

#include <stdexcept>

namespace DB {

RowHeader::RowHeader(std::initializer_list<std::string> const& list) : std::vector<std::string>(list) {}

RowHeader::~RowHeader() = default;

size_t RowHeader::add(std::string const& name) {
    push_back(name);
    hashes.push_back(ll::hash_utils::doHash2(name));
    return size() - 1;
}

bool RowHeader::contains(std::string const& name) {
    return std::ranges::find(hashes, ll::hash_utils::doHash2(name)) != hashes.end();
}

void RowHeader::remove(std::string const& name) {
    auto hs = ll::hash_utils::doHash2(name);
    for (size_t i = 0; i < size(); ++i) {
        if (hashes[i] == hs) {
            erase(begin() + i);
            hashes.erase(hashes.begin() + i);
            return;
        }
    }
    throw std::runtime_error("RowHeader::remove: Column " + name + " is not found");
}

size_t RowHeader::at(std::string const& name) const {
    auto hs = ll::hash_utils::doHash2(name);
    for (size_t i = 0; i < size(); ++i) {
        if (hashes[i] == hs) return i;
    }
    throw std::out_of_range("RowHeader::at: Column " + name + " is not found");
}
std::string& RowHeader::at(size_t index) { return Base::at(index); }

size_t RowHeader::size() const { return Base::size(); }

bool RowHeader::empty() const { return Base::empty(); }

std::vector<std::string>::iterator RowHeader::begin() { return Base::begin(); }

std::vector<std::string>::iterator RowHeader::end() { return Base::end(); }

bool RowHeader::check(Row const& row) const {
    if (row.header.get() == this) return true;

    if (row.size() == size()) {
        if (row.header && !row.header->empty()) {
            if (row.header->size() == size()) return true;
            else return false;
        }
        return true;
    }
    return false;
}

size_t RowHeader::operator[](std::string const& name) {
    auto hs = ll::hash_utils::doHash2(name);
    for (size_t i = 0; i < size(); ++i) {
        if (hashes[i] == hs) return i;
    }
    return add(name);
}
std::string& RowHeader::operator[](size_t index) { return at(index); }

Row::Row(std::shared_ptr<RowHeader> const& header) : header(header) {}
Row::Row(RowHeader const& header) : header(std::make_shared<RowHeader>(header)) {}
Row::Row(std::initializer_list<Any> const& list, RowHeader const& header)
: std::vector<Any>(list),
  header(std::make_shared<RowHeader>(header)) {
    if (!header.empty() && list.size() != header.size()) {
        throw std::invalid_argument("Row::Row: The row and the header mismatch");
    }
}
Row::Row(std::initializer_list<Any> const& list, std::shared_ptr<RowHeader> const& header)
: std::vector<Any>(list),
  header(header) {
    if (header != nullptr && list.size() != header->size()) {
        throw std::invalid_argument("Row::Row: The row and the header mismatch");
    }
}
Row::Row(std::vector<Any>&& list, RowHeader const& header)
: std::vector<Any>(std::move(list)),
  header(std::make_shared<RowHeader>(header)) {
    if (size() != header.size()) {
        list = static_cast<vector>(std::move(*this)); // Restore
        throw std::invalid_argument("Row::Row: The row and the header mismatch");
    }
}
Row::Row(std::vector<Any> const& list, RowHeader const& header)
: std::vector<Any>(list),
  header(std::make_shared<RowHeader>(header)) {
    if (list.size() != header.size()) {
        throw std::invalid_argument("Row::Row: The row and the header mismatch");
    }
}
Row::Row(std::initializer_list<std::pair<std::string, Any>> const& list) : header(std::make_shared<RowHeader>()) {
    for (auto& pair : list) {
        header->add(pair.first);
        this->push_back(pair.second);
    }
}
Row::Row(Row&& row) noexcept : header(row.header) { *this = std::move(row); }
Row::Row(Row const& row) : header(row.header) { *this = row; }

Row& Row::operator=(Row&& row) noexcept {
    header = row.header;
    this->swap(row);
    return *this;
}
Row& Row::operator=(Row const& row) {
    header = row.header;
    this->assign(row.begin(), row.end());
    return *this;
}

Any& Row::operator[](std::string const& name) {
    auto idx = (*header)[name];
    if (idx < static_cast<int>(size())) return std::vector<Any>::at(idx);
    resize((size_t)idx + 1, Any());
    return std::vector<Any>::at(idx);
}
Any const& Row::operator[](std::string const& name) const { return std::vector<Any>::at(header->at(name)); }

Any&       Row::at(std::string const& name) { return std::vector<Any>::at(header->at(name)); }
Any const& Row::at(std::string const& name) const { return std::vector<Any>::at(header->at(name)); }

void Row::forEach_ref(std::function<bool(std::string const&, Any&)> const& cb) {
    if (!this->header) return;
    for (auto& col : *this->header) {
        if (!cb(col, this->at(col))) break;
    }
}

void Row::forEach(std::function<bool(std::string const&, Any const&)> const& cb) const {
    if (!this->header) return;
    int i = 0;
    for (auto& col : *this->header) {
        if (!cb(col, this->data()[i])) break;
        i++;
    }
}

} // namespace DB
