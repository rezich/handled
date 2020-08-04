handled
=======

a memory allocator system for game development.

rationale and details will be outlined here eventually.

Sample usage
------------

```C++
#include "handled.h"

struct FrameData {
	float dt = 0.0f;
};

struct Cloud : Handled<Cloud, 100> {
	float x = 0.0f;
	float y = 0.0f;
	float speed = 0.0f;

	void update(FrameData* frame_data) {
		x += speed * frame_data->dt;
	}
	void draw() {
		// ...
	}
};

// init
Cloud::setup();
FrameData frame_data;

// main loop
while (!game_should_exit()) {
	// update
	Cloud::iterate(&Cloud::update, &frame_data);

	// draw
	Cloud::draw(&Cloud::draw);

	// end-of-frame cleanup
	Cloud::cleanup();

	frame_data.dt = get_delta_time();
}
```