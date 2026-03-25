# 🚀 GNU OpenGL Engine (PEngen) Documentation

Hello! I'm pleased to present the complete documentation for the **GNU OpenGL Engine**, which I affectionately call **PEngen**!

| Parameter | Value |
| :--- | :--- |
| **Engine Name** | **GNU OpenGL Engine (PEngen)** |
| **Version** | **0.1.0a (PBR & Anim Update)** |
| **Based** | C++, OpenGL (GLFW/GLAD), GLM |
| **Purpose** | This engine was created so that developers can **quickly and enjoyably** create working 3D scenes without getting bogged down in the OpenGL configuration nitty-gritty—ideal whether you're building a high-performance 3D game or rendering a slick, modern graphical interface for a custom operating system. PEngen provides ready-to-use tools for loading models, controlling the camera, advanced lighting, and UI.

Get ready to dive into the world of 3D graphics!

---

## 🚀 1. Core Module (`Engine` and `Scene`)

### `PEngine::Vec` Structure (Math)
Mathematical primitives for engine calculations. Now supports overloaded operators (`+`, `-`, `*`, `+=`) for `Vec3`.

| Structure | Fields | Description |
| --- | --- | --- |
| **`Vec2`** | `float x, y` | Coordinates for 2D/UI elements. |
| **`Vec3`** | `float x, y, z` | Coordinates for 3D space, physics, and color (`r, g, b`). |
| **`Mat4`** | `float data[16]` | 4x4 Matrix for transformations. |

### `PEngine::Engine` Class
The central class that manages the window, animation, and the application lifecycle.

* **`Engine(int width, int height, const std::string& title, gnu::Level_graphics lg)`**: Initializes GLFW, context, and compiles shaders based on the graphics preset (`PEGL_GRAPHICS_LOW`, `PEGL_GRAPHICS_MEDIUM`, `PEGL_GRAPHICS_HIGH`).
* **`bool ShouldClose()`**: Checks if the user has closed the window.
* **`void Update()`**: Clears buffers, updates animations, input, and swaps frames. Call at the end of each cycle.
* **`Scene* GetScene()`**: Provides access to world objects.
* **`PEngine::Animator animator`**: Built-in animation controller (see Section 3).

### `PEngine::Scene` Class & `Object` Structure
A scene object (`Object`) is a universal container for rendering both 3D meshes and 2D UI.

**`Object` Key Fields:**
* `long long id`: Unique ID.
* `std::string name`: Name to search for.
* `Vec3 pos, size, rotator`: Transformations.
* `uint8_t state`: Type (`0`: Off, `1`: 3D, `2`: 3D+Tex, `3`: Button, `4`: Checkbox, `5`: InputField, `6`: Panel, `7`: Image, `8`: Text).
* `gnu::PEGLMaterial material`: Colors, textures, and emission data.
* `gnu::PEGLModel model`: 3D mesh data.
* `bool is_my_sheder`: Flag to use a custom `shader_program` instead of defaults.

**`Scene` Functions:**
* **`AddObject(const Object& object)`**: Adds an object to the world.
* **`RemoveObject(const char* name)`**: Removes an object from the scene by its name.
* **`SearchObject(const char* name)`** / **`GetObjectById(long long id)`**: Search utilities.
* **`SetCamera(Vec3 position, float yaw, float pitch, float roll)`**: Controls the player's view.
* **Factory Methods**: Rapidly create objects:
  * `create3DObject(...)`, `create3DMaterialObject(...)`
  * `createUIObjectText(...)`, `createUIObjectImage(...)`, `createUIObjectPanel(...)`, `createUIObjectButton(...)`, `createUIObjectCheckbox(...)`, `createUIObjectInputField(...)`

---

## 💡 2. Graphics & Materials Module (`gnu::OpenGL`)

PEngen now uses an advanced rendering pipeline supporting emission and multiple light types.

### `gnu::PEGLMaterial` (PBR-lite Material)
Defines how an object looks and reacts to light.
* `baseColor`, `diffuseColor`: Main colors.
* `opacity`: Transparency level (0.0 to 1.0).
* `shininess`: Highlight sharpness (e.g., 128 for metal, 2 for wood).
* `emissionColor` & `emissionIntensity`: Global glow. Set intensity > 1.0 to create glowing objects (like the Sun or neon lights).
* `diffuseMap`, `normalMap`, `specularMap`: OpenGL texture IDs.

