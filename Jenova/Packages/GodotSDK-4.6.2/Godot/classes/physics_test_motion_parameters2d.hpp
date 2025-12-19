/**************************************************************************/
/*  physics_test_motion_parameters2d.hpp                                  */
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

#include <Godot/classes/ref.hpp>
#include <Godot/classes/ref_counted.hpp>
#include <Godot/variant/rid.hpp>
#include <Godot/variant/transform2d.hpp>
#include <Godot/variant/typed_array.hpp>
#include <Godot/variant/vector2.hpp>

#include <Godot/core/class_db.hpp>

#include <type_traits>

namespace godot {

class PhysicsTestMotionParameters2D : public RefCounted {
	GDEXTENSION_CLASS(PhysicsTestMotionParameters2D, RefCounted)

public:
	Transform2D get_from() const;
	void set_from(const Transform2D &p_from);
	Vector2 get_motion() const;
	void set_motion(const Vector2 &p_motion);
	float get_margin() const;
	void set_margin(float p_margin);
	bool is_collide_separation_ray_enabled() const;
	void set_collide_separation_ray_enabled(bool p_enabled);
	TypedArray<RID> get_exclude_bodies() const;
	void set_exclude_bodies(const TypedArray<RID> &p_exclude_list);
	TypedArray<int> get_exclude_objects() const;
	void set_exclude_objects(const TypedArray<int> &p_exclude_list);
	bool is_recovery_as_collision_enabled() const;
	void set_recovery_as_collision_enabled(bool p_enabled);

protected:
	template <typename T, typename B>
	static void register_virtuals() {
		RefCounted::register_virtuals<T, B>();
	}

public:
};

} // namespace godot

