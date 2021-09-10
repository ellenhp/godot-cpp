/*************************************************************************/
/*  godot.cpp                                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include <godot_cpp/godot.hpp>

#include <godot_cpp/classes/wrapped.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <godot_cpp/core/error_macros.hpp>

namespace godot {

namespace internal {

const GDNativeInterface *interface = nullptr;
GDNativeExtensionClassLibraryPtr library = nullptr;
void *token = nullptr;

} // namespace internal

GDExtensionBinding::Callback GDExtensionBinding::core_init = nullptr;
GDExtensionBinding::Callback GDExtensionBinding::server_init = nullptr;
GDExtensionBinding::Callback GDExtensionBinding::scene_init = nullptr;
GDExtensionBinding::Callback GDExtensionBinding::editor_init = nullptr;
GDExtensionBinding::Callback GDExtensionBinding::core_terminate = nullptr;
GDExtensionBinding::Callback GDExtensionBinding::server_terminate = nullptr;
GDExtensionBinding::Callback GDExtensionBinding::scene_terminate = nullptr;
GDExtensionBinding::Callback GDExtensionBinding::editor_terminate = nullptr;

GDNativeBool GDExtensionBinding::init(const GDNativeInterface *p_interface, const GDNativeExtensionClassLibraryPtr p_library, GDNativeInitialization *r_initialization) {
	internal::interface = p_interface;
	internal::library = p_library;
	internal::token = p_library;

	r_initialization->initialize = initialize_level;
	r_initialization->deinitialize = deinitialize_level;

	if (core_init) {
		r_initialization->minimum_initialization_level = GDNATIVE_INITIALIZATION_CORE;
	} else if (server_init) {
		r_initialization->minimum_initialization_level = GDNATIVE_INITIALIZATION_SERVERS;
	} else if (scene_init) {
		r_initialization->minimum_initialization_level = GDNATIVE_INITIALIZATION_SCENE;
	} else if (editor_init) {
		r_initialization->minimum_initialization_level = GDNATIVE_INITIALIZATION_EDITOR;
	} else {
		ERR_FAIL_V_MSG(false, "At least one initialization callback must be defined.");
	}

	Variant::init_bindings();

	return true;
}

void GDExtensionBinding::initialize_level(void *userdata, GDNativeInitializationLevel p_level) {
	ClassDB::current_level = p_level;
	switch (p_level) {
		case GDNATIVE_INITIALIZATION_CORE:
			if (core_init) {
				core_init();
			}
			break;
		case GDNATIVE_INITIALIZATION_SERVERS:
			if (server_init) {
				server_init();
			}
			break;
		case GDNATIVE_INITIALIZATION_SCENE:
			if (scene_init) {
				scene_init();
			}
			break;
		case GDNATIVE_INITIALIZATION_EDITOR:
			if (editor_init) {
				editor_init();
			}
			break;
	}
	ClassDB::initialize(p_level);
}

void GDExtensionBinding::deinitialize_level(void *userdata, GDNativeInitializationLevel p_level) {
	ClassDB::current_level = p_level;
	ClassDB::deinitialize(p_level);
	switch (p_level) {
		case GDNATIVE_INITIALIZATION_CORE:
			if (core_terminate) {
				core_terminate();
			}
			break;
		case GDNATIVE_INITIALIZATION_SERVERS:
			if (server_terminate) {
				server_terminate();
			}
			break;
		case GDNATIVE_INITIALIZATION_SCENE:
			if (scene_terminate) {
				scene_terminate();
			}
			break;
		case GDNATIVE_INITIALIZATION_EDITOR:
			if (editor_terminate) {
				editor_terminate();
			}
			break;
	}
}

void *GDExtensionBinding::create_instance_callback(void *p_token, void *p_instance) {
	ERR_FAIL_COND_V_MSG(p_token != internal::library, nullptr, "Asking for creating instance with invalid token.");
	Wrapped *wrapped = memnew(Wrapped(p_instance));
	return wrapped;
}

void GDExtensionBinding::free_instance_callback(void *p_token, void *p_instance, void *p_binding) {
	ERR_FAIL_COND_MSG(p_token != internal::library, "Asking for freeing instance with invalid token.");
	memdelete((Wrapped *)p_binding);
}

void GDExtensionBinding::InitObject::register_core_initializer(Callback p_core_init) const {
	GDExtensionBinding::core_init = p_core_init;
}

void GDExtensionBinding::InitObject::register_server_initializer(Callback p_server_init) const {
	GDExtensionBinding::server_init = p_server_init;
}

void GDExtensionBinding::InitObject::register_scene_initializer(Callback p_scene_init) const {
	GDExtensionBinding::scene_init = p_scene_init;
}

void GDExtensionBinding::InitObject::register_editor_initializer(Callback p_editor_init) const {
	GDExtensionBinding::editor_init = p_editor_init;
}

void GDExtensionBinding::InitObject::register_core_terminator(Callback p_core_terminate) const {
	GDExtensionBinding::core_terminate = p_core_terminate;
}

void GDExtensionBinding::InitObject::register_server_terminator(Callback p_server_terminate) const {
	GDExtensionBinding::server_terminate = p_server_terminate;
}

void GDExtensionBinding::InitObject::register_scene_terminator(Callback p_scene_terminate) const {
	GDExtensionBinding::scene_terminate = p_scene_terminate;
}

void GDExtensionBinding::InitObject::register_editor_terminator(Callback p_editor_terminate) const {
	GDExtensionBinding::editor_terminate = p_editor_terminate;
}

GDNativeBool GDExtensionBinding::InitObject::init() const {
	return GDExtensionBinding::init(interface, library, initialization);
}

} // namespace godot

extern "C" {

void GDN_EXPORT initialize_level(void *userdata, GDNativeInitializationLevel p_level) {
	godot::GDExtensionBinding::initialize_level(userdata, p_level);
}

void GDN_EXPORT deinitialize_level(void *userdata, GDNativeInitializationLevel p_level) {
	godot::GDExtensionBinding::deinitialize_level(userdata, p_level);
}
}
