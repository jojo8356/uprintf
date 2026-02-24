from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.files import copy
import os


class UprintfConan(ConanFile):
    name = "uprintf"
    version = "1.0.0"
    license = "MIT"
    url = "https://github.com/jojo8356/uprintf"
    description = "Universal printf for C — auto-dispatches between printf and wprintf"
    topics = ("printf", "wprintf", "unicode", "portable", "tchar")
    settings = "os", "compiler", "build_type", "arch"
    no_copy_source = True
    exports_sources = "include/*"

    def package(self):
        copy(self, "*.h", src=os.path.join(self.source_folder, "include"),
             dst=os.path.join(self.package_folder, "include"))
        copy(self, "LICENSE", src=self.source_folder,
             dst=os.path.join(self.package_folder, "licenses"))

    def package_info(self):
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        self.cpp_info.defines = ["UPRINTF_HEADER_ONLY"]

    def package_id(self):
        self.info.clear()
