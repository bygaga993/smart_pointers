# C++ Smart Pointers

Мои учебные реализации умных указателей на C++20 и тесты к ним.

- `unique` — UniquePtr и CompressedPair.
- `shared` — SharedPtr.
- `weak` — SharedPtr и WeakPtr.
- `shared-from-this` — поддержка SharedFromThis.
- `intrusive` — IntrusivePtr.

Каждый этап расположен в `tasks/smart-ptrs` и собирается отдельно: определения классов между этапами могут повторяться.

## Сборка и запуск

Нужны CMake 3.16+ и компилятор C++20 (Clang или GCC). Catch2 и вспомогательный счётчик аллокаций включены в репозиторий.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j 4
ctest --test-dir build --output-on-failure
```

Для проверки с AddressSanitizer и UndefinedBehaviorSanitizer добавьте `-DENABLE_SANITIZERS=ON` при настройке отдельной сборки.

## Источник заданий и тестов

Задания, исходные тесты, Catch2 и `allocations_checker` взяты из публичного [репозитория курса HSE Advanced C++ 2026](https://gitlab.com/hse-cpp/hse-advanced-cpp-public-2026), базовый коммит `c9ddda6a2318254cf9a5086d3429202826ab946e`.
Реализации и тесты сохранены в текущем локальном состоянии, включая мои изменения в тестах. Учебные тесты не заявляются как моя авторская работа. Уведомления об авторстве стороннего кода сохранены в исходниках.

## Результат локальной проверки

Проверено 30 октября 2026 года: macOS, AppleClang 21, Debug, без санитайзеров.
Сборка всех пяти наборов тестов успешна.

| Набор | Результат |
| --- | --- |
| UniquePtr | Проходит |
| SharedPtr | Проходит |
| WeakPtr | Проходит |
| IntrusivePtr | Проходит |
| SharedFromThis | SIGSEGV в тесте `SharedFromThis` (`test.cpp:25`) |

Реализация SharedFromThis сохранена как есть; ошибка пока не исправлена.
