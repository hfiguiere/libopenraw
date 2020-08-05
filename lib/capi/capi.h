/* -*- mode:c++; indent-tabs-mode:nil; c-basic-offset:4; tab-width:4; -*- */

#pragma once

#include <memory>

/*
 * Use this to mark a symbol to be exported
 */
#define API_EXPORT __attribute__ ((visibility ("default")))

/// Wrap a pointer so that we can return it.
template<class T>
class WrappedPointer {
public:
    WrappedPointer(const std::shared_ptr<T>& p)
        : m_p(p)
        {}
    const std::shared_ptr<T>& ptr() const
        { return m_p; }
private:
    std::shared_ptr<T> m_p;
};
