# 🚀 Документация GNU OpenGL Engine (PEngen)

Привет! Я рад представить полную документацию по **GNU OpenGL Engine**, который я ласково называю **PEngen**!

| Параметр | Значение |
| :--- | :--- |
| **Название движка** | **GNU OpenGL Engine (PEngen)** |
| **Версия** | **0.1.0a (PBR & Anim Update)** |
| **Основа** | C++, OpenGL (GLFW/GLAD), GLM |
| **Цель** | Этот движок создан для того, чтобы разработчики могли **быстро и с удовольствием** создавать работающие 3D-сцены, не увязая в рутине настройки OpenGL. Он идеально подходит как для создания высокопроизводительных 3D-игр, так и для рендеринга современных графических интерфейсов кастомных ОС. PEngen предоставляет готовые инструменты для загрузки моделей, управления камерой, продвинутого освещения и UI. |

Приготовьтесь погрузиться в мир 3D-графики!

---

## 🚀 1. Основной модуль (`Engine` и `Scene`)

### Структура `PEngine::Vec` (Математика)
Математические примитивы для вычислений движка. Теперь поддерживаются перегруженные операторы (`+`, `-`, `*`, `+=`) для `Vec3`.

| Структура | Поля | Описание |
| --- | --- | --- |
| **`Vec2`** | `float x, y` | Координаты для 2D/UI элементов. |
| **`Vec3`** | `float x, y, z` | Координаты для 3D пространства, физики и цвета (`r, g, b`). |
| **`Mat4`** | `float data[16]` | Матрица 4x4 для трансформаций. |

### Класс `PEngine::Engine`
Центральный класс, управляющий окном, анимацией и жизненным циклом приложения.

* **`Engine(int width, int height, const std::string& title, gnu::Level_graphics lg)`**: Инициализирует GLFW, контекст и компилирует шейдеры на основе графического пресета (`PEGL_GRAPHICS_LOW`, `PEGL_GRAPHICS_MEDIUM`, `PEGL_GRAPHICS_HIGH`).
* **`bool ShouldClose()`**: Проверяет, не закрыл ли пользователь окно.
* **`void Update()`**: Очищает буферы, обновляет анимации, ввод и меняет кадры. Вызывать в конце каждого цикла.
* **`Scene* GetScene()`**: Дает доступ к объектам мира.
* **`PEngine::Animator animator`**: Встроенный контроллер анимаций (см. раздел 3).

### Класс `PEngine::Scene` и структура `Object`
Объект сцены (`Object`) — это универсальный контейнер для рендеринга как 3D-сеток, так и 2D-интерфейса.

**Ключевые поля `Object`:**
* `long long id`: Уникальный ID.
* `std::string name`: Имя для поиска.
* `Vec3 pos, size, rotator`: Трансформации (позиция, масштаб, вращение).
* `uint8_t state`: Тип объекта (`0`: Выкл, `1`: 3D, `2`: 3D+Текстура, `3`: Кнопка, `4`: Чекбокс, `5`: Поле ввода, `6`: Панель, `7`: Изображение, `8`: Текст).
* `gnu::PEGLMaterial material`: Цвета, текстуры и данные свечения.
* `gnu::PEGLModel model`: Данные 3D-сетки (меш).
* `bool is_my_sheder`: Флаг использования кастомного шейдера вместо стандартного.

**Функции `Scene`:**
* **`AddObject(const Object& object)`**: Добавляет объект в мир.
* **`RemoveObject(const char* name)`**: Удаляет объект со сцены по имени.
* **`SearchObject(const char* name)`** / **`GetObjectById(long long id)`**: Утилиты поиска.
* **`SetCamera(Vec3 position, float yaw, float pitch, float roll)`**: Управление камерой игрока.
* **Фабричные методы (Factory)**: Быстрое создание объектов:
  * `create3DObject(...)`, `create3DMaterialObject(...)`
  * `createUIObjectText(...)`, `createUIObjectImage(...)`, `createUIObjectPanel(...)`, `createUIObjectButton(...)`, `createUIObjectCheckbox(...)`, `createUIObjectInputField(...)`

---

## 💡 2. Модуль графики и материалов (`gnu::OpenGL`)

PEngen использует продвинутый конвейер рендеринга с поддержкой излучения (emission) и нескольких типов освещения.

### `gnu::PEGLMaterial` (Материал PBR-lite)
Определяет, как объект выглядит и реагирует на свет.
* `baseColor`, `diffuseColor`: Основные цвета.
* `opacity`: Прозрачность (от 0.0 до 1.0).
* `shininess`: Интенсивность блика (например, 128 для металла, 2 для дерева).
* `emissionColor` и `emissionIntensity`: Глобальное свечение. Установите интенсивность > 1.0, чтобы создать светящиеся объекты (солнце, неон).
* `diffuseMap`, `normalMap`, `specularMap`: ID текстур OpenGL.

