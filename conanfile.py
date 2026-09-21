from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout


class ConanApplication(ConanFile):
    name = "2iREN"
    version = "0.1.0"
    description = "A C++23 Graphics Framework"
    topics = ("graphics", "rendering", "opengl", "metal")

    package_type = "static-library"
    settings = "os", "compiler", "build_type", "arch"

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        tc.generate()

    def requirements(self):
        self.requires("opengl/system")
        self.requires("glfw/3.4")  # with_wayland = False is default value in recipe

        self.requires("cgltf/1.15")
        self.requires("yaml-cpp/0.9.0")
        self.requires("stb/cci.20240531")

        if self.settings.os == "Macos":
            self.requires("metal-cpp/26")

        if self.settings.os in ("Windows", "Linux"):
            self.requires(
                "glad/2.0.8",
                options={
                    "gl_version": "4.6",
                    "gl_profile": "core",
                },
            )

    def build_requirements(self):
        self.test_requires("doctest/2.5.2")

    def build(self):
        """Instructs Conan on how to build this package."""
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
