# ESP-IDF Component CMakeLists.txt for main component
idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES jenlib
)
