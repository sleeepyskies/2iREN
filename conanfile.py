from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMakeToolchain, CMakeDeps, CMake


class ConanApplication(ConanFile):
    name = "2iREN"
    version = "0.1.0"
    description = "a cpp23 graphics framework"
    topics = ("graphics", "rendering", "opengl")

    package_type = "static-library"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.user_presets_path = False
        tc.generate()

    def requirements(self):
        requirements = self.conan_data.get('requirements', [])

        for requirement in requirements:
            if isinstance(requirement, dict):
                self.requires(
                    requirement["ref"],
                    options=requirement.get("options", {})
                )
            else:
                self.requires(requirement)
