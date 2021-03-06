// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2020 Team Dissolve and contributors

#pragma once

#include "genericitems/container.h"

// GenericItemContainer<int>
template <> class GenericItemContainer<int> : public GenericItem
{
    public:
    GenericItemContainer<int>(std::string_view name, int flags = 0) : GenericItem(name, flags) {}

    /*
     * Data
     */
    private:
    // Data item
    int data_;

    public:
    // Return data item
    int &data() { return data_; }

    /*
     * Item Class
     */
    protected:
    // Create a new GenericItem containing same class as current type
    GenericItem *createItem(std::string_view className, std::string_view name, int flags = 0)
    {
        if (DissolveSys::sameString(className, itemClassName()))
            return new GenericItemContainer<int>(name, flags);
        return nullptr;
    }

    public:
    // Return class name contained in item
    std::string_view itemClassName() { return "int"; }

    /*
     * I/O
     */
    public:
    // Write data through specified parser
    bool write(LineParser &parser) { return parser.writeLineF("{}\n", data_); }
    // Read data through specified parser
    bool read(LineParser &parser, CoreData &coreData)
    {
        if (parser.getArgsDelim(LineParser::Defaults) != LineParser::Success)
            return false;
        data_ = parser.argi(0);
        return true;
    }

    /*
     * Parallel Comms
     */
    public:
    // Broadcast item contents
    bool broadcast(ProcessPool &procPool, const int root, const CoreData &coreData) { return procPool.broadcast(data_, root); }
    // Check item equality
    bool equality(ProcessPool &procPool) { return procPool.equality(data_); }
};
