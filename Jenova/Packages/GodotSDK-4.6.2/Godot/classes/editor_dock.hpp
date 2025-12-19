/**************************************************************************/
/*  editor_dock.hpp                                                       */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

// THIS FILE IS GENERATED. EDITS WILL BE LOST.

#pragma once

#include <Godot/classes/editor_plugin.hpp>
#include <Godot/classes/margin_container.hpp>
#include <Godot/classes/ref.hpp>
#include <Godot/variant/string.hpp>
#include <Godot/variant/string_name.hpp>

#include <Godot/core/class_db.hpp>

#include <type_traits>

namespace godot {

class ConfigFile;
class Shortcut;
class Texture2D;

class EditorDock : public MarginContainer {
	GDEXTENSION_CLASS(EditorDock, MarginContainer)

public:
	enum DockLayout : uint64_t {
		DOCK_LAYOUT_VERTICAL = 1,
		DOCK_LAYOUT_HORIZONTAL = 2,
	};

	void set_title(const String &p_title);
	String get_title() const;
	void set_layout_key(const String &p_layout_key);
	String get_layout_key() const;
	void set_icon_name(const StringName &p_icon_name);
	StringName get_icon_name() const;
	void set_dock_icon(const Ref<Texture2D> &p_icon);
	Ref<Texture2D> get_dock_icon() const;
	void set_dock_shortcut(const Ref<Shortcut> &p_shortcut);
	Ref<Shortcut> get_dock_shortcut() const;
	void set_default_slot(EditorPlugin::DockSlot p_slot);
	EditorPlugin::DockSlot get_default_slot() const;
	void set_available_layouts(BitField<EditorDock::DockLayout> p_layouts);
	BitField<EditorDock::DockLayout> get_available_layouts() const;
	virtual void _update_layout(int32_t p_layout);
	virtual void _save_layout_to_config(const Ref<ConfigFile> &p_config, const String &p_section) const;
	virtual void _load_layout_from_config(const Ref<ConfigFile> &p_config, const String &p_section);

protected:
	template <typename T, typename B>
	static void register_virtuals() {
		MarginContainer::register_virtuals<T, B>();
		if constexpr (!std::is_same_v<decltype(&B::_update_layout), decltype(&T::_update_layout)>) {
			BIND_VIRTUAL_METHOD(T, _update_layout, 1286410249);
		}
		if constexpr (!std::is_same_v<decltype(&B::_save_layout_to_config), decltype(&T::_save_layout_to_config)>) {
			BIND_VIRTUAL_METHOD(T, _save_layout_to_config, 3076455711);
		}
		if constexpr (!std::is_same_v<decltype(&B::_load_layout_from_config), decltype(&T::_load_layout_from_config)>) {
			BIND_VIRTUAL_METHOD(T, _load_layout_from_config, 2838822993);
		}
	}

public:
};

} // namespace godot

VARIANT_BITFIELD_CAST(EditorDock::DockLayout);