### `gnu::PEGLLight` (Advanced Lighting)
Add lights directly to the scene using `scene->Scene_lights.push_back(light)`.
* `int type`: 
  * `LIGHT_POINT (0)`: Standard bulb with a `radius`.
  * `LIGHT_AMBIENT (1)`: Fills the whole scene (sky light).
  * `LIGHT_SOLAR (2)`: Directional light from infinitely far away (Sun).
  * `LIGHT_SPOT (3)`: Flashlight cone (uses `dir`, `innerCutoff`, `outerCutoff`).
  * `LIGHT_RIM (4)`: Cinematic edge-lighting.
* `color` & `intensity`: Brightness and hue.

---

## 🎬 3. Animation Module (`PEAnim.h`)

Animate any object's position, rotation, scale, and textures using `.peanim` JSON files.

### `PEngine::Animator` Class
Accessed via `engine.animator`.

* **`LoadAnimationPEANIM(const std::string& filepath, Scene* scene)`**: Loads a keyframe animation from a file.
* **`PlayAnimation(const std::string& anim_name, bool loop = false)`**: Starts an animation. Supports looping.
* **`StopAnimation(const std::string& anim_name)`**: Halts playback.
* **`UpdateAnimations(Scene* scene, float deltaTime_ms)`**: Called automatically by `Engine::Update()`. Supports `Linear`, `Sine`, and `SmoothStep` easing for fluid motion.

---

## ⚛️ 4. Physics Module (`Physics.h`, `Physics_Base.h`)

Physics runs in parallel with the scene, updating the coordinates of visual objects.

### `PhysicsObject` Structure
* `long long linked_object_id`: **Link!** ID of a visual object from `Scene`.
* `Vec3 pos, size, velocity, speed_gravity`: Movement and size parameters.
* `float mass`: Object mass.
* `uint8_t collision_type`: `0` - none, `1` - trigger, `2` - rigid body (bounce).
* `int friction`: Friction force.
* `void (*on_collision)(PhysicsObject& other)`: Collision event.

### `PEngine::Physics_Base` Class
* **`Physics_Base(Scene* scene)`**: Constructor that links physics to the scene.
* **`AddPhysicsObject(const PhysicsObject& physics_object)`**: Registers a body.
* **`UpdatePhysics(float deltaTime)`**: Performs collision and displacement calculations.

---

## 🗑️ 5. Cleanup Module (`OpenGL.h`)

Functions for freeing up GPU memory to prevent leaks.
* **`void PEGLDelete_Model(PEGLModel& model)`**: Deletes meshes, VAOs, VBOs, and EBOs.
* **`void PEGLDelete_Shader_Program(PEGLShaderProgram& program)`**: Deletes a compiled shader.

---

## 🔊 6. Audio Module (`Audio.h`)

`AudioManager` class (all functions are static).
* **`PEAPlaySound(const std::string& filePath, int id)`**: Single playback.
* **`PEAPlayLoopedSound(const std::string& filePath, int id)`**: Background music (loop).
* **`PEAStopSound(int id)`** / **`PEAStopAll()`**: Stop audio playback.
* **`PEASetVolume(float volume, int id)`**: Set the volume (0.0 - 1.0).

---

## ⌨️ 7. Input Module (`Input.h`)

`InputManager` class for emulation and management.
* **`PEIKeyPress(WORD vkCode)`** / **`PEIKeyDown(...)`** / **`PEIKeyUp(...)`**: Keyboard state management.
* **`PEIMouseClickL()` / `PEIMouseClickR()**`: Mouse clicks.
* **`PEIMouseMoveAbsolute(int x, int y)`**: Move the cursor to a point on the screen.

---

## 💾 8. Config and Log Module (`cfg.h`, `log.h`)

* **`void PEsave(const string& filePath, const string& data)`**: Saves text to the `PEngine\cfg\` folder.
* **`string PEload(const string& filePath)`**: Loads text from a file.
* **`void PElogger(const char* message)`**: Logs an important event to `log.txt`.
