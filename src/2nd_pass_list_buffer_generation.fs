#version 430 core

// This is the atomic counter used to allocate items in the linked list
layout (binding = 0, offset = 0) uniform atomic_uint list_counter;

// Linked list 1D buffer
layout (binding = 2, rgba32ui) uniform writeonly uimageBuffer list_buffer;

// Head pointer 2D buffer
layout (binding = 1, r32ui) uniform uimage2D head_pointer_image;

// Camera visibility map
layout (binding = 1) uniform sampler2D visibility_map;

void main(void) {

	// Read sample position from the visibility map that is transformed to the light space
	vec4 camera_sample_pos = texelFetch(visibility_map, ivec2(gl_FragCoord.xy), 0);

	if(camera_sample_pos.w != 1.0f) {
		discard;
    }

	// Discard fragments that are outside of the viewing frustum
	if (any(greaterThan(abs(camera_sample_pos.xy), abs(camera_sample_pos.zz)))) discard;

	// Get view plane coordinates
	vec2 light_sample_coord = -(camera_sample_pos.xy / camera_sample_pos.z) * 0.5 + 0.5;

	// Allocate an index in the linked list buffer.
	uint index = atomicCounterIncrement(list_counter);

	// Insert the fragment into the list - atomically exchange newly allocated index with the current content of the head pointer image
	uint old_head_ptr = imageAtomicExchange(head_pointer_image, ivec2(light_sample_coord * imageSize(head_pointer_image)), index);

	// Linked list entry
	uvec4 item;
	// head_pointer_image(x,y) -> new_item (.x) -> old_item.
	item.x = old_head_ptr;
	// Store fragment coordinates needed to access the visibility map in the next pass
	item.yz = uvec2(gl_FragCoord.xy);
	// The point does not lie in shadow by default
	item.w = 0U;

	// Write the data into the buffer at the right location
	imageStore(list_buffer, int(index), item);
}