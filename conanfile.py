from conans import ConanFile, CMake, tools


class LibtcodConan(ConanFile):
    name = "libtcod"
    license = "BSD 3-Clause License"
    author = "Kyle Stewart 4b796c65+libtcod@gmail.com"
    url = "https://github.com/libtcod/libtcod"
    description = "A free, fast, portable and uncomplicated API for roguelike developers."
    topics = ("roguelikedev",)
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": False}
    generators = "cmake"
    requires = (
        "sdl2/[~=2.0.5]@bincrafters/stable",
        "zlib/[~=1.2.11]@conan/stable",
    )
    scm = {
        "type": "git",
        "url": "https://github.com/libtcod/libtcod.git",
        "revision": "auto"
    }

    def set_version(self):
        """Use `git describe` for the version string."""
        git = tools.Git()
        self.version = git.run("describe")

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder="src")
        cmake.build()

    def package(self):
        self.copy("*.h", dst="include", src="src", excludes="vendor/*")
        self.copy("*.hpp", dst="include", src="src", excludes="vendor/*")
        self.copy("*.lib", dst="lib", src="lib", keep_path=False)
        self.copy("TCOD.dll", dst="bin", src="bin", keep_path=False)
        self.copy("*.so", dst="lib", src="lib", keep_path=False)
        self.copy("*.dylib", dst="lib", src="lib", keep_path=False)
        self.copy("*.a", dst="lib", src="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["TCOD"]
        if not self.options.shared:
            self.cpp_info.defines = ["LIBTCOD_STATIC"]