### `gnu::PEGLLight` (Продвинутое освещение)
Добавляйте источники света прямо в сцену: `scene->Scene_lights.push_back(light)`.
* `int type`: 
  * `LIGHT_POINT (0)`: Точечный свет (лампочка) с радиусом `radius`.
  * `LIGHT_AMBIENT (1)`: Фоновое освещение всей сцены (свет неба).
  * `LIGHT_SOLAR (2)`: Направленный свет с бесконечного расстояния (Солнце).
  * `LIGHT_SPOT (3)`: Прожектор/фонарик (использует `dir`, `innerCutoff`, `outerCutoff`).
  * `LIGHT_RIM (4)`: Кинематографическая контурная подсветка.
* `color` и `intensity`: Цвет и яркость.

---

## 🎬 3. Модуль анимации (`PEAnim.h`)

Анимируйте позицию, вращение, масштаб и текстуры любого объекта с помощью JSON-файлов `.peanim`.

### Класс `PEngine::Animator`
Доступен через `engine.animator`.

* **`LoadAnimationPEANIM(const std::string& filepath, Scene* scene)`**: Загружает анимацию ключевых кадров из файла.
* **`PlayAnimation(const std::string& anim_name, bool loop = false)`**: Запускает анимацию. Поддерживает зацикливание.
* **`StopAnimation(const std::string& anim_name)`**: Останавливает воспроизведение.
* **`UpdateAnimations(Scene* scene, float deltaTime_ms)`**: Вызывается автоматически в `Engine::Update()`. Поддерживает сглаживание `Linear`, `Sine` и `SmoothStep` для плавности.

---

## ⚛️ 4. Модуль физики (`Physics.h`, `Physics_Base.h`)

Физика работает параллельно со сценой, обновляя координаты визуальных объектов.

### Структура `PhysicsObject`
* `long long linked_object_id`: **Связь!** ID визуального объекта из `Scene`.
* `Vec3 pos, size, velocity, speed_gravity`: Параметры движения и размеров.
* `float mass`: Масса объекта.
* `uint8_t collision_type`: `0` - нет, `1` - триггер, `2` - твердое тело (отскок).
* `int friction`: Сила трения.
* `void (*on_collision)(PhysicsObject& other)`: Событие при столкновении.

### Класс `PEngine::Physics_Base`
* **`Physics_Base(Scene* scene)`**: Конструктор, связывающий физику со сценой.
* **`AddPhysicsObject(const PhysicsObject& physics_object)`**: Регистрирует тело в симуляции.
* **`UpdatePhysics(float deltaTime)`**: Проводит расчет столкновений и перемещений.

---

## 🗑️ 5. Модуль очистки (`OpenGL.h`)

Функции для освобождения памяти GPU и предотвращения утечек.
* **`void PEGLDelete_Model(PEGLModel& model)`**: Удаляет меши, VAO, VBO и EBO.
* **`void PEGLDelete_Shader_Program(PEGLShaderProgram& program)`**: Удаляет скомпилированный шейдер.

---

## 🔊 6. Модуль аудио (`Audio.h`)

Класс `AudioManager` (все функции статические).
* **`PEAPlaySound(const std::string& filePath, int id)`**: Одиночное воспроизведение.
* **`PEAPlayLoopedSound(const std::string& filePath, int id)`**: Фоновая музыка (цикл).
* **`PEAStopSound(int id)`** / **`PEAStopAll()`**: Остановка звуков.
* **`PEASetVolume(float volume, int id)`**: Установка громкости (0.0 - 1.0).

---

## ⌨️ 7. Модуль ввода (`Input.h`)

Класс `InputManager` для эмуляции и управления.
* **`PEIKeyPress(WORD vkCode)`** / **`PEIKeyDown(...)`** / **`PEIKeyUp(...)`**: Управление состоянием клавиш.
* **`PEIMouseClickL()` / `PEIMouseClickR()`**: Клики мыши.
* **`PEIMouseMoveAbsolute(int x, int y)`**: Перемещение курсора в точку экрана.

---

## 💾 8. Модуль конфигурации и логов (`cfg.h`, `log.h`)

* **`void PEsave(const string& filePath, const string& data)`**: Сохраняет текст в папку `PEngine\cfg\`.
* **`string PEload(const string& filePath)`**: Загружает текст из файла.
* **`void PElogger(const char* message)`**: Записывает важное событие в `log.txt`.