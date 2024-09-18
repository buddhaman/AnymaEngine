# AnymaEngine 

<img src="anyma_engine_logo.png" alt="Anyma Engine Logo" width="300"/>

This will be the most large scale and fastest evolution simulation out there. 

Brain update rate will be 1 / 6 of the physics update rate.

## Building
Make sure you have cmake, vcpkg and all necessary c++ development tools for your operating system.
 - Go to the root directory.
 - Make a folder ./build
 - `cd build`
 - `cmake ..`

 this will generate a build folder with a visual studio solution file if you are on windows. Open this file or you can build using cmake

 - `cmake --build . --config Release`

 --- 

### Large scale TODO
 - Switch to HandmadeMath linear algebra library.
 - Break world up in chunks and process chunks using multithreading.
 - Make cute renderer.
 - Make the world absolutely huge and do processing by chunk offsets to prevent floating point errors.
 - Two pass collision solving.
 - Improve standard library: Add string type, better sorting.
 - Improve standard library: Make own multithreading primitives.
 - Build introspection and profiling tools.
 - Do real good collision detection, fix the fact that lots of agents are being checked twice. 
 - Do rendering on background thread such that the main thread doesnt block and feels smooth.
 - Saving and loading the state of an entire world.
 - Tracking succesful agents.
 - Tracking lineage.
 - Showing the entire state of the brain and genes.
 - Make a circular buffer.
 - Remove CMAKE. I decided to use CMake for this project and i regret it. Very much. 
 - IDEA: Create a kind of business rule table where you can set rules like: 
    - Required speed for carnivores to do damage to herbivores
    - Allow sprinting, how much energy to take on a sprint?
    - Do carnivores get a reproduction boost if their population is low?

#### Design

Ideas for charge / attack:
- Give both carnivores and herbivores the ability to sprint, but for carnivores this is the only way to
  attack a herbivore. Sprinting can either deplete total energy, or there can be a seperate sprint energy 
  that just fills over time.

#### TODO
- Show charge of agent in ui.
- Make grass fields , energy in ground.
- Configure amount of eyes and fov.
- Create real and simple memory pool with chunks
- Show indicator when herbivore is hurt.
- Edit reproduction rate calculation, more offspring if population is low for instance.
- Implement my own std::function.
- OPTION: Boost or nerf agents if population is large.
- Plotting utility functions.
- Make active and inactive chunks.
- Fix agents popping into view.
- Draw scale with some text too. 
- Show agents in chunk in debug window. 
- Consistently call everything either draw or render. 
- Make random number generator.
- Make texture region.
- Global empty texture for convenience.
- Make list of common colors.

### Done
- Recoil, or however you wanna call it when a herbivore is hurt. They cannot move unless they sprint?
- Use the charge or refractory value to disable movement for herbivore agents.
- Fix colors everywhere, follow imgui layout.
- Give carnivore agents a charge/attack procedure that is controlled by a neuron. This costs energy.
- Configure energy transfer on hit.
- Show overlays: Make coloring of genes to show genetic distance of different agents.
- Make color helper functions.
- Better agent selection, give feedback on agent hitboxes.
- Plot low dimensional version of genes.
- Draw active chunks.
- Compare two agent genes.
- FIX BUG: SelectFromWorld often has idx out of range.
- Highlight currently hovered agent.
- Create some background in the world, it looks boring right now. 
- Make neural network library.
- Implement alpha blending.
- Icons for imgui.
- Make a startup screen to start simulation with new persistent parameters.
- Turn off asserts when not debugging.
- Show number of workers and available threads. Make settings.
- Custom font for imgui.
- Show time also in seconds, not just ticks.
- Make agents squishy in direction of their velocity.
- Make carnivore attack.
- Collide herbivores and carnivores.
- Implement simple jobsystem.
- Do sensor updates and brain updates in parallel.
#### RELEASE 3
- More options for user. Spawn agents.
- Make mutation_rate adjustable in gui.
- Keep selection when moving camera around.
- Organize interface.
- Debug tools for memory arenas, check if clearing doesnt lead to problems.
- Implement 2 memory arenas for lifetime management.
- Make super simple brain.
- Agent lifespan.
- Agent asexual reproduction with mutations.
- Make cumulative graph of types of agents.
- IMPORTANT: Solve the huge memory leak in AddAgent.
- Remove agents.
- Pause simulation.
#### RELEASE 2
- Simple behavior.
- Setup screen to make new simulation.
- Show agent sensors when selected.
- Set debug option: Show chunks etc.
- Refactor main loop and main in general. Move to SimulationScreen.
- Show agent rays when selected.
- Show agent info in debug window.
- Raycasting for all eyes.
- Sort agents into all chunks they belong to.
- Draw grid in a more efficient way and in the background.
- Grid based collision detection for rays.
- Camera to world position.
- Make agents selectable.
- Handle collisions using chunks.
- Draw eye rays for selected agent.
#### RELEASE 1
- Create itch.io page.
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
- Set business rules for evolution.
- Make interesting presets, make it easier for the user.

### Bugs
- No agents leads to deadlock?