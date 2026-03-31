#pragma once
#include "legacy//db/Row.h"

#include <vector>

#undef max

namespace DB {

class RowSet : public std::vector<Row> {

    using Base = std::vector<Row>;

public:
    std::shared_ptr<RowHeader> header; //!< The header of the rows

    /**
     * @brief Construct a new Row Set object
     *
     * @param header  The header(column names) of rows(shared_ptr)
     */
    explicit RowSet(std::shared_ptr<RowHeader> const& header = nullptr);
    /**
     * @brief Construct a new Row Set object
     *
     * @param header  The header(column names) of rows
     */
    explicit RowSet(RowHeader const& header);
    /// Move constructor
    RowSet(RowSet&& set) noexcept;
    /// Copy constructor
    RowSet(RowSet const& set);
    /// Move assignment operator
    RowSet& operator=(RowSet&& set) noexcept;
    /// Copy assignment operator
    RowSet& operator=(RowSet const& set);

    /**
     * @brief Add a row to the set.
     *
     * @param row  The row to add
     */
    void add(Row const& row);
    /**
     * @brief Get if the set is valid
     *
     * @return bool  True if valid
     */
    [[nodiscard]] bool valid() const;
    /**
     * @brief Add a row to the set.
     *
     * @param row  The row to add
     * @see   add(const Row&)
     */
    void push_back(Row const& row);
    /**
     * @brief Convert to the table string.
     *
     * @param  nullPattern When the value is null, what to replace with(default
     * '\<NULL\>')
     * @return std::string The result string
     * @par  sample
     * @code
     * |  a  |   b    |
     * |=====|========|
     * | awa | 114514 |
     * | qwq | 233    |
     * | ll  | <NULL> |
     * |=====|========|
     * @endcode
     */
    [[nodiscard]] std::string toTableString(std::string const& nullPattern = "<NULL>") const;
};

using ResultSet = RowSet;

} // namespace DB
