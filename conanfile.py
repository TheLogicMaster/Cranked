from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMakeToolchain
from conan.tools.files import copy

class ConanApplication(ConanFile):
    package_type = "application"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"
    # generators = "CMakeDeps"

    # def layout(self):
    #     cmake_layout(self)

    # def generate(self):
    #     tc = CMakeToolchain(self)
    #     tc.user_presets_path = False
    #     tc.generate()

    def requirements(self):
        requirements = self.conan_data.get('requirements', [])
        for requirement in requirements:
            self.requires(requirement)

    def generate(self):
        for dep in self.dependencies.values():
            if dep.cpp_info.bindirs:
                copy(self, "*.dll",
                     src=dep.cpp_info.bindir,
                     dst=self.build_folder,
                     keep_path=False)
