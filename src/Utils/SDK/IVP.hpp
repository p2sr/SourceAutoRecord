#pragma once

#include "Utils/SDK/Math.hpp"

// NOTE: these definitions have only been confirmed correct on Linux so far.

/// utility ///

//enum class IVP_BOOL { TRUE, FALSE };

struct IVP_Time {
	float seconds;
	float sub_seconds;
};

struct alignas(16) IVP_U_Float_Point {
	float x, y, z, hesse_val;
	inline Vector toVector() {
		return Vector{x, y, z};
	}
};

typedef IVP_U_Float_Point IVP_U_Point;

struct IVP_U_Float_Hesse {
	float x, y, z;
	float hesse_val;
};

struct IVP_U_Vector_Base {
	unsigned short memsize;
	unsigned short nelems;
	void **elems;
};

template<typename T>
struct IVP_U_Vector : public IVP_U_Vector_Base {
	T *get(unsigned short idx) {
		if (idx >= this->nelems) return nullptr;
		else return (T *)this->elems[idx];
	}
};

// FVector does differ from Vector, but only in operations I haven't implemented
// here, namely deletion
template<typename T>
struct IVP_U_FVector : public IVP_U_Vector_Base {
	T *get(unsigned short idx) {
		if (idx >= this->nelems) return nullptr;
		else return (T *)this->elems[idx];
	}
};

struct IVP_Core;
struct IVP_Vector_of_Cores_2 : public IVP_U_Vector<IVP_Core> {
	IVP_Core *elem_buffer[2];
};

struct IVP_Real_Object;
struct IVP_Vector_of_Objects : public IVP_U_Vector<IVP_Real_Object> {
	IVP_Real_Object *elem_buffer[1];
};

struct IVP_U_Min_List {
	unsigned short malloced_size;
	unsigned short free_list;
	void *elems;
	float min_value;
	int first_long;
	int first_element;
	int counter;
};

#define INCH_PER_METER 39.3701f
inline Vector ivpToHl(Vector ivp) {
	return Vector{
		ivp.x * INCH_PER_METER,
		ivp.z * INCH_PER_METER,
		-ivp.y * INCH_PER_METER,
	};
}


/// objects ///

struct IVP_Hull_Manager_Base {
	IVP_Time last_vpsi_time;
	float gradient;
	float center_gradient;
	float hull_value_last_vpsi;
	float hull_center_value_last_vpsi;
	float hull_value_next_psi;
	int time_of_next_reset;
	IVP_U_Min_List sorted_synapses;
};

struct IVP_Object {
	virtual ~IVP_Object();

	int object_type;
	IVP_Object *next_in_cluster, *prev_in_cluster;
	void *parent_cluster;
	const char *name;
};

struct IVP_Real_Object_Fast_Static : public IVP_Object {
	void *controller_phantom;
	void *exact_synapses;
	void *invalid_synapses;
	void *friction_synapses;
	void *q_core_f_object;
	IVP_U_Float_Point shift_core_f_object;
};

struct IVP_Real_Object_Fast : public IVP_Real_Object_Fast_Static {
	void *cache_object;
	struct {
		int object_movement_state : 8;
		bool collision_detection_enabled : 1;
		bool collision_detection_is_debris : 1;
		bool collision_detection_is_static_solid : 1;
		bool shift_core_f_object_is_zero : 1;
		unsigned object_listener_exists : 1;
		unsigned collision_listener_exists : 1;
		unsigned collision_listener_listens_to_friction : 1;
	} flags;
	IVP_Hull_Manager_Base hull_manager;
};

struct IVP_OV_Element;
struct CPhysicsObject;
struct IVP_Real_Object : public IVP_Real_Object_Fast {
	virtual ~IVP_Real_Object();

	void *anchors;
	void *surface_manager;
	void *default_material;
	IVP_OV_Element *ov_element;
	float extra_radius;

	// current core
	IVP_Core *physical_core; // this is usually what we give a shit about
	IVP_Core *friction_core;
	// was valid at object creation
	IVP_Core *original_core;

	CPhysicsObject *client_data; // actually a 'void *' but vphysics always sets it to the CPhysicsObject
};


/// ov tree ///

struct IVP_OV_Node;
struct IVP_Collision;
struct IVP_OV_Element {
	void *vtable;
	unsigned minlist_index; // from IVP_Listener_Hull superclass
	
	IVP_OV_Node *node;
	void *hull_manager;
	IVP_U_Float_Point center;
	float radius;
	IVP_Real_Object *real_object;
	IVP_U_FVector<IVP_Collision> collision_fvector;
};

struct IVP_OV_Node_Data {
	int x, y, z;
	int rasterlevel;
	int sizelevel;
};

struct IVP_OV_Node {
	IVP_OV_Node_Data data;

	IVP_OV_Node *parent;
	IVP_U_Vector<IVP_OV_Node> children;
	IVP_U_Vector<IVP_OV_Element> elements;
};

struct IVP_Environment;
struct IVP_OV_Tree_Manager {
	IVP_OV_Node search_node;
	IVP_U_Vector<IVP_OV_Element> *collision_partners;
	void *hash_table;
	IVP_Environment *environment;
	IVP_OV_Node *root;
};

struct IVP_Simulation_Unit {
	int sim_unit_movement_type : 8;
	int union_find_needed_for_sim_unit : 2;
	int sim_unit_has_fast_objects : 2;
	int sim_unit_just_slowed_down : 2;

	IVP_Simulation_Unit *prev_sim_unit;
	IVP_Simulation_Unit *next_sim_unit;

	IVP_Vector_of_Cores_2 sim_unit_cores;
	// TODO: controller_cores
};

struct IVP_Core_Fast_Static {
	char unknown[0x50]; // TODO
	IVP_Vector_of_Objects objects;
	float abs_omega;
};

struct IVP_Core_Fast_PSI : public IVP_Core_Fast_Static {
	char unknown[0x70]; // TODO
	IVP_U_Float_Point speed;
	IVP_U_Point pos_world_f_core_last_psi;
	// TODO incomplete
};

struct IVP_Core : public IVP_Core_Fast_PSI {
	// TODO incomplete
};

// TODO validate this constant
#define IVP_SIM_SLOTS_NUM 100
struct IVP_Sim_Units_Manager {
	IVP_Environment *environment;
	IVP_Time nb;
	IVP_Time bt;
	IVP_Simulation_Unit *sim_units_slots[IVP_SIM_SLOTS_NUM];
	IVP_Simulation_Unit *still_slot;
};

struct IVP_Environment {
	void *standard_gravity_controller;
	void *time_manager;
	IVP_Sim_Units_Manager *sim_units_manager;
	void *cluster_manager;
	void *mindist_manager;
	IVP_OV_Tree_Manager *ov_tree_manager;
	// (incomplete)
};


/// vphysics ///

struct CPhysicsEnvironment {
	void *vtable;
	IVP_Environment *phys_env;
	// (incomplete)
};

struct CPhysicsObject {
	void *vtable;
	void *m_pGameData; // generally (always???????) an entity pointer
	IVP_Real_Object *real_obj;
	// (incomplete)
};
