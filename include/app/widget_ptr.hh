/**
 * aurgh Copyright (C) 2025 RQuarx
 *
 * This file is part of aurgh
 *
 * aurgh is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * aurgh is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with aurgh. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once
#ifndef __WIDGET_PTR__HH
#define __WIDGET_PTR__HH

#include <utility>


template<typename T_Widget>
class widget_ptr
{
public:
    widget_ptr() noexcept = default;

    template<typename... T_Args>
    explicit widget_ptr(T_Args &&...p_construct_args) :
        m_widget(new T_Widget(std::forward<T_Args>(p_construct_args)...))
    {}

    explicit widget_ptr(T_Widget *p_widget) noexcept : m_widget(p_widget) {}

    /* Copy & copy assignment operator */
    widget_ptr(const widget_ptr &) = delete;
    auto operator=(const widget_ptr &) -> widget_ptr & = delete;

    /* Move & move assignment operator */
    widget_ptr(widget_ptr &&p_other) noexcept :
        m_widget(p_other.m_widget)
    { p_other.m_widget = nullptr; }


    auto
    operator=(widget_ptr &&p_other) noexcept -> widget_ptr &
    {
        if (this != &p_other) {
            delete m_widget;
            m_widget = p_other.m_widget;
            p_other.m_widget = nullptr;
        }
        return *this;
    }


    ~widget_ptr()
    { delete m_widget; }


    auto
    operator=(T_Widget *p_widget) -> widget_ptr &
    {
        if (m_widget == nullptr) {
            m_widget = p_widget;
        } else {
            delete m_widget;
            m_widget = p_widget;
        }
        return *this;
    }


    auto
    operator*() const noexcept -> T_Widget &
    { return *m_widget; }


    auto
    operator->() const noexcept -> T_Widget *
    { return m_widget; }


    [[nodiscard]]
    auto get() const noexcept -> T_Widget *
    { return m_widget; }


    void swap(widget_ptr &other) noexcept
    { std::swap(m_widget, other.m_widget); }

private:
    T_Widget *m_widget = nullptr;
};

#endif /* __WIDGET_PTR__HH */