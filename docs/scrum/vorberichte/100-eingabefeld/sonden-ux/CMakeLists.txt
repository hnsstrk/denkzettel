# Sonden der UX-Entscheidung zur Textfarbe (#100, 07.08.2026).
#
# Eigenes Bauverzeichnis (build-ux/), nicht `build/` im Projektwurzelverzeichnis.
#
#   cmake -B ../build-ux -S . -DCMAKE_BUILD_TYPE=Release && cmake --build ../build-ux

cmake_minimum_required(VERSION 3.28)
project(textfarbsonden LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(KF6Svg REQUIRED)
find_package(KF6ColorScheme REQUIRED)

# Was `widgets/lineedit` unter `base`, `hover` und `focus` je Theme zeichnet.
add_executable(vorsatzsonde vorsatzsonde.cpp)
target_link_libraries(vorsatzsonde PRIVATE Qt6::Widgets KF6::Svg)

# Die vier Paarungen von Vordergrund und Grund über alle Farbschemata.
add_executable(paarungssonde paarungssonde.cpp)
target_link_libraries(paarungssonde PRIVATE Qt6::Widgets KF6::ColorScheme)

# Der zweite Farbweg: die `colors`-Datei des Desktop-Themes (#85).
add_executable(themefarbsonde themefarbsonde.cpp)
target_link_libraries(themefarbsonde PRIVATE Qt6::Widgets KF6::Svg)

# Die Deckung der Huelle je Theme.
add_executable(deckungssonde deckungssonde.cpp)
target_link_libraries(deckungssonde PRIVATE Qt6::Widgets KF6::Svg)

# Der Fokuszustand des Feldes: was die vier Vorsaetze ueber die ganze Flaeche zeichnen.
add_executable(fokussonde fokussonde.cpp)
target_link_libraries(fokussonde PRIVATE Qt6::Widgets KF6::Svg)

# Wo genau die Fokuskante liegt.
add_executable(kantensonde kantensonde.cpp)
target_link_libraries(kantensonde PRIVATE Qt6::Widgets KF6::Svg)

# Die Hinweis-Elemente, an denen Plasmas eigener Bau die Fallunterscheidungen festmacht.
add_executable(hinweissonde hinweissonde.cpp)
target_link_libraries(hinweissonde PRIVATE Qt6::Widgets KF6::Svg)

# Was die Fokusschicht dem Kunden bringt.
add_executable(gewinnsonde gewinnsonde.cpp)
target_link_libraries(gewinnsonde PRIVATE Qt6::Widgets KF6::Svg)

# Ob der Zustand "Fenster offen, aber nicht aktiv" offscreen pruefbar ist.
add_executable(aktivsonde aktivsonde.cpp)
target_link_libraries(aktivsonde PRIVATE Qt6::Widgets)

# Woher frameContrast kommt (Zeichnung 3a).
add_executable(kontrastsonde kontrastsonde.cpp)
target_link_libraries(kontrastsonde PRIVATE Qt6::Widgets KF6::ColorScheme KF6::ConfigCore)
