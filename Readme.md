# AnymaEngine 

This will be the most large scale and fastest evolution simulation out there. 

Brain update rate will be 1 / 6 of the physics update rate.

### Large scale TODO
 - Switch to HandmadeMath linear algebra library.
 - Make neural network library.
 - Implement Evolution Strategies.
 - Break world up in chunks and process chunks using multithreading.
 - Make cute renderer.

#### TODO
- Make agents selectable.
- Show agent info in debug window.
- Show agent rays and sensors when selected.
- Make super simple brain.
- Set debug option: Show chunks etc.
- Separate data collection and debug data rendering.
- Show agents in chunk in debug window. 
- Pause simulation.
- Make texture region.
- Pass tex_orig and tex_size as texture region.
- Make chunks selectable.
- Global empty texture for convenience.
- Make color header with list of colors and color helper functions.
- Make creature respond to sensor in a simple way.

#### Done
- Asexual reproduction.
- Collect statistics about chunk occupancy.
- Only render visible agents.
- Divide the world into chunks.
- Update creature sensor info based on raycast (seeing).
- Do raycast;
- Do simple collision detection.
- Resize screen.
- Fix Array issues.
- Level of detail.
- Render eyes.
- Use N-gon to draw creatures.
- Sorting in Array
- Render N-polygon
- Make creature struct (Agent).
- Line rendering.
- Fix frame update rate at user specified FPS with SDL_Delay.
- Create window struct for platform handling.
- Render a creature.
- Implement scrolling.
- Move the 2D camera with WASD.
- Move the 2D camera with the mouse.
- Handle user input.
- Make a 2D camera.
- Make a basic 2D renderer.
- Import imgui with docking.

### Ideas
- For an overview of how the world is evolving, create a kind of sampling minimap. Dont render each individual creature but take a couple of samples for each pixel using the bsp-tree and take the average color. Store these frames and show as animation.